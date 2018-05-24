cmake_minimum_required( VERSION 3.5.0 )

set( CMAKE_CXX_STANDARD 17 )

# dependencies
find_package( Boost 1.67 REQUIRED COMPONENTS system )

# a simple generator
add_executable( mg manual_generator.cpp )
# a simple thing-that-awaits
add_executable( ba basic_awaiter.cpp )
# a long-running task modeled with an execution queue with completion callbacks
add_executable( cb callbacks.cpp run_queue.cpp )
# the same task done as a coroutine with co_await
add_executable( cac cb_as_coro.cpp )
# Asio as the execution queue *and* co_await
add_executable( ac asio_coro.cpp )

foreach( ex mg ba cb cac ac )
    target_compile_options( ${ex} PUBLIC -fcoroutines-ts -stdlib=libc++ -D_LIBCPP_DEBUG_=1 )
    target_link_libraries( ${ex} PUBLIC -stdlib=libc++ )
endforeach()

# Asio example needs Boost, and Asio needs pthread and system
target_link_libraries( ac PRIVATE Boost::boost pthread Boost::system )
