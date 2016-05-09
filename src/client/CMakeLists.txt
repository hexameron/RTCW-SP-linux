cmake_minimum_required(VERSION 2.8)

# initialize our variables
set(RENDERER_SOURCES)
set(QCOMMON_SOURCES)
set(UNIX_SOURCES)
set(SERVER_SOURCES)
set(CLIENT_SOURCES)

## Make lists of all the sources we need
# renderer sources
file(GLOB RENDERER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../renderer/*.c")
list(SORT RENDERER_SOURCES)

# common sources
file(GLOB QCOMMON_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../qcommon/*.c")
# Remove this for now.
list(REMOVE_ITEM QCOMMON_SOURCES "../qcommon/vm_x86.c")
list(SORT QCOMMON_SOURCES)
# message(STATUS "QCOMMON_SOURCES: ${QCOMMON_SOURCES}")

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

# message(STATUS "UNIX_SOURCES: ${UNIX_SOURCES}")

# Server sources
file(GLOB SERVER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../server/*.c")
list(SORT SERVER_SOURCES)
# message(STATUS "SERVER_SOURCES: ${SERVER_SOURCES}")

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
list(APPEND CLIENT_SOURCES ${RENDERER_SOURCES} ${QCOMMON_SOURCES} ${UNIX_SOURCES}
			${ADDITONAL_SOURCES} ${SERVER_SOURCES} ${SERVER_SOURCES})

set_source_files_properties(${CLIENT_SOURCES} PROPERTIES LANGUAGE C COMPILE_FLAGS "-Wno-unused")
# message(STATUS "CLIENT_SOURCES: ${CLIENT_SOURCES}")

# Compile our program
add_executable(${PROJECT_NAME} ${CLIENT_SOURCES})
add_dependencies(${PROJECT_NAME} botlib.a jpeg-6.a splines.a)

# Our executable
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C PREFIX "../../" SUFFIX "")
target_link_libraries(${PROJECT_NAME} SDL2 GL dl m botlib.a jpeg-6.a splines.a)
