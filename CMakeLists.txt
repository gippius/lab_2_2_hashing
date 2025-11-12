cmake_minimum_required(VERSION 3.16)
project(lab_2_hash_table LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# чуть-чуть варнингов, но без фанатизма
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# пути
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# на всякий случай создадим папку для выгрузок
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

# ===== библиотека структур данных (хеш-таблица) =====
add_library(ds
        ${SRC_DIR}/ds/hash_table.cpp
        ${SRC_DIR}/ds/hash_func.cpp
        ${SRC_DIR}/ds/key_utils.cpp
        ${SRC_DIR}/ds/exporter.cpp
)
target_include_directories(ds PUBLIC ${SRC_DIR})   # чтобы подключать "ds/....hpp"

# ===== библиотека приложения (меню) =====
add_library(app
        ${SRC_DIR}/app/menu.cpp
)
target_link_libraries(app PRIVATE ds)
target_include_directories(app PUBLIC ${SRC_DIR})

# ===== исполняемый файл =====
add_executable(lab_2_hash_table
        ${SRC_DIR}/app/main.cpp
)
target_link_libraries(lab_2_hash_table PRIVATE app)

# опционально: куда класть бинарь (удобно для CLion/VSCode)
set_target_properties(lab_2_hash_table PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
