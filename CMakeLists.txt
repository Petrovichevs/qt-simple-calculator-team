cmake_minimum_required(VERSION 3.16)

project(simpleCalculator LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)

add_executable(simpleCalculator WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    expression.cpp
    expression.h
    mainwindow.ui
)
target_link_libraries(simpleCalculator PRIVATE Qt5::Widgets)
target_compile_definitions(simpleCalculator PRIVATE QT_DEPRECATED_WARNINGS)
