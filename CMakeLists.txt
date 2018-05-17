cmake_minimum_required( VERSION 3.5.0 )

set( CMAKE_CXX_STANDARD 17 )

add_executable( mg manual_generator.cpp )
add_executable( ba basic_awaiter.cpp )
add_executable( cb callbacks.cpp run_queue.cpp )

foreach( ex mg ba cb )
    target_compile_options( ${ex} PUBLIC -fcoroutines-ts -stdlib=libc++ )
    target_link_libraries( ${ex} PUBLIC -stdlib=libc++ )
endforeach()
