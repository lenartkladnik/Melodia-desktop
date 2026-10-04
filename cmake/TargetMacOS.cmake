include(ExternalProject)

if(NOT CMAKE_CXX_COMPILER MATCHES "clang")
  message(WARNING "macOS build expects CMAKE_CXX_COMPILER=clang++, set with -DCMAKE_CXX_COMPILER=clang++")
endif()

set(BUILD_JOBS 4 CACHE STRING "Parallel build jobs for external dependencies")

set(SFML_MAC_SRC "${CMAKE_SOURCE_DIR}/external/build/SFML")
set(SFML_MAC_BUILD "${SFML_MAC_SRC}/build-macos")

ExternalProject_Add(dep_sfml_macos
  GIT_REPOSITORY    https://github.com/SFML/SFML.git
  GIT_SHALLOW       TRUE
  SOURCE_DIR        "${SFML_MAC_SRC}"
  BINARY_DIR        "${SFML_MAC_BUILD}"
  CMAKE_ARGS        -DCMAKE_BUILD_TYPE=Release
                    -DBUILD_SHARED_LIBS=FALSE
                    -DBUILD_TESTING=OFF
                    -DMBEDTLS_THREADING_C=ON
                    -DMBEDTLS_THREADING_PTHREAD=ON
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . -j${BUILD_JOBS}
  INSTALL_COMMAND   ""
  BUILD_BYPRODUCTS  "${SFML_MAC_BUILD}/lib/libsfml-graphics.a"
                    "${SFML_MAC_BUILD}/lib/libsfml-audio.a"
                    "${SFML_MAC_BUILD}/lib/libsfml-window.a"
                    "${SFML_MAC_BUILD}/lib/libsfml-system.a"
)

add_dependencies(${PROJECT_DISPLAY_NAME} dep_sfml_macos)

target_link_libraries(${PROJECT_DISPLAY_NAME} PRIVATE
  sfml-graphics sfml-audio sfml-window sfml-system
)

target_compile_options(${PROJECT_DISPLAY_NAME} PRIVATE -x objective-c++)

target_include_directories(${PROJECT_DISPLAY_NAME} PRIVATE "${SFML_MAC_SRC}/include")
target_link_directories(${PROJECT_DISPLAY_NAME} PRIVATE "${SFML_MAC_BUILD}/lib")

set(PLATFORM_NAME "macos")
