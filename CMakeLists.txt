cmake_minimum_required( VERSION 3.5.0 )

set( CMAKE_CXX_STANDARD 17 )

add_executable( mg manual_generator.cpp )
target_compile_options( mg PUBLIC -fcoroutines-ts -stdlib=libc++ )
target_link_libraries( mg PUBLIC -stdlib=libc++ )

add_executable( ba basic_awaiter.cpp )
target_compile_options( ba PUBLIC -fcoroutines-ts -stdlib=libc++ )
target_link_libraries( ba PUBLIC -stdlib=libc++ )
