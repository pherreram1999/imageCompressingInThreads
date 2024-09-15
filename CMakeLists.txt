cmake_minimum_required(VERSION 3.29)
project(ict C)

set(CMAKE_C_STANDARD 23)

# Encuentra los paquetes requeridos
find_package(PkgConfig REQUIRED)

# Usa PkgConfig para encontrar MagickWand
pkg_check_modules(MAGICKWAND REQUIRED MagickWand)


add_executable(${PROJECT_NAME} main.c)

# Incluye los directorios de MagickWand
target_include_directories(${PROJECT_NAME} PRIVATE ${MAGICKWAND_INCLUDE_DIRS})

# agrega la dependecia de la libreria math.h
target_link_libraries(${PROJECT_NAME} ${MAGICKWAND_LIBRARIES} m)


#Añade las banderas de compilación
target_compile_options(${PROJECT_NAME} PRIVATE ${MAGICKWAND_CFLAGS_OTHER})
