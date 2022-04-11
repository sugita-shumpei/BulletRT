add_library(
    BulletRT_Utils STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/BulletRT/Utils/VulkanStaging.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/VulkanStaging.cpp
)

target_include_directories(
    BulletRT_Utils 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

target_link_libraries(
    BulletRT_Utils PUBLIC BulletRT_Core
)