cmake_minimum_required(VERSION 3.16)

# Two targets:
#   equinox-pcsx-rearmed  the Switch .nro (built with the devkitPro toolchain file)
#   eqx_tests             host unit tests, with -DEQX_HOST_TESTS=ON

project(equinox-pcsx-rearmed LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_ALTER "0")
set(APP_TITLE "Equinox PS1")
set(APP_AUTHOR "Equinox")
set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}")
set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg)

option(EQX_HOST_TESTS "Build the host unit tests" OFF)

file(GLOB_RECURSE EQX_ALL_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(EQX_LOGIC_SRC ${EQX_ALL_SRC})
list(FILTER EQX_LOGIC_SRC EXCLUDE REGEX "/src/platform/")
# core.cpp calls into the libretro core, which is only linked for the Switch.
list(FILTER EQX_LOGIC_SRC EXCLUDE REGEX "/src/core\\.cpp$")

# Vendored miniz (MIT) writes the savestate pictures: memory APIs only, no zlib-name clashes.
add_compile_definitions(MINIZ_NO_STDIO MINIZ_NO_TIME MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
set(EQX_MINIZ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/extern/miniz/miniz.c)

if (EQX_HOST_TESTS)
    enable_testing()
    file(GLOB EQX_TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
    add_executable(eqx_tests ${EQX_LOGIC_SRC} ${EQX_MINIZ_SRC} ${EQX_TEST_SRC})
    target_include_directories(eqx_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/extern)
    find_package(Threads REQUIRED)
    target_link_libraries(eqx_tests PRIVATE Threads::Threads)
    add_test(NAME eqx_tests COMMAND eqx_tests)
endif ()

if (NINTENDO_SWITCH)
    # Partially linked by the core's libnx build: only the retro_* symbols are global, so
    # its bundled libretro-common, miniz and zstd never clash with the frontend.
    set(PCSX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/pcsx_rearmed)
    set(PCSX_LIB ${PCSX_DIR}/pcsx_rearmed_libretro_libnx.a)

    set(PORTLIBS_SWITCH /opt/devkitpro/portlibs/switch)

    add_executable(equinox-pcsx-rearmed ${EQX_ALL_SRC} ${EQX_MINIZ_SRC})
    target_link_directories(equinox-pcsx-rearmed PRIVATE ${PORTLIBS_SWITCH}/lib)
    target_include_directories(equinox-pcsx-rearmed PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/extern
        ${PORTLIBS_SWITCH}/include
        ${PORTLIBS_SWITCH}/include/SDL2
        ${PCSX_DIR}/deps/libretro-common/include)
    target_link_libraries(equinox-pcsx-rearmed PRIVATE
        ${PCSX_LIB} SDL2_ttf SDL2 freetype harfbuzz freetype png bz2 z EGL glapi drm_nouveau nx)

    nx_generate_nacp(equinox-pcsx-rearmed.nacp NAME "${APP_TITLE}" AUTHOR "${APP_AUTHOR}" VERSION "${APP_VERSION}")
    nx_create_nro(equinox-pcsx-rearmed NACP equinox-pcsx-rearmed.nacp ICON ${APP_ICON})
endif ()
