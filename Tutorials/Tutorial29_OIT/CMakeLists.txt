cmake_minimum_required (VERSION 3.10)

project(Tutorial29_OIT CXX)

set(SOURCE
    src/Tutorial29_OIT.cpp
)

set(INCLUDE
    src/Tutorial29_OIT.hpp
)

set(SHADERS
    assets/common.fxh
    assets/oit.fxh
    assets/geometry.vsh
    assets/alpha_blend.psh
    assets/clear_oit_layers.csh
    assets/update_oit_layers.psh
    assets/attenuate_background.psh
    assets/layered_oit_blend.psh
    assets/screen_triangle.vsh
    assets/weighted_blend.psh
    assets/weighted_resolve.psh
)

set(ASSETS
)

add_sample_app("Tutorial29_OIT" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
