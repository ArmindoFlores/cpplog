#include "cpplog.hpp"
using namespace cpplog;

int main()
{
    Logger::warning("Hello World!");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}