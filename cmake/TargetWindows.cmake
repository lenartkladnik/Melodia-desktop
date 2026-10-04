include(ExternalProject)

set(BUILD_JOBS 4 CACHE STRING "Parallel build jobs for external dependencies")
set(DEPS_ROOT "${CMAKE_SOURCE_DIR}/external/build")
set(DEPS_BUILD_DIR "${DEPS_ROOT}/build-mingw-deps")

set(MINGW_BASE_CMAKE_ARGS
  -DCMAKE_SYSTEM_NAME=Windows
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
  -DBUILD_SHARED_LIBS=FALSE
  -DBUILD_TESTING=OFF
)

# libogg
ExternalProject_Add(dep_ogg
  GIT_REPOSITORY    https://github.com/xiph/ogg.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/libogg"
  BINARY_DIR        "${DEPS_BUILD_DIR}/ogg"
  CMAKE_ARGS        ${MINGW_BASE_CMAKE_ARGS}
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/ogg/lib/libogg.a"
)

# libvorbis (needs ogg)
ExternalProject_Add(dep_vorbis
  GIT_REPOSITORY    https://github.com/xiph/vorbis.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/libvorbis"
  BINARY_DIR        "${DEPS_BUILD_DIR}/vorbis"
  DEPENDS           dep_ogg
  CMAKE_ARGS        ${MINGW_BASE_CMAKE_ARGS}
                    -DOGG_INCLUDE_DIR=${DEPS_ROOT}/libogg/include
                    -DOGG_LIBRARY=${DEPS_BUILD_DIR}/ogg/lib/libogg.a
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/vorbis/lib/libvorbis.a"
                    "${DEPS_BUILD_DIR}/vorbis/lib/libvorbisfile.a"
)

# flac (needs ogg)
ExternalProject_Add(dep_flac
  GIT_REPOSITORY    https://github.com/xiph/flac.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/flac"
  BINARY_DIR        "${DEPS_BUILD_DIR}/flac"
  DEPENDS           dep_ogg
  CMAKE_ARGS        ${MINGW_BASE_CMAKE_ARGS}
                    -DINSTALL_MANPAGES=OFF
                    -DBUILD_PROGRAMS=OFF
                    -DBUILD_EXAMPLES=OFF
                    -DBUILD_DOCS=OFF
                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${DEPS_BUILD_DIR}/flac/lib
                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${DEPS_BUILD_DIR}/flac/lib
                    -DCMAKE_INSTALL_PREFIX=${DEPS_BUILD_DIR}/flac/install
                    -DOGG_INCLUDE_DIR=${DEPS_ROOT}/libogg/include
                    -DOGG_LIBRARY=${DEPS_BUILD_DIR}/ogg/lib/libogg.a
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/flac/lib/libFLAC.a"
)

# openal-soft
ExternalProject_Add(dep_openal
  GIT_REPOSITORY    https://github.com/kcat/openal-soft.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/openal-soft"
  BINARY_DIR        "${DEPS_BUILD_DIR}/openal-soft"
  CMAKE_ARGS        -DCMAKE_SYSTEM_NAME=Windows
                    -DBUILD_SHARED_LIBS=FALSE
                    -DALSOFT_EXAMPLES=OFF
                    -DALSOFT_UTILS=OFF
                    -DALSOFT_TESTS=OFF
                    -DALSOFT_BACKEND_PIPEWIRE=OFF
                    -DALSOFT_BACKEND_PULSEAUDIO=OFF
                    -DALSOFT_BACKEND_ALSA=OFF
                    -DALSOFT_BACKEND_OSS=OFF
                    -DALSOFT_BACKEND_SNDIO=OFF
                    -DALSOFT_INSTALL_CONFIG=OFF
                    -DALSOFT_REQUIRE_WINMM=ON
                    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc
                    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
                    -DBUILD_TESTING=OFF
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/openal-soft/libOpenAL32.a"
)

# harfbuzz
ExternalProject_Add(dep_harfbuzz
  GIT_REPOSITORY    https://github.com/harfbuzz/harfbuzz.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/harfbuzz"
  BINARY_DIR        "${DEPS_BUILD_DIR}/harfbuzz"
  CMAKE_ARGS        ${MINGW_BASE_CMAKE_ARGS}
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/harfbuzz/libharfbuzz.a"
)

