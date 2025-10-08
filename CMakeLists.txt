cmake_minimum_required(VERSION 3.10)
project(testeimportacao)
set(CMAKE_CXX_STANDARD 17)

# Diretórios de includes
include_directories(
    ${PROJECT_SOURCE_DIR}/glad/include
    C:/glfw34/install/include
    C:/glfw34/glfw34/install/include
    C:/Projetos/glm
    C:/Projetos/glm/glm
)

# Diretórios de bibliotecas
link_directories(
    C:/glfw34/install/lib
    C:/glfw34/glfw34/install/lib
)

# TinyObjLoader
include_directories(
    ${PROJECT_SOURCE_DIR}/external/tinyobjloader
)

# ImGui
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/external/imgui)
include_directories(${IMGUI_DIR})

# Executável com os sources
add_executable(testeimportacao
    ${PROJECT_SOURCE_DIR}/testeimportacao.cpp
    ${PROJECT_SOURCE_DIR}/glad.c
    # ImGui sources
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
)

# Linkar GLFW + OpenGL + dependências do Windows
target_link_libraries(testeimportacao
    glfw3
    opengl32
    gdi32
    shell32
    user32
)
