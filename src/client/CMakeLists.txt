cmake_minimum_required(VERSION 2.8)

# initialize our variables
set(RENDERER_SOURCES)
set(QCOMMON_SOURCES)
set(BOTLIB_SOURCES)
set(UNIX_SOURCES)
set(JPEG_SOURCES)
set(SERVER_SOURCES)
set(CLIENT_SOURCES)

## Make lists of all the sources we need
file(GLOB RENDERER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../renderer/*.c")
file(GLOB QCOMMON_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../qcommon/*.c")
file(GLOB SERVER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../server/*.c")
file(GLOB BOTLIB_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../botlib/*.c")
file(GLOB JPEG_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../jpeg-6/*.c")
list(SORT RENDERER_SOURCES)
list(SORT QCOMMON_SOURCES)
list(SORT SERVER_SOURCES)
list(SORT BOTLIB_SOURCES)
list(SORT JPEG_SOURCES)

# Remove this for now.
list(REMOVE_ITEM QCOMMON_SOURCES "../qcommon/vm_x86.c")

set(UNIX_SOURCES
	${CMAKE_SOURCE_DIR}/src/unix/unix_main.c
	${CMAKE_SOURCE_DIR}/src/unix/unix_net.c
	${CMAKE_SOURCE_DIR}/src/unix/unix_shared.c
	${CMAKE_SOURCE_DIR}/src/unix/linux_common.c
	${CMAKE_SOURCE_DIR}/src/unix/sdl2_gamma.c
	${CMAKE_SOURCE_DIR}/src/unix/sdl2_glimp.c
	${CMAKE_SOURCE_DIR}/src/unix/sdl2_input.c
	${CMAKE_SOURCE_DIR}/src/unix/sdl2_snd.c
)

# Get all the client .c files for compile
file(GLOB CLIENT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")
list(SORT CLIENT_SOURCES)
# message(STATUS "CLIENT_SOURCES: ${CLIENT_SOURCES}")

set(ADDITONAL_SOURCES
	${CMAKE_SOURCE_DIR}/src/game/q_shared.c
	${CMAKE_SOURCE_DIR}/src/game/q_math.c
)
# message(STATUS "ADDITIONAL_SOURCES: ${ADDITIONAL_SOURCES}")
# Append all of our sources to one list and set it ready to compile.
list(APPEND CLIENT_SOURCES ${RENDERER_SOURCES} ${QCOMMON_SOURCES} ${UNIX_SOURCES})
list(APPEND CLIENT_SOURCES ${BOTLIB_SOURCES} ${JPEG_SOURCES} ${ADDITONAL_SOURCES} ${SERVER_SOURCES})

# TODO: Fix all warnings
set_source_files_properties(${CLIENT_SOURCES} PROPERTIES LANGUAGE C COMPILE_FLAGS "-Wno-unused")

# Compile our program
add_executable(${PROJECT_NAME} ${CLIENT_SOURCES})
add_definitions(-DBOTLIB)

# Our executable
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C PREFIX "../../" SUFFIX "")
target_link_libraries(${PROJECT_NAME} SDL2 GL dl m splines.a game.a)
