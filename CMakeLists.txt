cmake_minimum_required(VERSION 3.10)
project(testeimportacao)

set(CMAKE_CXX_STANDARD 17)

# Diretórios de includes
include_directories(
    ${PROJECT_SOURCE_DIR}/glad/include
    C:/glfw34/install/include
    C:/Projetos/glm
)

# Diretórios de bibliotecas
link_directories(
    C:/glfw34/install/lib
)

include_directories(
    ${PROJECT_SOURCE_DIR}/external/tinyobjloader
)


# Executável com os sources
add_executable(testeimportacao
    ${PROJECT_SOURCE_DIR}/testeimportacao.cpp
    ${PROJECT_SOURCE_DIR}/glad.c
)

# Linkar GLFW + OpenGL + dependências do Windows
target_link_libraries(testeimportacao
    glfw3
    opengl32
    gdi32
    shell32
    user32
)
