# Introduction

Kouta is a small **C++20** application framework.

It is divided in *packages* depending on the type of functionality offered:

- Base package (`kouta::base`): Provides the basic building blocks for the application (event loop access and callbacks)
- I/O package (`kouta::io`): Provides I/O related functionality


## Building

Kouta requires the following libraries:

- [CMake](https://cmake.org/) >= `3.27`
- [Boost](https://www.boost.org/) >= `1.78.0`
- [Asio](https://think-async.com/Asio)>= `1.22.0` (if standalone Asio is used via the `KOUTA_STANDALONE_ASIO` option)

```
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

## Documentation

Documentation can be built with [Doxygen](https://www.doxygen.nl/):

```
$ doxygen Doxyfile
```

The resulting HTML documentation can be found in `docs/html`.


## Testing

Tests are implemented using [GoogleTest](https://github.com/google/googletest) and can be compiled after enabling the `KOUTA_BUILD_TESTS` option in CMake:

```
$ cmake --build build --target kouta-tests
```

The above command will result in the binary `build/kouta-tests`, which can be executed to run all the test cases.
