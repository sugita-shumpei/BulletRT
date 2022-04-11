add_library(
    BulletRT_Core STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/BulletRT/Core/BulletRTCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/BulletRTCore.cpp
)

target_include_directories(
    BulletRT_Core 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

target_link_libraries(
    BulletRT_Core PUBLIC Vulkan::Vulkan glfw
)