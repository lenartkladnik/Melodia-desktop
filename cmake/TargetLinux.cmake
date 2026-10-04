find_package(PkgConfig REQUIRED)

pkg_check_modules(SFML_GRAPHICS REQUIRED IMPORTED_TARGET sfml-graphics)
pkg_check_modules(SFML_AUDIO    REQUIRED IMPORTED_TARGET sfml-audio)
pkg_check_modules(GTK3          REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(WEBKIT2GTK    REQUIRED IMPORTED_TARGET webkit2gtk-4.1)

target_link_libraries(${PROJECT_DISPLAY_NAME} PRIVATE
  PkgConfig::SFML_GRAPHICS
  PkgConfig::SFML_AUDIO
  PkgConfig::GTK3
  PkgConfig::WEBKIT2GTK
)

set(PLATFORM_NAME "linux")
