
cmake_minimum_required (VERSION 3.8)

project ("netology_homework_cpp_sql1")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (netology_homework_cpp_sql1 "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET netology_homework_cpp_sql1 PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(C:/Users/dpoli/source/repos/libpqxx libpqxx-build)

target_compile_features(netology_homework_cpp_sql1 PRIVATE cxx_std_17)

target_link_libraries(netology_homework_cpp_sql1 pqxx)