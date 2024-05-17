# Kouta

A small application framework based on Boost, with components that can be reused accross different projects.

**Requires C++20**.

## Building

Kouta requires the following libraries:

- [CMake](https://cmake.org/) >= `3.27`
- [Boost](https://www.boost.org/) >= `1.78.0`

```
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

## Documentation

Documentation can be built with **Doxygen** after enabling the `BUILD_DOCS` option in CMake:

```
$ cmake --build build --target doxygen
```

The resulting HTML documentation can be found in `build/docs/html`.

## Testing

Tests are implemented using [GoogleTest](https://github.com/google/googletest) and can be compiled after enabling the `BUILD_TESTS` option in CMake:

```
$ cmake --build build --target kouta-tests
```

The above command will result in the binary `build/kouta-tests`, which can be executed to run all the test cases.
