# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("altv-weapon-tool")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/vendors/rapidxml"
	"${PROJECT_SOURCE_DIR}/vendors/pugixml/src"
	"${PROJECT_SOURCE_DIR}/vendors/sdl2/include"
	"${PROJECT_SOURCE_DIR}/vendors/imgui"
)
link_directories(
#	${PROJECT_SOURCE_DIR}/vendors/glfw337/lib
	${PROJECT_SOURCE_DIR}/vendors/sdl2/lib
)

file(
	GLOB_RECURSE PROJECT_SOURCE_FILES
	"./src/*.h" "./src/*.hpp" "./src/*.cpp"
	"./vendors/imgui/*.h" "./vendors/imgui/*.hpp" "./vendors/imgui/*.cpp"
	"./vendors/pugixml/src/*.h" "./vendors/pugixml/src/*.hpp" "./vendors/pugixml/src/*.cpp"
)
add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} opengl32 x64/SDL2 x64/SDL2main x64/SDL2_image)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/vendors/sdl2/lib/x64/SDL2.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/resources/fonts/ $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/fonts)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/resources/images/ $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/images)

file(GLOB SDL_LIBRARIES "./vendors/sdl2/lib/x64/*.dll")
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        ${SDL_LIBRARIES}      # <--this is in-file
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)