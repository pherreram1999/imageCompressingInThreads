cmake_minimum_required(VERSION 3.29)
project(ict C)

set(CMAKE_C_STANDARD 23)

# Encuentra los paquetes requeridos
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(VIPS REQUIRED vips)


add_executable(${PROJECT_NAME} main.c)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLIB_INCLUDE_DIRS} ${VIPS_INCLUDE_DIRS})

# agrega la dependecia de la libreria math.h
target_link_libraries(${PROJECT_NAME} ${GLIB_LIBRARIES} ${VIPS_LIBRARIES} m)

# Ayuda a CLion con la indexación
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configura variables para que CLion pueda encontrar las definiciones
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${GLIB_CFLAGS_OTHER} ${VIPS_CFLAGS_OTHER})