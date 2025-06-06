cmake_minimum_required (VERSION 3.10)

project(Diligent-SampleBase)

if(PLATFORM_WIN32)
    set(SOURCE 
        src/Win32/SampleAppWin32.cpp
        src/Win32/InputControllerWin32.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE 
        include/SampleApp.hpp
        include/Win32/InputControllerWin32.hpp
    )
    set(WIN32_RESOURCES
        src/Win32/resources/Win32AppResource.h
        src/Win32/resources/directx11-logo.bmp
        src/Win32/resources/directx12-logo.bmp
        src/Win32/resources/vulkan-logo.bmp
        src/Win32/resources/opengl-logo.bmp
        src/Win32/resources/webgpu-logo.bmp
        src/Win32/resources/Win32AppResource.rc
    )

    function(append_sample_base_win32_source TARGET_NAME)
        get_target_property(SAMPLE_BASE_SOURCE_DIR Diligent-SampleBase SOURCE_DIR)
        # We need to add .res file to executable target to make resources available
        set(RES_FILE ${SAMPLE_BASE_SOURCE_DIR}/src/Win32/resources/Win32AppResource.rc)
        target_sources(${TARGET_NAME} PRIVATE ${RES_FILE})
        source_group("resources" FILES ${RES_FILE})
    endfunction()

elseif(PLATFORM_UNIVERSAL_WINDOWS)

    # Windows Runtime types cannot be included into static libraries
    # https://social.msdn.microsoft.com/Forums/en-US/269db513-64ef-4817-a025-43954f614eb3/lnk4264-why-are-static-libraries-not-recommended-when-authoring-windows-runtime-types?forum=winappswithnativecode
    # So as a workaround, we will include all source files into the target app project
    function(append_sample_base_uwp_source TARGET_NAME)
        get_target_property(SAMPLE_BASE_SOURCE_DIR Diligent-SampleBase SOURCE_DIR)

        set(SAMPLE_BASE_UWP_SOURCE
            ${SAMPLE_BASE_SOURCE_DIR}/src/UWP/ImguiUWPEventHelper.cpp
            ${SAMPLE_BASE_SOURCE_DIR}/src/UWP/SampleAppUWP.cpp
            ${SAMPLE_BASE_SOURCE_DIR}/src/UWP/InputControllerEventHandlerUWP.cpp
            ${SAMPLE_BASE_SOURCE_DIR}/src/SampleApp.cpp
        )

        set(SAMPLE_BASE_UWP_INCLUDE
            ${SAMPLE_BASE_SOURCE_DIR}/src/UWP/ImguiUWPEventHelper.h
            ${SAMPLE_BASE_SOURCE_DIR}/src/UWP/InputControllerEventHandlerUWP.h
            ${SAMPLE_BASE_SOURCE_DIR}/include/SampleApp.hpp
            ${SAMPLE_BASE_SOURCE_DIR}/include/UWP/InputControllerUWP.hpp
        )

        set(SAMPLE_BASE_UWP_INCLUDE_DIR
            ${SAMPLE_BASE_SOURCE_DIR}/src/UWP
        )

        target_sources(${TARGET_NAME} PRIVATE ${SAMPLE_BASE_UWP_SOURCE} ${SAMPLE_BASE_UWP_INCLUDE})
        source_group("src\\SampleBase" FILES ${SAMPLE_BASE_UWP_SOURCE})
        source_group("include\\SampleBase" FILES ${SAMPLE_BASE_UWP_INCLUDE})
        target_include_directories(${TARGET_NAME} PRIVATE ${SAMPLE_BASE_UWP_INCLUDE_DIR})
    endfunction()

elseif(PLATFORM_ANDROID)
    set(SOURCE
        src/Android/InputControllerAndroid.cpp
        src/Android/SampleAppAndroid.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE 
        include/Android/InputControllerAndroid.hpp
        include/SampleApp.hpp
    )
elseif(PLATFORM_LINUX)
    set(SOURCE 
        src/Linux/InputControllerLinux.cpp
        src/Linux/SampleAppLinux.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE 
        include/Linux/InputControllerLinux.hpp
        include/SampleApp.hpp
    )
elseif(PLATFORM_MACOS)

    set(SOURCE
        src/MacOS/InputControllerMacOS.cpp
        src/MacOS/SampleAppMacOS.mm
        src/SampleApp.cpp
    )
    set(INCLUDE
        Include/MacOS/InputControllerMacOS.hpp
        include/SampleApp.hpp
    )

