cmake_minimum_required(VERSION 3.24)

project(limitless_gui LANGUAGES CXX)

option(LIMITLESS_GUI_BUILD_EXAMPLE "Build the Windows Direct3D 11 preview" OFF)
option(LIMITLESS_GUI_BUILD_PREVIEW "Deprecated alias for LIMITLESS_GUI_BUILD_EXAMPLE" OFF)

if(LIMITLESS_GUI_BUILD_PREVIEW)
    set(LIMITLESS_GUI_BUILD_EXAMPLE ON)
endif()

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)
add_library(limitless_gui::imgui ALIAS imgui)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_compile_features(imgui PUBLIC cxx_std_20)

add_library(limitless_gui STATIC
    src/limitless_gui/renderer.cpp
)
add_library(limitless_gui::limitless_gui ALIAS limitless_gui)
add_library(limitless_gui_core ALIAS limitless_gui)
target_include_directories(limitless_gui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(limitless_gui PUBLIC cxx_std_20)
target_link_libraries(limitless_gui PUBLIC limitless_gui::imgui)

if(WIN32)
    target_link_libraries(limitless_gui PUBLIC shell32)
endif()

if(LIMITLESS_GUI_BUILD_EXAMPLE)
    if(NOT WIN32)
        message(FATAL_ERROR "The preview example requires Windows and Direct3D 11.")
    endif()

    add_executable(limitless_gui_preview WIN32
        examples/preview/main.cpp
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_link_libraries(limitless_gui_preview PRIVATE limitless_gui d3d11 dxgi dwmapi)
    target_compile_definitions(limitless_gui_preview PRIVATE UNICODE _UNICODE)
    add_custom_command(TARGET limitless_gui_preview POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:limitless_gui_preview>/resources"
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(limitless_gui_tests tests/limitless_interactions.cpp)
    target_link_libraries(limitless_gui_tests PRIVATE limitless_gui)
    target_compile_definitions(limitless_gui_tests PRIVATE
        LIMITLESS_GUI_TEST_FONT="${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/PreviewSans.ttf"
    )
    add_test(NAME limitless_gui_tests COMMAND limitless_gui_tests)
endif()
