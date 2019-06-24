cmake_minimum_required( VERSION 3.5.0 )

# create compile_commands.json for tools
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CMAKE_CXX_STANDARD 17 )

# dependencies
find_package( Boost 1.67 COMPONENTS system )
find_package( Qt5Widgets REQUIRED )

add_compile_options( -Wall -Wextra -Werror )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_LIBCPP_DEBUG=1" )

# a simple generator
add_executable( mg manual_generator.cpp )
# a simple thing-that-awaits
add_executable( ba basic_awaiter.cpp )
# a long-running task modeled with an execution queue with completion callbacks
add_executable( cb callbacks.cpp run_queue.cpp )
# the same task done as a coroutine with co_await
add_executable( cac cb_as_coro.cpp )

# Qt basic example, no coroutines
QT5_WRAP_CPP( CR_MOC_SRC colorrect.h )
add_executable( qb qt_basic.cpp ${CR_MOC_SRC} colorrect.cpp )
target_compile_options( qb PUBLIC -stdlib=libc++ )
target_link_libraries( qb Qt5::Widgets -stdlib=libc++ )

# Qt with coroutines
add_executable( qc qt_coro.cpp ${CR_MOC_SRC} colorrect.cpp )
target_compile_options( qc PUBLIC -fcoroutines-ts -stdlib=libc++ )
target_link_libraries( qc Qt5::Widgets -stdlib=libc++ )

foreach( ex mg ba cb cac )
    target_compile_options( ${ex} PUBLIC -fcoroutines-ts -stdlib=libc++ -D_LIBCPP_DEBUG_=1 )
    target_link_libraries( ${ex} PUBLIC -stdlib=libc++ )
endforeach()

if (${Boost_FOUND})
    # Asio as the execution queue *and* co_await
    add_executable( ac asio_coro.cpp )
    target_compile_options( ac PUBLIC -fcoroutines-ts -stdlib=libc++ -D_LIBCPP_DEBUG_=1 )
    target_link_libraries( ac PUBLIC -stdlib=libc++ PRIVATE Boost::boost pthread Boost::system )
else()
    message( WARNING "Boost not found; Asio example will not be built" )
endif()
