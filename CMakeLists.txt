cmake_minimum_required(VERSION 3.16)
project(MyEngineAll LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opção: usar libs do sistema em vez do vendor (se quiseres mudar no futuro)
option(USE_SYSTEM_LIBS "Use system libs instead of vendor/ThirdParty" OFF)

# Subprojetos
add_subdirectory(vendor/ThirdParty)      # lib ThirdParty
add_subdirectory(vendor/irrlicht)        # lib irrlicht (liga à ThirdParty)

# Apps
add_subdirectory(teste)
add_subdirectory(component_scene)