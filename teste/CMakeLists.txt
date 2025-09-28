cmake_minimum_required(VERSION 3.16)


find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(teste
  src/main.cpp
)

 
target_link_libraries(teste PRIVATE 
OpenGL::GL
irrlicht
SDL2::SDL2
)

 
# target_include_directories(teste PRIVATE ${CMAKE_SOURCE_DIR}/vendor/ThirdParty)
