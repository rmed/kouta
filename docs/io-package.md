# I/O package

The I/O package provides building blocks for I/O-related functionalities.

## Timer

Implemented in `kouta::io::Timer`.

The `Timer` allows performing actions after a certain amount of time. Its duration can be modified at any point, and can also be stopped before it elapses (for instance to cancel a timeout in a request).

Once the `Timer` elapses, it **is within the context of its parent component**, so it is up to the developer to make sure that proper synchronization occurs if calling something outside of that context.

```cpp
#include <iostream>
#include <kouta/io/timer.hpp>

class MyComponent : public kouta::base::Component
{
public:
    explicit MyComponent(kouta::base::Component* parent)
        : kouta::base::Component{parent}
        , m_timer{this, std::chrono::milliseconds{200}, std::bind_front(this, &MyComponent::handle_timeout)}
    {
        m_timer.start();
    }

    void handle_timeout(Timer& timer)
    {
        std::cout << "Timer expired " << std::endl;

        // Modify duration and restart
        timer.set_duration(std::chrono::milliseconds{500});
        timer.start();
    }

private:
    kouta::io::Timer m_timer;
};
```

## Parser

Implemented in `kouta::io::Parser`.

The `Parser` is supposed to be placed over a sequence of raw bytes in order to help extracting information from said sequence. In particular, the following information can be extracted:

- **Integer values** (endian-aware via `boost::endian`)
- **Floating point values** (endian-aware via `boost::endian`)
- **String values**

Moreover, a **read-only view** of the underlying sequence can also be obtained.

Note that this is backed by `std::span`, meaning that the `Parser` **does not own the memory it points to** (and is not thread-safe).

```cpp
#include <cstdint>
#include <string>
#include <vector>
#include <kouta/io/parser.hpp>

std::vector<std::uint8_t> buf{0xFE, 0x84, 0x1D, 0x29, 0xD9, 0x00, 0x04, 0x01, 0xFF};

kouta::io::Parser parser{buf};

// Extract uint16_t starting at position 1 (big endian)
std::uint16_t val1{parser.extract_integral<std::uint16_t>(1)};

// Extract a theoretical int24_t starting at position 3 (little endian)
std::int32_t val2{parser.extract_integral<std::int32_t, 3, parser::Order::little>(3)};

// Extract a float at position 0 (big endian)
float val3{parser.extract_floating_point<float>(0)};

// Extract a string from position 4 to 7
std::string val4{parser.extract_string(4, 4)};

// Get std::span
parser::View view{parser.view()};
```

## Packer

Implemented in `kouta::io::Packer`.

The `Packer` does the **opposite from the `Parser`**: it adds the following to a raw byte sequence:

- **Integer values** (endian-aware via `boost::endian`)
- **Floating point values** (endian-aware via `boost::endian`)
- **String values**
- **Sequences of bytes**

As opposed to the `Parser`, however, the `Packer` **does own the underlying byte container**.

```cpp
#include <cstdint>
#include <string>
#include <vector>
#include <kouta/io/packer.hpp>

std::vector<std::uint8_t> buf{0xFE, 0x84, 0x1D, 0x29, 0xD9, 0x00, 0x04, 0x01, 0xFF};

kouta::io::Packer packer{};

// Add uint16_t (big endian)
packer.insert_integral{std::unit16_t{259}};

// Add a theoretical int24_t (little endian)
packer.insert_integral<std::int32_t, 3, packer::Order::little>{std::int32_t{2837}};

// Add a float (big endian)
packer.insert_float(float{48.293});

// Add a string
packer.insert_string("Hello world!");

// Add bytes
packer.insert_bytes(buf.begin(), buf.end());

// Get byte sequence
auto& data{packer.data()};
```
