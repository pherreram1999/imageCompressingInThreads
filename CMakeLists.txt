cmake_minimum_required(VERSION 3.29)
project(ict C)

set(CMAKE_C_STANDARD 23)

# Requiere el uso de pkg-config
find_package(PkgConfig REQUIRED)

# Buscar GLib-2.0
pkg_check_modules(glib REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

# Buscar libvips
pkg_check_modules(vips REQUIRED vips)
include_directories(${VIPS_INCLUDE_DIRS})
link_directories(${VIPS_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} main.c)

# agrega la dependecia de la libreria math.h
target_link_libraries(${PROJECT_NAME} m)
