cmake_minimum_required(VERSION 3.21)
project(ClothingRepairAIS)

# Встановлюємо стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Вмикаємо автоматичну обробку файлів Qt (MOC, UIC, RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Шукаємо Qt 6 та потрібні нам компоненти:
# Widgets - для графічного інтерфейсу
# Sql - для роботи з базою даних SQLite
find_package(Qt6 COMPONENTS Core Gui Widgets Sql REQUIRED)

# Збираємо всі початкові файли (поки що лише main.cpp)
set(PROJECT_SOURCES
        main.cpp
)

# Створюємо виконувальний файл
add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})

# Підключаємо бібліотеки Qt до нашого проекту
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
)