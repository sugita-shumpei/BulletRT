add_executable(Test0  
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/Test0.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/Test0.cpp
)
target_include_directories(Test0 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Inc)
target_link_libraries(Test0 PUBLIC BulletRT_Core BulletRT_Utils)
set_target_properties(
    Test0 PROPERTIES FOLDER Test/Test0
)