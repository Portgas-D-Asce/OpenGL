cmake_minimum_required(VERSION 3.31)
project(hello_opengl)

set(CMAKE_CXX_STANDARD 17)
if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit")
    #add_compile_definitions(GL_SILENCE_DEPRECATION)
endif ()

# 包含glad头文件
include_directories(3rd/glad/include)
include_directories(3rd/glfw/include)
include_directories(3rd/stb_image)
include_directories(3rd/glm)
include_directories(3rd/glfw/include)
link_directories(3rd/glfw/lib-arm64)

add_executable(main 3rd/glad/src/glad.c main.cpp)

# 目标链接库
target_link_libraries(
        main # demo_cpp是项目名称
        glfw3
)