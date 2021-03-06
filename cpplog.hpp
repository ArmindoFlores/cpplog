#ifndef _CPPLOG_H_
#define _CPPLOG_H_

#include <atomic>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <chrono>
#include <ctime>
#include <iostream>

static const std::string levels[] = {"DEBUG", "INFO", "WARNING", "ERROR"};

namespace cpplog {
    enum class LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    struct LogMessage {
        std::string message_body;
        std::chrono::system_clock::time_point time_point;
        std::thread::id thread_id;
        LogLevel level;
    };

    class Logger {
    public:
        Logger(Logger &) = delete;
        void operator=(const Logger&) = delete;

        static Logger* get_instance()
        {
            std::unique_lock<std::mutex> lck(instance_mutex);
            if (!Logger::instance)
                Logger::instance = new Logger();
            return Logger::instance;
        }
        static void delete_instance()
        {
            std::unique_lock<std::mutex> lck(instance_mutex);
            delete Logger::instance;
            Logger::instance = nullptr;
        }

        static void set_log_level(LogLevel level) 
        {
            get_instance()->log_level = level; 
        }
        static LogLevel get_log_level() { return get_instance()->log_level; }
        static void set_config_format(const std::string& config) 
        { 
            Logger *inst = get_instance();
            inst->config_mutex.lock();
            inst->config_format = config; 
            inst->config_mutex.unlock();
        }

        static void debug(const std::string& msg)
        {
            Logger *inst = get_instance();
            if (inst->log_level <= LogLevel::DEBUG)
                inst->add_message(msg, LogLevel::DEBUG);
        }
        static void info(const std::string& msg)
        {
            Logger *inst = get_instance();
            if (inst->log_level <= LogLevel::INFO)
                inst->add_message(msg, LogLevel::INFO);
        }
        static void warning(const std::string& msg)
        {
            Logger *inst = get_instance();
            if (inst->log_level <= LogLevel::WARNING)
                inst->add_message(msg, LogLevel::WARNING);
        }
        static void error(const std::string& msg)
        {
            Logger *inst = get_instance();
            if (inst->log_level <= LogLevel::ERROR)
                inst->add_message(msg, LogLevel::ERROR);
        }

        static void register_current_thread(const std::string& name)
        {
            Logger *inst = get_instance();
            inst->map_mutex.lock();
            inst->thread_names[std::this_thread::get_id()] = name;
            inst->map_mutex.unlock();
        }

    protected:
        static Logger *instance;
        static std::mutex instance_mutex;
        Logger(LogLevel log_level=LogLevel::WARNING, const std::string& config_format="")
        : log_level(log_level), running(true)
        {
            if (this->config_format == "")
                this->config_format = "$thread_name::$level $message"; 
            consumer = std::thread([this](){ this->consumer_thread(); });
        }
        ~Logger()
        {
            running = false;
            consumer.join();
        }

    private:
        LogLevel log_level;
        std::string config_format;
        std::atomic<bool> running;
        std::thread consumer;
        std::map<std::thread::id, std::string> thread_names;
        std::queue<LogMessage> log_messages;
        std::condition_variable queue_cv;
        std::mutex queue_mutex, map_mutex, config_mutex;

        void output_log_message(const LogMessage& msg)
        {
            config_mutex.lock();
            std::string config = config_format;
            config_mutex.unlock();
            std::time_t t = std::chrono::system_clock::to_time_t(msg.time_point);
            auto tm = std::localtime(&t);
            std::size_t size = std::strftime(nullptr, config_format.length()*2, config_format.c_str(), tm);
            char *buffer = new char[size+2];
            std::strftime(buffer, size+1, config_format.c_str(), tm);
            std::string pre_processed(buffer);
            delete[] buffer;
            
            std::string final = "";
            std::string keyword_text;
            bool keyword = false;
            for (std::size_t i = 0; i < pre_processed.length(); i++) {
                if (!keyword) {
                    if (pre_processed[i] == '$') {
                        keyword = true;
                        keyword_text = "";
                    }
                    else
                        final += pre_processed[i];
                } 
                else {
                    if ((pre_processed[i] >= 'a' && pre_processed[i] <= 'z') || (pre_processed[i] >= 'A' && pre_processed[i] <= 'Z') || pre_processed[i] == '_') {
                        keyword_text += pre_processed[i];
                        if (i != pre_processed.length() - 1)
                            continue;
                    }
                    if (keyword_text == "thread_name")
                        final += get_thread_name(msg.thread_id);
                    else if (keyword_text == "message")
                        final += msg.message_body;
                    else if (keyword_text == "level")
                        final += levels[(int) msg.level];

                    final += pre_processed[i];
                    keyword = false;
                }
            }
            std::cout << final << std::endl;
        }

        std::string get_thread_name(std::thread::id thread_id)
        {
            std::unique_lock<std::mutex> lck(map_mutex);
            if (thread_names.count(thread_id))
                return thread_names[thread_id];
            return "unnamed-thread-" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }

        void consumer_thread()
        {
            while (running) {
                auto next_cycle = std::chrono::system_clock::now() + std::chrono::milliseconds(25);
                {
                    std::unique_lock<std::mutex> lck(queue_mutex);
                    bool success = queue_cv.wait_for(lck, std::chrono::milliseconds(25), [this]{ return !this->log_messages.empty(); });
                    if (success) {
                        while (!log_messages.empty()) {
                            output_log_message(log_messages.front());
                            log_messages.pop();
                        }
                    }
                }
                std::this_thread::sleep_until(next_cycle);
            }
        }

        void add_message(const std::string& msg, LogLevel level)
        {
            queue_mutex.lock();
            log_messages.push(LogMessage{msg, std::chrono::system_clock::now(), std::this_thread::get_id(), level});
            queue_cv.notify_one();
            queue_mutex.unlock();
        }
    };

    Logger *Logger::instance = nullptr; 
    std::mutex Logger::instance_mutex;
}

#endif