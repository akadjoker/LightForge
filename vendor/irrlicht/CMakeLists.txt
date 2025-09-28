cmake_minimum_required(VERSION 3.16)
project(IrrlichtFork LANGUAGES CXX)

# Só dependências de janela/render do sistema
find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)
find_path(XRANDR_INCLUDE_DIR X11/extensions/Xrandr.h)
find_library(XRANDR_LIBRARY Xrandr)
find_library(XXF86VM_LIBRARY Xxf86vm)

set(IRR_ROOT ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE IRR_SOURCES ${IRR_ROOT}/source/*.cpp)
list(FILTER IRR_SOURCES EXCLUDE REGEX ".*/(D3D9|D3D10|D3D11|D3D12|Direct3D|Win32)/.*")
add_library(irrlicht STATIC ${IRR_SOURCES})
set_target_properties(irrlicht PROPERTIES POSITION_INDEPENDENT_CODE ON)

# target_compile_definitions(irrlicht PRIVATE
#   _IRR_STATIC_LIB_
#   _IRR_COMPILE_WITH_X11_
#   _IRR_LINUX_PLATFORM_
#   _IRR_POSIX_API
#   _POSIX_C_SOURCE=200809L    
# )


target_compile_definitions(irrlicht PRIVATE
  LINUX
  _IRR_STATIC_LIB_
  #_IRR_COMPILE_WITH_X11_
  _IRR_LINUX_PLATFORM_   
  _IRR_POSIX_API         
  _POSIX_C_SOURCE=200809L 
)


# Só headers do irrlicht aqui
target_include_directories(irrlicht
  PUBLIC  ${IRR_ROOT}/include
  PUBLIC  ${IRR_ROOT}/source
  PRIVATE ${X11_INCLUDE_DIR} ${XRANDR_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}
)

# Liga ao pacote ThirdParty (compilado de fontes) + X11/OpenGL
target_link_libraries(irrlicht
  PUBLIC  ThirdParty
  PRIVATE ${X11_LIBRARIES} ${OPENGL_gl_LIBRARY} ${XRANDR_LIBRARY} ${XXF86VM_LIBRARY} pthread
)
