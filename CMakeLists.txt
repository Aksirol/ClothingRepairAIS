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
# Збираємо всі початкові файли
set(PROJECT_SOURCES
        main.cpp
        DatabaseManager.cpp
        DatabaseManager.h
        resources.qrc

        # Models
        src/models/OrderStatusIds.h
        src/models/Position.h
        src/models/Employee.h
        src/models/RepairCategory.h
        src/models/RepairService.h
        src/models/OrderStatus.h
        src/models/Client.h
        src/models/Order.h
        src/models/OrderItem.h

        # Repositories
        src/repositories/PositionRepository.cpp
        src/repositories/PositionRepository.h
        src/repositories/EmployeeRepository.cpp
        src/repositories/EmployeeRepository.h
        src/repositories/RepairCategoryRepository.cpp
        src/repositories/RepairCategoryRepository.h
        src/repositories/RepairServiceRepository.cpp
        src/repositories/RepairServiceRepository.h
        src/repositories/OrderStatusRepository.cpp
        src/repositories/OrderStatusRepository.h
        src/repositories/ClientRepository.cpp
        src/repositories/ClientRepository.h
        src/repositories/OrderRepository.cpp
        src/repositories/OrderRepository.h
        src/repositories/OrderItemRepository.cpp
        src/repositories/OrderItemRepository.h

        # Managers
        src/managers/OrderManager.cpp
        src/managers/OrderManager.h
        src/managers/ClientManager.cpp
        src/managers/ClientManager.h
        src/managers/ReportBuilder.cpp
        src/managers/ReportBuilder.h

        # UI
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/ui/CategoriesTab.cpp
        src/ui/CategoriesTab.h
        src/ui/PositionsTab.cpp
        src/ui/PositionsTab.h
        src/ui/ReferencesTab.cpp
        src/ui/ReferencesTab.h
        src/ui/StatusesTab.cpp
        src/ui/StatusesTab.h
        src/ui/ClientsTab.cpp
        src/ui/ClientsTab.h
        src/ui/EmployeesTab.cpp
        src/ui/EmployeesTab.h
)

include_directories(src/models src/repositories src/managers src/ui)

# Створюємо виконувальний файл
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Підключаємо бібліотеки Qt до нашого проекту
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
)