# cpplog
cpplog is a logging framework for C++ projects. It is thread safe and works in both Linux and Windows. It's a single header file that can be included in you project and used right out of the gate.

## Usage
After including the header file (`#include "cpplog.hpp"`), you can call Logger::debug(), Logger::info(), Logger::warning() or Logger::error() to log a message. Use Logger::set_log_level() to set the minimum log level (all logs below that threshold won't be displayed) and Logger::set_config_format() to set the message output format. To compile a program that uses cpplog, you need use the `-pthread` flag.

This example program

```c++
#include "cpplog.hpp"
using namespace cpplog;

int main()
{
    Logger::warning("Hello World!");
}
```

produces this output

```
unnamed-thread-15926518431851862680::WARNING Hello World!
```

### Logger::set_config_format()
`Logger::set_config_format()` takes a string parameter that defines how messages should be style. It converts % codes into strftime() values and also supports the aliases $message, $thread_name and $level. The default configuration format is `$thread_name::$level $message`.

### Logger::set_log_level()
`Logger::set_log_level()` takes a LogLevel parameter that defines the minimum log level that should be output. For exaple, if the log level is set to LogLevel::WARNING, `info` and `debug` messages won't be shown. By default, the log level is set to LogLevel::WARNING.

### Logger::register_current_thread()
`Logger::register_current_thread()` takes a string corresponding to the thread name as the parameter. When this thread logs a message, it will use that name instead of unnamed-thread-[TID].