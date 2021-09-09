#include "cpplog.hpp"

int main()
{
    cpplog::Logger logger(cpplog::LogLevel::INFO, "$thread_name: [%D %T] ($level) -> $message");

    logger.register_current_thread("main-thread");

    logger.debug("This is an debug message");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    logger.info("This is an info message");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    logger.warning("This is an warning message");
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    logger.error("This is an error message");
}