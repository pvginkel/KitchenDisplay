cmake_minimum_required(VERSION 3.10)
project(KitchenDisplay)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
set(LV_CONF_BUILD_DISABLE_DEMOS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(ENABLE_CJSON_TEST OFF)
set(BUILD_TESTING OFF)

add_subdirectory(lib/lvgl)
add_subdirectory(lib/cJSON)
add_subdirectory(lib/cmark)

target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR}/src)

FILE(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(main ${SOURCES})

find_library(BACKTRACE_LIB backtrace HINTS ${PROJECT_SOURCE_DIR}/build/lib/libbacktrace/lib)
find_library(ICUUC_LIB icuuc HINTS ${PROJECT_SOURCE_DIR}/build/lib/icu/lib)
find_library(ICUI18N_LIB icui18n HINTS ${PROJECT_SOURCE_DIR}/build/lib/icu/lib)
find_library(ICUDATA_LIB icudata HINTS ${PROJECT_SOURCE_DIR}/build/lib/icu/lib)
find_library(CURL_LIB curl HINTS ${PROJECT_SOURCE_DIR}/build/lib/curl/lib)
find_library(OPENSSL_LIB ssl HINTS ${PROJECT_SOURCE_DIR}/build/lib/openssl/lib)
find_library(CRYPTO_LIB crypto HINTS ${PROJECT_SOURCE_DIR}/build/lib/openssl/lib)
find_library(ZLIB_LIB z HINTS ${PROJECT_SOURCE_DIR}/build/lib/zlib/lib)

target_link_libraries(main lvgl m pthread cmark cjson ${OPENSSL_LIB} ${CRYPTO_LIB} ${ZLIB_LIB} ${CURL_LIB} ${ICUUC_LIB} ${ICUI18N_LIB} ${ICUDATA_LIB} ${BACKTRACE_LIB})

target_include_directories(
    main PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lib/lvgl
    ${PROJECT_SOURCE_DIR}/lib/cJSON
    ${PROJECT_SOURCE_DIR}/lib/cmark/src
    ${PROJECT_SOURCE_DIR}/lib/curl/include
    ${PROJECT_SOURCE_DIR}/build/lib/cmark/src
    ${PROJECT_SOURCE_DIR}/build/lib/icu/include
    ${PROJECT_SOURCE_DIR}/build/lib/libbacktrace/include
    ${PROJECT_SOURCE_DIR}/build/lib/openssl/include
)

add_custom_target(copy_dependencies ALL
    COMMENT "Copying dependencies to ${PROJECT_SOURCE_DIR}/bin"
)

set(
    DEPENDENCIES
    ${ICUUC_LIB} ${ICUI18N_LIB} ${ICUDATA_LIB}
    ${OPENSSL_LIB} ${CRYPTO_LIB}
    ${ZLIB_LIB}
)

foreach(DEPENDENCY ${DEPENDENCIES})
    add_custom_command(
        TARGET copy_dependencies
        COMMAND cp -a -u ${DEPENDENCY}* ${PROJECT_SOURCE_DIR}/bin
        COMMENT "Copying ${DEPENDENCY}* to ${PROJECT_SOURCE_DIR}/bin"
    )
endforeach()
