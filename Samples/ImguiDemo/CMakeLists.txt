cmake_minimum_required (VERSION 3.10)

project(ImguiDemo CXX)

set(SOURCE
    src/ImguiDemo.cpp
    ${DILIGENT_DEAR_IMGUI_PATH}/imgui_demo.cpp
)

set(INCLUDE
    src/ImguiDemo.hpp
)

set(SHADERS
)

set(ASSETS
)

add_sample_app("ImguiDemo" "DiligentSamples/Samples" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
