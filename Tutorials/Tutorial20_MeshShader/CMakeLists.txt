cmake_minimum_required (VERSION 3.10)

project(Tutorial20_MeshShader CXX)

set(SOURCE
    src/Tutorial20_MeshShader.cpp
)

set(INCLUDE
    src/Tutorial20_MeshShader.hpp
)

set(SHADERS
    assets/cube.ash
    assets/cube.msh
    assets/cube.psh
    assets/structures.fxh
)

set(ASSETS
    assets/DGLogo.png
)

add_sample_app("Tutorial20_MeshShader" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
