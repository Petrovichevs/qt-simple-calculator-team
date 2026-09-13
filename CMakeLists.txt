cmake_minimum_required(VERSION 3.16)
project(simpleCalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 QUIET COMPONENTS Widgets Test)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets Test)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# ===== Основное приложение =====
add_executable(simpleCalculator
    main.cpp
    mainwindow.cpp
    mainwindow.h
    expression.cpp
    expression.h
    mainwindow.ui
)

if (QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(simpleCalculator PRIVATE Qt6::Widgets)
else()
    target_link_libraries(simpleCalculator PRIVATE Qt5::Widgets)
endif()

# ===== Unit-тесты =====
enable_testing()

add_executable(expression_tests
    tests/test_expression.cpp
    expression.cpp
    expression.h
)

if (QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(expression_tests PRIVATE Qt6::Test Qt6::Core)
else()
    target_link_libraries(expression_tests PRIVATE Qt5::Test Qt5::Core)
endif()

# Регистрируем тест для ctest
add_test(NAME ExpressionTests COMMAND expression_tests)
