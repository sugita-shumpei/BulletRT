add_subdirectory(glfw)
add_subdirectory(SPIRV-Cross)
set_target_properties(
    spirv-cross 
    spirv-cross-c 
    spirv-cross-core
    spirv-cross-cpp
    spirv-cross-glsl
    spirv-cross-hlsl
    spirv-cross-msl
    spirv-cross-reflect
    spirv-cross-util
    PROPERTIES FOLDER SPIRV-Cross
)