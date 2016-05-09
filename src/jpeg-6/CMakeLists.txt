cmake_minimum_required(VERSION 2.8)

# Get all C source files
file(GLOB C_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")

# Sort the C sources into a neat list
list(SORT C_SOURCES)

# Set the compiler to use
set_source_files_properties(${C_SOURCES} PROPERTIES LANGUAGE C COMPILE_FLAGS "-g")

# Add to our library.
add_library(jpeg-6.a STATIC ${C_SOURCES})
set_target_properties(jpeg-6.a PROPERTIES LINKER_LANGUAGE C PREFIX "" SUFFIX "")