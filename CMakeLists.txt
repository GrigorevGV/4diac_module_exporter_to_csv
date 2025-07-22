cmake_minimum_required(VERSION 3.10)

# Название проекта
project(Parser)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исполняемый файл
add_executable(parser parser.cpp)

# Если нужны дополнительные библиотеки, раскомментируйте:
# find_package(SomeLibrary REQUIRED)
# target_link_libraries(parser SomeLibrary)

# Настройки компилятора
if(MSVC)
    target_compile_options(parser PRIVATE /W4)
else()
    target_compile_options(parser PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Устанавливаем имя выходного файла
set_target_properties(parser PROPERTIES OUTPUT_NAME "parser") 