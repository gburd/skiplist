cmake_minimum_required(VERSION 3.10)

# Project name
project(skiplist C CXX)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Og -g -std=c99 -fPIC")

# Include directories
include_directories(include)

# Test flags
set(TEST_FLAGS -Itests/)

# Source files and object files
set(OBJS)  # Add your object files here
set(STATIC_LIB libmystatic.a)  # Static library output name
set(SHARED_LIB libmyshared.so)  # Shared library output name
set(TESTS tests/test)  # Test executables
set(TEST_OBJS tests/test.o tests/munit.o)  # Test object files
set(EXAMPLES examples/ex1 examples/ex2)  # Example executables

# Add static library
add_library(${STATIC_LIB} STATIC ${OBJS})
target_compile_options(${STATIC_LIB} PRIVATE ${CMAKE_C_FLAGS})

# Add shared library
add_library(${SHARED_LIB} SHARED ${OBJS})
target_compile_options(${SHARED_LIB} PRIVATE ${CMAKE_C_FLAGS})

# Add example targets
foreach(EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE} ${EXAMPLE}.c)
    target_link_libraries(${EXAMPLE} ${STATIC_LIB})
    target_compile_options(${EXAMPLE} PRIVATE ${CMAKE_C_FLAGS})
endforeach()

# Add tests
add_executable(${TESTS} ${TEST_OBJS})
target_link_libraries(${TESTS} ${STATIC_LIB} pthread m)
target_compile_options(${TESTS} PRIVATE ${CMAKE_C_FLAGS} ${TEST_FLAGS})

# Format command
add_custom_target(format
    COMMAND clang-format -i include/*.h src/*.c tests/*.c tests/*.h examples/*.c
)

# Clean target
add_custom_target(clean
    COMMAND rm -f ${OBJS} munit.o test.o
    COMMAND rm -f examples/mls.c
    COMMAND rm -f ${STATIC_LIB}
    COMMAND rm -f ${SHARED_LIB}
    COMMAND rm -f ${TESTS}
    COMMAND rm -f ${EXAMPLES}
)

# Create a custom target for 'examples/mls' as a special case
add_custom_command(OUTPUT examples/mls.c
    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -C -E examples/ex1.c | sed -e '1,7d' -e '/^# [0-9]* "/d' | clang-format > examples/mls.c
    DEPENDS examples/ex1.c
    COMMENT "Generating examples/mls.c from examples/ex1.c"
)

# Add executable for 'mls'
add_executable(examples/mls examples/mls.c)
target_link_libraries(examples/mls ${STATIC_LIB})
target_compile_options(examples/mls PRIVATE ${CMAKE_C_FLAGS} ${TEST_FLAGS} pthread m)

# Add executable for 'ex2_sl'
add_executable(examples/ex2_sl examples/ex2_sl.c)
target_link_libraries(examples/ex2_sl ${STATIC_LIB})
target_compile_options(examples/ex2_sl PRIVATE ${CMAKE_C_FLAGS} ${TEST_FLAGS} pthread m)

# Optional: Add mls-related custom command for postprocessing (if required)
# add_custom_target(dot
#     COMMAND dot -Tpdf /tmp/ex1.dot -o /tmp/ex1.pdf
#     DEPENDS examples/mls
# )
