# Introduction

A small asynchronous application framework based on Boost/Asio, with components that can be reused accross different projects.

**Requires C++20**.

It is divided in *packages* depending on the type of functionality offered:

- Base package (`kouta::base`): Provides the basic building blocks for the application (event loop access and callbacks)
- I/O package (`kouta::io`): Provides I/O related functionality
- Utilities package (`kouta::utils`): Provides useful utilities

## Building

Kouta requires the following libraries:

- [CMake](https://cmake.org/) >= `3.18`
- [Boost](https://www.boost.org/) >= `1.74.0`
- [Asio](https://think-async.com/Asio)>= `1.18.0` (if standalone Asio is used via the `KOUTA_STANDALONE_ASIO` option)

```
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

The library can be built statically or as a shared library (configurable via the `KOUTA_BUILD_SHARED`). In addition, there are targets exposing a **header-only** interface, which may be identified by the suffix `-header`.

## Documentation

Documentation can be built with [Doxygen](https://www.doxygen.nl/):

```
$ doxygen Doxyfile
```

The resulting HTML documentation can be found in `docs/html`.


## Testing

Tests are implemented using [GoogleTest](https://github.com/google/googletest) and can be compiled after enabling the `KOUTA_BUILD_TESTS` option in CMake

```
# Static/Shared library version
$ cmake --build build --target kouta-tests

# Header-only version
$ cmake --build build --target kouta-tests-header
```

The above command will result in the binaries `build/tests/kouta-tests` and `build/tests/kouta-tests-header` respectively, which can be executed to run all the test cases.
