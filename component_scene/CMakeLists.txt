cmake_minimum_required(VERSION 3.16)


find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(component_scene
  src/main.cpp src/glad.cpp
)

 
target_link_libraries(component_scene PRIVATE 
OpenGL::GL
irrlicht
SDL2::SDL2
)

 
   