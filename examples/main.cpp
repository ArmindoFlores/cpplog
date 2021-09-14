#include "cpplog.hpp"
using namespace cpplog;

void thread_worker()
{
    Logger::debug("Counting to 10...");
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        Logger::info("i=" + std::to_string(i+1));
    }
    Logger::warning("Counting finished");
}

int main()
{
    Logger::set_config_format("$thread_name: [%D %T] ($level) -> $message ");
    Logger::set_log_level(LogLevel::DEBUG);
    Logger::register_current_thread("main-thread");

    Logger::info("Starting threads...");

    std::thread t1([](){ Logger::register_current_thread("thread-worker-1"); thread_worker(); });
    std::thread t2([](){ Logger::register_current_thread("thread-worker-2"); thread_worker(); });
    std::thread t3([](){ Logger::register_current_thread("thread-worker-3"); thread_worker(); });

    t1.join();
    t2.join();
    t3.join();

    Logger::info("Done!");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    Logger::delete_instance();
}