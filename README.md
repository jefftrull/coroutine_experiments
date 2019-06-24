# My Coroutine Experiments
This repo contains the results of some experiments using the Coroutines TS - both by itself, and with libraries like Boost.Asio and Qt, to help me better understand its capabilities.

With the exception of the Qt examples I implemented a really basic coroutine using an integer multiply as an example "asynchronous" task. We wait for it to asynchronously execute and then complete the computation by adding another integer. I figured out the necessary infrastructure (awaitable and promise types) necessary to get this working in each case.

The Qt examples are a bit more complex and involve coroutines that use 0, 1, and 2 asynchronous results delivered via signal. The "normal" Qt implementation is in [qt_basic.cpp](qt_basic.cpp) while the Coroutine version is in [qt_coro.cpp](qt_coro.cpp).

## To Build

It's the usual CMake flow, but your compiler needs to be a recent Clang or MSVC 2017+:

    mkdir build; cd build; cmake ..

Common options to cmake:

- Path to the compiler, usually needed for clang: `-DCMAKE_CXX_COMPILER=/path/to/clang++`

- Path to Boost - needed when Boost is in a less-than-usual location: `-DBOOST_ROOT=/path/to/boost_install`. This variable is cached and needs to be given only once per build location.

- Building a release build with debug information: `-DCMAKE_BUILD_TYPE=relwithdebinfo`

### Platform Notes

- It is not necessary to pass the path to the MSVC compiler, but the build has to start from the Visual Studio Command Line.

- No additional cmake options are needed on macOS. Once macPorts updates their boost to 1.77.0, it will be detected by the build and used.

- gcc does not support coroutines at this time.
