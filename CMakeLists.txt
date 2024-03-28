cmake_minimum_required(VERSION 3.10)
project(lvgl)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} lib/cmark/src build/lib/cmark/src)

add_subdirectory(lib/cJSON)

add_subdirectory(lib/cmark)

FILE(GLOB lv_font_files lv_font_*.c)

add_executable(
    main main.cpp mouse_cursor_icon.c Application.cpp Device.cpp lvgl_events.cpp MarkdownTestUI.cpp
    support.cpp TrelloApi.cpp CardUI.cpp HomeUI.cpp LvglUI.cpp MarkdownRenderer.cpp Queue.cpp Tasks.cpp
    ${lv_font_files})

target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread cmark cjson curl crypto icuuc icui18n)
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)
