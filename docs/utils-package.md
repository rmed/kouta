# Utils package {#docs-utils}

The utils package provides some common utilities that may be reused internally or externally.

## EnumSet

Implemented in kouta::utils::EnumSet.

The `EnumSet` is a specialization of `std::bitset` that allows **using enumerations in order to refer to specific bits**.

Enumerations must:

- Derive from `std::size_t`
- Not set any value for the labels (optionally, value 0 can be set for the first one)
- Contain a `_Total` label at the end

The `_Total` label is used to determine the number of values in the enumeration (by explicitly casting it to an `std::size_t` value).

```c++
enum class MyEnum : std::size_t
{
    A,
    B,
    C,

    _Total
};

EnumSet<MyEnum> set{MyEnum::A, MyEnum::C};

// Test and set using enumeration values
set.test(MyEnum::A);
set.set{MyEnum::B};

// Or raw indices
set.test(0);
set.set(1);
```

## Logger-aware classes

Implemented in kouta::utils::LoggerAware.

The `LoggerAware` abstract class facilitates basic integration of library code with an application logger via the `ILogger` interface (kouta::utils::ILogger). For instance, the following may be an implementation of the interface to be able to log messages via `spdlog`:

```c++
#include <spdlog/spdlog.h>
#include <kouta/utils/ilogger.hpp>

class CustomLogger : public virtual kouta::utils::Ilogger
{
public:
    void debug(std::string_view msg)
    {
        spdlog::debug(msg);
    }

    void info(std::string_view msg)
    {
        spdlog::info(msg);
    }

    void warning(std::string_view msg)
    {
        spdlog::warn(msg);
    }

    void error(std::string_view msg)
    {
        spdlog::error(msg);
    }

    void critical(std::string_view msg)
    {
        spdlog::critical(msg);
    }
};
```

Which could then be provided to a logger-aware class:

```c++
#include <format>

#include <kouta/utils/logger-aware.hpp>

class MyClass : public kouta::utils::LoggerAware
{
public:
    MyClass()
        : kouta::utils::LoggerAware{}
    {}

    void method_1()
    {
        log_info("Logging from the first method");
    }

    void method_2(int val)
    {
        log_warning(std::format("Logging from the second method: {}", val));
    }
};

// ...

MyClass obj{};

// These will not do anything because no logger has been specified
obj.method_1();
obj.method_2(100);

// Set logger
CustomLogger logger{};
obj.set_logger(&logger);

// Logger will forward messages to spdlog
obj.method_1();
obj.method_2(42);
```
