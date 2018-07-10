cmake_minimum_required( VERSION 3.5.0 )

# create compile_commands.json for tools
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CMAKE_CXX_STANDARD 17 )

# dependencies
find_package( Boost 1.67 REQUIRED COMPONENTS system )
find_package( Qt5Widgets REQUIRED )

add_compile_options( -Wall -Wextra -Werror )

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

# Qt basic example, no coroutines
QT5_WRAP_CPP( CR_MOC_SRC colorrect.h )
add_executable( qb qt_basic.cpp ${CR_MOC_SRC} colorrect.cpp )
target_compile_options( qb PUBLIC -stdlib=libc++ )
target_link_libraries( qb Qt5::Widgets -stdlib=libc++ )

foreach( ex mg ba cb cac ac )
    target_compile_options( ${ex} PUBLIC -fcoroutines-ts -stdlib=libc++ -D_LIBCPP_DEBUG_=1 )
    target_link_libraries( ${ex} PUBLIC -stdlib=libc++ )
endforeach()

# Asio example needs Boost, and Asio needs pthread and system
target_link_libraries( ac PRIVATE Boost::boost pthread Boost::system )