elseif(PLATFORM_IOS)
    set(SOURCE
        src/IOS/InputControllerIOS.cpp
        src/IOS/SampleAppIOS.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE
        include/IOS/InputControllerIOS.hpp
        include/SampleApp.hpp
    )

elseif(PLATFORM_TVOS)
    set(SOURCE
        src/TVOS/SampleAppTVOS.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE
        include/SampleApp.hpp
    )

elseif(PLATFORM_WEB)
    set(SOURCE
        src/Emscripten/InputControllerEmscripten.cpp
        src/Emscripten/SampleAppEmscripten.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE
        include/Emscripten/InputControllerEmscripten.hpp
        include/SampleApp.hpp
    )

    function(append_sample_base_emscripten_source TARGET_NAME)     
        get_target_property(SAMPLE_BASE_SOURCE_DIR Diligent-SampleBase SOURCE_DIR)

        if(TARGET_NAME STREQUAL "Atmosphere" OR
           TARGET_NAME STREQUAL "Shadows" OR
           TARGET_NAME STREQUAL "Tutorial14_ComputeShader" OR
           TARGET_NAME STREQUAL "Tutorial16_BindlessResources" OR
           TARGET_NAME STREQUAL "Tutorial26_StateCache" OR
           TARGET_NAME STREQUAL "Tutorial29_OIT")
            set(DILIGENT_DISABLE_WEBGL "disabled")
        endif()

        if (DILIGENT_EMSCRIPTEN_INCLUDE_COI_SERVICE_WORKER)
            set(DILIGENT_COI_SERVICE_WORKER_SCRIPT "<script src=\"coi-serviceworker.js\"></script>")
            file(COPY ${SAMPLE_BASE_SOURCE_DIR}/src/Emscripten/resources/coi-serviceworker.js DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        else()
            set(DILIGENT_COI_SERVICE_WORKER_SCRIPT "")
        endif()

        configure_file(
            ${SAMPLE_BASE_SOURCE_DIR}/src/Emscripten/resources/emscripten_template.html.in
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.html
            @ONLY
        )
    endfunction()
    
else()
    message(FATAL_ERROR "Unknown platform")
endif()

list(APPEND SOURCE
    src/FirstPersonCamera.cpp
    src/SampleBase.cpp
)

list(APPEND INCLUDE
    include/FirstPersonCamera.hpp
    include/TrackballCamera.hpp
    include/InputController.hpp
    include/SampleBase.hpp
)


add_library(Diligent-SampleBase STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(Diligent-SampleBase)

target_include_directories(Diligent-SampleBase
PUBLIC
    include
)

if(MSVC)
    target_compile_options(Diligent-SampleBase PRIVATE -DUNICODE)

    if(PLATFORM_UNIVERSAL_WINDOWS)
        # Disable w4189: local variable is initialized but not referenced
        # Disable w4063: case is not a valid value for switch of enum
        # Consume the windows runtime extensions (/ZW)
        target_compile_options(Diligent-SampleBase INTERFACE /wd4189 /wd4063 /ZW)
    endif()
endif()

get_supported_backends(ENGINE_LIBRARIES)

target_link_libraries(Diligent-SampleBase 
PRIVATE 
    Diligent-BuildSettings
PUBLIC
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-TextureLoader
    Diligent-TargetPlatform
    Diligent-Imgui
    Diligent-GraphicsAccessories
    ${ENGINE_LIBRARIES}
    Diligent-NativeAppBase
)

if(PLATFORM_UNIVERSAL_WINDOWS)
    target_link_libraries(Diligent-SampleBase PRIVATE dxguid.lib)
elseif(PLATFORM_ANDROID)
    target_link_libraries(Diligent-SampleBase PRIVATE GLESv3 PUBLIC native_app_glue)
elseif(PLATFORM_LINUX)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Diligent-SampleBase PRIVATE XCBKeySyms OpenGL::GL OpenGL::GLX X11::X11)
elseif(PLATFORM_MACOS OR PLATFORM_IOS)

endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

if(PLATFORM_WIN32)
    target_sources(Diligent-SampleBase PRIVATE ${WIN32_RESOURCES})
    source_group("resources" FILES ${WIN32_RESOURCES})
endif()

set_target_properties(Diligent-SampleBase PROPERTIES
    FOLDER DiligentSamples
)
