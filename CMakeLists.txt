cmake_minimum_required(VERSION 3.16)
project(MyEngineAll LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(USE_SYSTEM_LIBS "Use system libs instead of vendor/ThirdParty" ON)

# Subprojetos
add_subdirectory(vendor/ThirdParty)       
add_subdirectory(vendor/irrlicht)        

# Apps
#add_subdirectory(teste)
add_subdirectory(component_scene)