# freetype
ExternalProject_Add(dep_freetype
  GIT_REPOSITORY    https://github.com/freetype/freetype.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/freetype"
  BINARY_DIR        "${DEPS_BUILD_DIR}/freetype"
  CMAKE_ARGS        ${MINGW_BASE_CMAKE_ARGS}
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/freetype/lib/libfreetype.a"
)

# OpenSSL
ExternalProject_Add(dep_openssl
  GIT_REPOSITORY    https://github.com/openssl/openssl.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${DEPS_ROOT}/openssl"
  BINARY_DIR        "${DEPS_BUILD_DIR}/openssl/build"
  CONFIGURE_COMMAND perl ${DEPS_ROOT}/openssl/Configure
                    mingw64 no-shared no-tests
                    --cross-compile-prefix=x86_64-w64-mingw32-
                    --prefix=${DEPS_BUILD_DIR}/openssl/install
                    --openssldir=${DEPS_BUILD_DIR}/openssl/install
  BUILD_COMMAND     make -j${BUILD_JOBS}
  INSTALL_COMMAND   make install_sw
  BUILD_IN_SOURCE   FALSE
  BUILD_BYPRODUCTS  "${DEPS_BUILD_DIR}/openssl/install/lib64/libssl.a"
                    "${DEPS_BUILD_DIR}/openssl/install/lib64/libcrypto.a"
)

# SFML
set(SFML_WIN_SRC "${DEPS_ROOT}/SFML")
set(SFML_WIN_BUILD "${SFML_WIN_SRC}/build-mingw")

ExternalProject_Add(dep_sfml
  GIT_REPOSITORY    https://github.com/SFML/SFML.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${SFML_WIN_SRC}"
  BINARY_DIR        "${SFML_WIN_BUILD}"
  DEPENDS           dep_ogg dep_vorbis dep_flac dep_openal dep_harfbuzz dep_freetype dep_openssl
  CMAKE_ARGS        ${MINGW_BASE_CMAKE_ARGS}
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${SFML_WIN_BUILD}/lib/libsfml-graphics-s.a"
                    "${SFML_WIN_BUILD}/lib/libsfml-audio-s.a"
                    "${SFML_WIN_BUILD}/lib/libsfml-window-s.a"
                    "${SFML_WIN_BUILD}/lib/libsfml-system-s.a"
)

add_dependencies(${PROJECT_DISPLAY_NAME} dep_sfml)

target_include_directories(${PROJECT_DISPLAY_NAME} PRIVATE
  "${SFML_WIN_SRC}/include"
  "${DEPS_ROOT}/libogg/include"
  "${DEPS_ROOT}/libvorbis/include"
  "${DEPS_ROOT}/flac/include"
  "${DEPS_ROOT}/openal-soft/include"
  "${DEPS_ROOT}/harfbuzz/include"
  "${DEPS_ROOT}/freetype/include"
  "${DEPS_BUILD_DIR}/openssl/install/include"
)

target_link_directories(${PROJECT_DISPLAY_NAME} PRIVATE
  "${SFML_WIN_BUILD}/lib"
  "${DEPS_BUILD_DIR}/ogg/lib"
  "${DEPS_BUILD_DIR}/vorbis/lib"
  "${DEPS_BUILD_DIR}/flac/lib"
  "${DEPS_BUILD_DIR}/openal-soft"
  "${DEPS_BUILD_DIR}/harfbuzz"
  "${DEPS_BUILD_DIR}/freetype/lib"
  "${DEPS_BUILD_DIR}/openssl/install/lib64"
)

target_link_libraries(${PROJECT_DISPLAY_NAME} PRIVATE
  sfml-graphics-s sfml-audio-s sfml-window-s sfml-system-s
  ogg vorbis vorbisfile FLAC
  OpenAL32 opengl32 winmm gdi32
  harfbuzz freetype
  crypt32 ws2_32
  libbcrypt libicudt
)

target_compile_definitions(${PROJECT_DISPLAY_NAME} PRIVATE SFML_STATIC)

target_compile_options(${PROJECT_DISPLAY_NAME} PRIVATE -fpermissive)

set_target_properties(${PROJECT_DISPLAY_NAME} PROPERTIES
    SUFFIX ".exe"
)

set(PLATFORM_NAME "windows")

