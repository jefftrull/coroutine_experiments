cmake_minimum_required( VERSION 3.5.0 )
if ( ${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.15.0 )
    # Do not insert "/W3" by default into MSVC compile options.
    # This quietens warning level override warnings
    cmake_policy( SET CMP0092 NEW )
endif()

# create compile_commands.json for tools
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CMAKE_CXX_STANDARD 17 )

# dependencies
find_package( Boost 1.70 COMPONENTS system )
find_package( Qt5Widgets REQUIRED )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options( -Wall -Wextra -Werror )
    add_compile_options( -stdlib=libc++ )
    add_link_options( -stdlib=libc++ )
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_LIBCPP_DEBUG=1" )
    set( WITH_COROUTINES -fcoroutines-ts )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options( /W4 /WX )
    set( WITH_COROUTINES /await )
    if ( MSVC_VERSION LESS 1920 )
        add_compile_definitions( INTERNAL_VOID_SPECIALIZATION )
    endif()
endif()

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
target_link_libraries( qb Qt5::Widgets )

# Qt with coroutines
add_executable( qc qt_coro.cpp ${CR_MOC_SRC} colorrect.cpp )
target_link_libraries( qc Qt5::Widgets )

foreach( target mg ba cb cac qc )
    target_compile_options( ${target} PUBLIC ${WITH_COROUTINES} )
endforeach()

if (${Boost_FOUND})
    # Asio as the execution queue *and* co_await
    find_package( Threads REQUIRED )
    add_executable( ac asio_coro.cpp )
    target_compile_options( ac PUBLIC ${WITH_COROUTINES} )
    target_link_libraries( ac PRIVATE Boost::boost Threads::Threads Boost::system )
else()
    message( WARNING "Boost not found; Asio example will not be built" )
endif()
