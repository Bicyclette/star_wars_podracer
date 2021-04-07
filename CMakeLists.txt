cmake_minimum_required(VERSION 3.16)

project(Podracer)

set(CMAKE_BUILD_TYPE Debug)

set(SRCS
	src/main.cpp
	src/color.cpp
	src/shader.cpp
	src/game.cpp
	src/mesh.cpp
	src/object.cpp
	src/joint.cpp
	src/animation.cpp
	src/skybox.cpp
	src/smoke.cpp
	src/power.cpp
	src/audio.cpp)

set(HEADERS
	include/color.hpp
	include/stb_image.hpp
	include/shader.hpp
	include/game.hpp
	include/mesh.hpp
	include/object.hpp
	include/joint.hpp
	include/animation.hpp
	include/skybox.hpp
	include/smoke.hpp
	include/power.hpp
	include/audio.hpp)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/glm-0.9.9.6)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

cmake_policy(SET CMP0072 NEW)

find_package(SDL2 REQUIRED)
if(SDL2_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
else()
	message(FATAL_ERROR "sdl2 not found")
endif()

find_package(GLEW REQUIRED)
if(GLEW_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
else()
	message(FATAL_ERROR "glew not found")
endif()

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY})
else()
	message(FATAL_ERROR "OpenGL not found")
endif()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
	target_link_libraries(${PROJECT_NAME} ${OpenMP_LD_FLAGS})
else()
	message(FATAL_ERROR "OpenMP not found")
endif()

find_package(ASSIMP REQUIRED)
if(ASSIMP_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
else()
	message(FATAL_ERROR "Assimp not found")
endif()

find_package(OpenAL REQUIRED)
if(OPENAL_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENAL_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${OPENAL_LIBRARY})
else()
	message(FATAL_ERROR "OpenAL not found")
endif()

find_package(LIBSNDFILE REQUIRED)
if(LIBSNDFILE_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${LIBSNDFILE_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${LIBSNDFILE_LIBRARIES})
else()
	message(FATAL_ERROR "libSndFile not found")
endif()

find_package(BULLET REQUIRED)
if(BULLET_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${BULLET_LIBRARIES})
else()
	message(FATAL_ERROR "Bullet not found")
endif()
