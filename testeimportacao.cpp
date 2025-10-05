#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tinyobjloader/tiny_obj_loader.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

// ============= CONFIGURAÇÕES =============
// Modelos principais
const std::string IRONMAN_MODEL_PATH = "C:/Projetos/modeltest/external/models/IronMan/IronMan.obj";

// Nuvens
const std::string CLOUD_MODEL_PATHS[5] = {
    "C:/Projetos/modeltest/external/models/cloud/altostratus00.obj",
    "C:/Projetos/modeltest/external/models/cloud/altostratus01.obj",
    "C:/Projetos/modeltest/external/models/cloud/cumulus00.obj",
    "C:/Projetos/modeltest/external/models/cloud/cumulus01.obj",
    "C:/Projetos/modeltest/external/models/cloud/cumulus02.obj"
};

// Dragão
const std::string DRAGON_MODEL_PATH = "C:/Projetos/modeltest/external/models/dragon/dragon.obj";

// Árvores
const std::string TREE_MODEL_PATHS[2] = {
    "C:/Projetos/modeltest/external/models/white_oak/white_oak.obj",
    "C:/Projetos/modeltest/external/models/white_oak/white_oak.obj"
};

const float IRONMAN_SCALE = 0.005f; // Escala para o modelo Iron Man

const unsigned int SHADOW_WIDTH = 2500, SHADOW_HEIGHT = 2500;

const bool FULLSCREEN = false;
const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 900;

// AJUSTE DE ZOOM - Aumente esses valores para mais espaço
const float CAMERA_DISTANCE = 10.0f;
const float CAMERA_HEIGHT = 7.0f;
const float FOV_DEGREES = 60.0f;

const float WALL_LENGTH = 100.0f;
const float WALL_HEIGHT = 12.0f;
const float WALL_THICKNESS = 0.5f;
const float LANE_WIDTH = 18.0f;

// Tamanho do chão expandido
const int GROUND_SIZE = 120;
const float GROUND_QUAD_SIZE = 1.5f;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============= ESTADOS DO JOGO =============
enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

// ============= ESTRUTURAS =============
struct GameObject {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
    bool active;
    int type;
    int meshType;
};

struct Wall {
    glm::vec3 position;
    glm::vec3 scale;
};

// ============= VARIÁVEIS GLOBAIS =============
int currentWidth = WINDOW_WIDTH;
int currentHeight = WINDOW_HEIGHT;

GameState gameState = MENU;

glm::vec3 playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
float playerSpeed = 0.05f;
float gameSpeed = 0.08f;

int score = 0;
int highScore = 0;
float gameTime = 0.0f;

float cameraDistance = CAMERA_DISTANCE;
float cameraHeight = CAMERA_HEIGHT;
float cameraYaw = -90.0f;
float cameraPitch = -20.0f;
float cameraRotSpeed = 0.8f;

std::vector<GameObject> obstacles;
std::vector<GameObject> collectibles;
const int MAX_OBJECTS = 15;
float spawnTimer = 0.0f;
float spawnInterval = 1.5f;

std::vector<Wall> walls;

std::vector<float> playerVertices;
std::vector<float> cubeVertices;
std::vector<float> sphereVertices;

GLuint playerVAO, playerVBO;
GLuint cubeVAO, cubeVBO;
GLuint sphereVAO, sphereVBO;

int playerVertexCount = 0;
int cubeVertexCount = 0;
int sphereVertexCount = 0;

GLuint depthMapFBO, depthMap;
GLuint groundTexture;

glm::vec3 sunPos(5.0f, 40.0f, 10.0f); // Posição inicial do sol
float sunScale = 2.0f; // Tamanho do sol

std::vector<float> cloudVertices[5];
GLuint cloudVAO[5], cloudVBO[5];
int cloudVertexCount[5];

// Nuvens
glm::vec3 cloudPos[10] = {
    {-30.0f, 30.0f, -40.0f},
    {-15.0f, 30.0f, -20.0f},
    {0.0f,   30.0f,  0.0f},
    {15.0f,  30.0f,  20.0f},
    {30.0f,  30.0f,  40.0f},
    {-25.0f, 30.0f,  35.0f},
    {-10.0f, 30.0f,  15.0f},
    {10.0f,  30.0f, -15.0f},
    {25.0f,  30.0f, -35.0f},
    {5.0f,   30.0f,  25.0f}
};
float cloudScale[10] = {0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f};
std::vector<float> treeVertices[2];
GLuint treeVAO[2], treeVBO[2];
int treeVertexCount[2];
// Árvores
glm::vec3 treePos[10] = {
    {-20.0f, 0.0f, -30.0f},
    {-15.0f, 0.0f, -10.0f},
    {-10.0f, 0.0f,  10.0f},
    {-5.0f,  0.0f,  30.0f},
    {0.0f,   0.0f, -20.0f},
    {5.0f,   0.0f,  20.0f},
    {10.0f,  0.0f,  0.0f},
    {15.0f,  0.0f, -10.0f},
    {20.0f,  0.0f,  10.0f},
    {25.0f,  0.0f,  30.0f}
};
float treeScale[10] = {0.006f, 0.008f, 0.008f, 0.009f, 0.007f, 0.007f, 0.006f, 0.008f, 0.008f, 0.009f};

// Função para carregar textura de arquivo usando stb_image
GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Falha ao carregar textura: " << filename << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

// ============= CALLBACKS =============
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        static bool isFullscreen = FULLSCREEN;
        isFullscreen = !isFullscreen;

        if (isFullscreen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            currentWidth = mode->width;
            currentHeight = mode->height;
        }
        else {
            glfwSetWindowMonitor(window, nullptr, 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
            currentWidth = WINDOW_WIDTH;
            currentHeight = WINDOW_HEIGHT;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    currentWidth = width;
    currentHeight = height;
}

// ============= GERAÇÃO DE PRIMITIVAS =============
// Gera vértices de um cubo unitário
void generateCube(std::vector<float>& vertices) {
    float s = 1.0f;
    float cubeData[] = {
        -s,-s, s,  0, 0, 1,  0,0,  s,-s, s,  0, 0, 1,  1,0,  s, s, s,  0, 0, 1,  1,1,
        -s,-s, s,  0, 0, 1,  0,0,  s, s, s,  0, 0, 1,  1,1, -s, s, s,  0, 0, 1,  0,1,
         s,-s,-s,  0, 0,-1,  0,0, -s,-s,-s,  0, 0,-1,  1,0, -s, s,-s,  0, 0,-1,  1,1,
         s,-s,-s,  0, 0,-1,  0,0, -s, s,-s,  0, 0,-1,  1,1,  s, s,-s,  0, 0,-1,  0,1,
         s,-s, s,  1, 0, 0,  0,0,  s,-s,-s,  1, 0, 0,  1,0,  s, s,-s,  1, 0, 0,  1,1,
         s,-s, s,  1, 0, 0,  0,0,  s, s,-s,  1, 0, 0,  1,1,  s, s, s,  1, 0, 0,  0,1,
        -s,-s,-s, -1, 0, 0,  0,0, -s,-s, s, -1, 0, 0,  1,0, -s, s, s, -1, 0, 0,  1,1,
        -s,-s,-s, -1, 0, 0,  0,0, -s, s, s, -1, 0, 0,  1,1, -s, s,-s, -1, 0, 0,  0,1,
        -s, s, s,  0, 1, 0,  0,0,  s, s, s,  0, 1, 0,  1,0,  s, s,-s,  0, 1, 0,  1,1,
        -s, s, s,  0, 1, 0,  0,0,  s, s,-s,  0, 1, 0,  1,1, -s, s,-s,  0, 1, 0,  0,1,
        -s,-s,-s,  0,-1, 0,  0,0,  s,-s,-s,  0,-1, 0,  1,0,  s,-s, s,  0,-1, 0,  1,1,
        -s,-s,-s,  0,-1, 0,  0,0,  s,-s, s,  0,-1, 0,  1,1, -s,-s, s,  0,-1, 0,  0,1,
    };
    vertices.assign(cubeData, cubeData + sizeof(cubeData) / sizeof(float));
}

// Gera vértices de uma esfera unitária
void generateSphere(std::vector<float>& vertices, int segments = 20) {
    float radius = 1.0f;
    std::vector<float> tempVerts;

    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * (float)M_PI / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = static_cast<float>(lon * 2.0 * M_PI); // segments;
            float sinPhi = static_cast<float>(sin(phi));
            float cosPhi = static_cast<float>(cos(phi));

            float x = static_cast<float>(cosPhi * sinTheta);
            float y = static_cast<float>(cosTheta);
            float z = static_cast<float>(sinPhi * sinTheta);
            float u = (float)lon / segments;
            float v = (float)lat / segments;

            tempVerts.push_back(x * radius); tempVerts.push_back(y * radius); tempVerts.push_back(z * radius);
            tempVerts.push_back(x); tempVerts.push_back(y); tempVerts.push_back(z);
            tempVerts.push_back(u); tempVerts.push_back(v);
        }
    }

    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = (lat * (segments + 1)) + lon;
            int second = first + segments + 1;

            int idx1 = first * 8, idx2 = second * 8, idx3 = (first + 1) * 8, idx4 = (second + 1) * 8;

            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx1 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx2 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx3 + i]);

            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx3 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx2 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx4 + i]);
        }
    }
}

// Gera textura procedural para o chão
GLuint generateGrassTexture() {
    int width = 256, height = 256;
    std::vector<unsigned char> data(width * height * 3);

    glm::vec3 grass1(0.4f, 0.7f, 0.3f), grass2(0.35f, 0.65f, 0.25f), grass3(0.45f, 0.75f, 0.35f);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            int block = ((x / 16) + (y / 16)) % 3;
            glm::vec3 color = (block == 0) ? grass1 : (block == 1) ? grass2 : grass3;
            color += glm::vec3((rand() % 100) / 1000.0f);

            data[idx + 0] = static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255);
            data[idx + 1] = static_cast<unsigned char>(glm::clamp(color.g, 0.0f, 1.0f) * 255);
            data[idx + 2] = static_cast<unsigned char>(glm::clamp(color.b, 0.0f, 1.0f) * 255);
        }
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}

// Configura o framebuffer para shadow mapping
void setupShadowMapping() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Carrega modelo OBJ (sem materiais/texturas)
bool loadOBJ(const std::string& path, std::vector<float>& vertices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << "ERR: " << err << std::endl;
    if (!ret) return false;

    for (const auto& shape : shapes) {
        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            for (size_t v = 0; v < 3; v++) {
                auto idx = shape.mesh.indices[3 * f + v];
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                if (idx.normal_index >= 0) {
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                else {
                    vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
                }

                if (idx.texcoord_index >= 0) {
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                }
                else {
                    vertices.push_back(0.0f); vertices.push_back(0.0f);
                }
            }
        }
    }
    return true;
}

// Gera o chão quadriculado
void generateGround(std::vector<float>& vertices, int size = GROUND_SIZE, float quadSize = GROUND_QUAD_SIZE) {
    for (int x = -size; x < size; ++x) {
        for (int z = -size; z < size; ++z) {
            float x0 = (float)x * quadSize, x1 = (x + 1) * quadSize;
            float z0 = (float)z * quadSize, z1 = (z + 1) * quadSize;
            float u0 = (float)x + size, u1 = (float)x + size + 1;
            float v0 = (float)z + size, v1 = (float)z + size + 1;

            vertices.insert(vertices.end(), {
                x0, 0.0f, z0,  0.0f, 1.0f, 0.0f,  u0, v0,
                x1, 0.0f, z0,  0.0f, 1.0f, 0.0f,  u1, v0,
                x1, 0.0f, z1,  0.0f, 1.0f, 0.0f,  u1, v1,
                x0, 0.0f, z0,  0.0f, 1.0f, 0.0f,  u0, v0,
                x1, 0.0f, z1,  0.0f, 1.0f, 0.0f,  u1, v1,
                x0, 0.0f, z1,  0.0f, 1.0f, 0.0f,  u0, v1
                });
        }
    }
}

// ============= LÓGICA DO JOGO =============
// Spawna obstáculos ou colecionáveis
void spawnObject(int type) {
    GameObject obj;
    obj.position = glm::vec3((rand() % 7 - 3) * 2.0f, 0.7f, -35.0f);
    obj.active = true;
    obj.type = type;

    if (type == 0) {
        obj.scale = glm::vec3(0.8f, 1.5f, 0.8f);
        obj.color = glm::vec3(0.85f, 0.2f, 0.15f);
        obj.meshType = 1;
        obstacles.push_back(obj);
    }
    else {
        obj.scale = glm::vec3(0.5f);
        obj.color = glm::vec3(1.0f, 0.84f, 0.0f);
        obj.meshType = 2;
        collectibles.push_back(obj);
    }
}

// Atualiza estado do jogo
void updateGame(float deltaTime, GLFWwindow* window) {
    if (gameState != PLAYING) return;

    gameTime += deltaTime;
    spawnTimer += deltaTime;

    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        if (obstacles.size() < MAX_OBJECTS / 2) spawnObject(0);
        if (collectibles.size() < MAX_OBJECTS / 2) spawnObject(1);
    }

    for (auto& obs : obstacles) {
        if (obs.active) {
            obs.position.z += gameSpeed;
            float dist = glm::length(obs.position - playerPos);
            if (dist < 1.2f) {
                gameState = GAME_OVER;
                if (score > highScore) highScore = score;
            }
            if (obs.position.z > 8.0f) obs.active = false;
        }
    }

    for (auto& col : collectibles) {
        if (col.active) {
            col.position.z += gameSpeed;
            col.position.y = 0.7f + sin(gameTime * 3.0f + col.position.x) * 0.2f;

            float dist = glm::length(col.position - playerPos);
            if (dist < 1.2f) {
                col.active = false;
                score += 10;
            }
            if (col.position.z > 8.0f) col.active = false;
        }
    }

    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [](const GameObject& o) { return !o.active; }), obstacles.end());
    collectibles.erase(std::remove_if(collectibles.begin(), collectibles.end(),
        [](const GameObject& o) { return !o.active; }), collectibles.end());
}

// Processa entrada do usuário
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (gameState == MENU) {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            gameState = PLAYING;
            score = 0;
            gameTime = 0.0f;
            playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
            obstacles.clear();
            collectibles.clear();
        }
        return;
    }

    if (gameState == GAME_OVER) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            gameState = PLAYING;
            score = 0;
            gameTime = 0.0f;
            playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
            obstacles.clear();
            collectibles.clear();
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            gameState = MENU;
            score = 0;
            gameTime = 0.0f;
            playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
            obstacles.clear();
            collectibles.clear();
        }
        return;
    }

    glm::vec3 moveDir(0.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir.z -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir.z += 1.0f;

    if (glm::length(moveDir) > 0.0f) {
        playerPos += glm::normalize(moveDir) * playerSpeed;
        playerPos.x = glm::clamp(playerPos.x, -8.0f, 8.0f);
        playerPos.z = glm::clamp(playerPos.z, -3.0f, 5.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) cameraYaw -= cameraRotSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraYaw += cameraRotSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) cameraPitch += cameraRotSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) cameraPitch -= cameraRotSpeed;

    cameraPitch = glm::clamp(cameraPitch, -89.0f, 89.0f);
}

// Checa compilação de shader
void checkShaderCompile(GLuint shader, const char* type) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << type << " error:\n" << infoLog << std::endl;
    }
}

// Checa linkagem de programa
void checkProgramLink(GLuint program) {
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program link error:\n" << infoLog << std::endl;
    }
}

// ============= MAIN =============
int main() {
    srand(static_cast<unsigned>(time(0)));

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;

    if (FULLSCREEN) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        window = glfwCreateWindow(mode->width, mode->height, "Corrida 3D - CG UFCA", monitor, nullptr);
        currentWidth = mode->width;
        currentHeight = mode->height;
    }
    else {
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Corrida 3D - CG UFCA", nullptr, nullptr);
    }

    if (!window) {
        std::cerr << "Falha ao criar janela\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Remover limite de FPS (desativa V-Sync)
    //glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD\n";
        return -1;
    }

    // Inicializar ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);

    // Carregamento dos modelos principais
    if (!loadOBJ(IRONMAN_MODEL_PATH, playerVertices)) {
        std::cerr << "Falha ao carregar modelo - usando cubo\n";
        generateCube(playerVertices);
    }
    playerVertexCount = playerVertices.size() / 8;

    generateCube(cubeVertices);
    cubeVertexCount = cubeVertices.size() / 8;

    generateSphere(sphereVertices, 15);
    sphereVertexCount = sphereVertices.size() / 8;

    // Função para configurar VAO/VBO
    auto setupVAO = [](GLuint& vao, GLuint& vbo, const std::vector<float>& data) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    };

    setupVAO(playerVAO, playerVBO, playerVertices);
    setupVAO(cubeVAO, cubeVBO, cubeVertices);
    setupVAO(sphereVAO, sphereVBO, sphereVertices);

    std::vector<float> groundVertices;
    generateGround(groundVertices);
    GLuint groundVAO, groundVBO;
    setupVAO(groundVAO, groundVBO, groundVertices);

    float halfWidth = LANE_WIDTH / 2.0f;
    float wallStartZ = -WALL_LENGTH / 2.0f;
    float wallCenterZ = 0.0f;

    walls = {
        {glm::vec3(-halfWidth, WALL_HEIGHT / 2.0f, wallCenterZ), glm::vec3(WALL_THICKNESS, WALL_HEIGHT, WALL_LENGTH)},
        {glm::vec3(halfWidth, WALL_HEIGHT / 2.0f, wallCenterZ), glm::vec3(WALL_THICKNESS, WALL_HEIGHT, WALL_LENGTH)},
        {glm::vec3(0.0f, WALL_HEIGHT / 2.0f, wallStartZ), glm::vec3(LANE_WIDTH + WALL_THICKNESS * 2, WALL_HEIGHT, WALL_THICKNESS)}
    };

    //groundTexture = generateGrassTexture();
    groundTexture = loadTexture("C:/Projetos/modeltest/external/textures/concretewall.jpg");
    setupShadowMapping();

    // Shaders
    const char* depthVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
void main() { gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0); }
)";
    const char* depthFS = R"(
#version 330 core
void main() {}
)";

    GLuint depthVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(depthVertShader, 1, &depthVS, nullptr);
    glCompileShader(depthVertShader);
    checkShaderCompile(depthVertShader, "Depth VS");

    GLuint depthFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(depthFragShader, 1, &depthFS, nullptr);
    glCompileShader(depthFragShader);

    GLuint depthProgram = glCreateProgram();
    glAttachShader(depthProgram, depthVertShader);
    glAttachShader(depthProgram, depthFragShader);
    glLinkProgram(depthProgram);
    checkProgramLink(depthProgram);
    glDeleteShader(depthVertShader);
    glDeleteShader(depthFragShader);

    const char* mainVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

    const char* mainFS = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform int useTexture;
uniform float brightness;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec3 color = (useTexture == 1) ? texture(texture1, TexCoord).rgb : objectColor;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * color;
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;
    
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    lighting *= brightness;
    
    FragColor = vec4(lighting, 1.0);
}
)";

    GLuint mainVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(mainVertShader, 1, &mainVS, nullptr);
    glCompileShader(mainVertShader);
    checkShaderCompile(mainVertShader, "Main VS");

    GLuint mainFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mainFragShader, 1, &mainFS, nullptr);
    glCompileShader(mainFragShader);
    checkShaderCompile(mainFragShader, "Main FS");

    GLuint mainProgram = glCreateProgram();
    glAttachShader(mainProgram, mainVertShader);
    glAttachShader(mainProgram, mainFragShader);
    glLinkProgram(mainProgram);
    checkProgramLink(mainProgram);
    glDeleteShader(mainVertShader);
    glDeleteShader(mainFragShader);

    float lastTime = glfwGetTime();

    std::cout << "\n╔═════════════════════════════════════════╗" << std::endl;
    std::cout << "║       CORRIDA 3D - BEM-VINDO!           ║" << std::endl;
    std::cout << "╚═════════════════════════════════════════╝\n" << std::endl;

    // Carregamento dos modelos de nuvens
    for (int i = 0; i < 5; ++i) {
        if (!loadOBJ(CLOUD_MODEL_PATHS[i], cloudVertices[i])) {
            std::cerr << "Falha ao carregar nuvem " << i << std::endl;
            cloudVertices[i].clear();
        }
        cloudVertexCount[i] = cloudVertices[i].size() / 8;
        setupVAO(cloudVAO[i], cloudVBO[i], cloudVertices[i]);
    }
  
        // Carregamento das árvores
    for (int i = 0; i < 2; ++i) {
        if (!loadOBJ(TREE_MODEL_PATHS[i], treeVertices[i])) {
            std::cerr << "Falha ao carregar árvore " << i << std::endl;
            treeVertices[i].clear();
        }
        treeVertexCount[i] = treeVertices[i].size() / 8;
        setupVAO(treeVAO[i], treeVBO[i], treeVertices[i]);
    }

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window);
        updateGame(deltaTime, window);

        glm::vec3 lightPos = sunPos;

        float near_plane = 1.0f, far_plane = 60.0f;
        glm::mat4 lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        glUseProgram(depthProgram);
        glUniformMatrix4fv(glGetUniformLocation(depthProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        auto renderDepth = [&](const glm::mat4& model, GLuint vao, int count) {
            glUniformMatrix4fv(glGetUniformLocation(depthProgram, "model"), 1, GL_FALSE, &model[0][0]);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, count);
        };

        if (gameState == PLAYING) {
            glm::mat4 groundModel = glm::mat4(1.0f);
            renderDepth(groundModel, groundVAO, groundVertices.size() / 8);

            glm::mat4 playerModel = glm::translate(glm::mat4(1.0f), playerPos);
            playerModel = glm::scale(playerModel, glm::vec3(IRONMAN_SCALE));
            renderDepth(playerModel, playerVAO, playerVertexCount);

            for (const auto& obs : obstacles) {
                if (obs.active) {
                    glm::mat4 m = glm::translate(glm::mat4(1.0f), obs.position);
                    m = glm::scale(m, obs.scale * glm::vec3(0.15f));
                    renderDepth(m, cubeVAO, cubeVertexCount);
                }
            }

            for (const auto& col : collectibles) {
                if (col.active) {
                    glm::mat4 m = glm::translate(glm::mat4(1.0f), col.position);
                    m = glm::scale(m, col.scale * glm::vec3(0.12f));
                    renderDepth(m, sphereVAO, sphereVertexCount);
                }
            }

            for (const auto& wall : walls) {
                glm::mat4 m = glm::translate(glm::mat4(1.0f), wall.position);
                m = glm::scale(m, wall.scale * glm::vec3(0.15f));
                renderDepth(m, cubeVAO, cubeVertexCount);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, currentWidth, currentHeight);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 cameraPos;
        glm::mat4 view;

        if (gameState == PLAYING) {
            float yawRad = glm::radians(cameraYaw);
            float pitchRad = glm::radians(cameraPitch);

            glm::vec3 cameraDir(cos(yawRad) * cos(pitchRad), sin(pitchRad), sin(yawRad) * cos(pitchRad));
            cameraDir = glm::normalize(cameraDir);

            float dynamicCameraDistance = cameraDistance;
            float minHeight = 2.0f;
            float minDistance = 3.0f;
            float maxDistance = cameraDistance;

            cameraPos = playerPos - cameraDir * dynamicCameraDistance + glm::vec3(0.0f, cameraHeight, 0.0f);

            if (cameraPos.y < minHeight) {
                dynamicCameraDistance = minDistance;
                cameraPos = playerPos - cameraDir * dynamicCameraDistance + glm::vec3(0.0f, cameraHeight, 0.0f);
                if (cameraPos.y < 0.5f) cameraPos.y = 0.5f;
                // Olha para cima (alvo acima do personagem)
                view = glm::lookAt(cameraPos, playerPos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            } else {
                // Comportamento padrão
                view = glm::lookAt(cameraPos, playerPos, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        } else {
            cameraPos = glm::vec3(0.0f, 5.0f, 10.0f);
            view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        float aspectRatio = (float)currentWidth / (float)currentHeight;
        if (aspectRatio <= 0.0f) aspectRatio = 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(FOV_DEGREES), aspectRatio, 0.1f, 100.0f);

        glUseProgram(mainProgram);
        glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(mainProgram, "viewPos"), 1, &cameraPos[0]);
        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(glGetUniformLocation(mainProgram, "shadowMap"), 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        glUniform1i(glGetUniformLocation(mainProgram, "texture1"), 0);
        glUniform1i(glGetUniformLocation(mainProgram, "useTexture"), 1);
        glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.0f);

        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &groundModel[0][0]);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, groundVertices.size() / 8);

        glUniform1i(glGetUniformLocation(mainProgram, "useTexture"), 0);

        // Renderizar nuvens
        for (int i = 0; i < 10; ++i) {
            if (cloudVertexCount[i % 5] > 0) {
                glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.95f, 0.95f, 1.0f);
                glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 2.0f);
                glm::mat4 model = glm::translate(glm::mat4(1.0f), cloudPos[i]);
                model = glm::scale(model, glm::vec3(cloudScale[i]));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &model[0][0]);
                glBindVertexArray(cloudVAO[i % 5]);
                glDrawArrays(GL_TRIANGLES, 0, cloudVertexCount[i % 5]);
            }
        }

        // Renderizar árvores
        for (int i = 0; i < 10; ++i) {
            if (treeVertexCount[i % 2] > 0) {
                glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.6f, 0.5f, 0.3f);
                glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.2f);
                glm::mat4 model = glm::translate(glm::mat4(1.0f), treePos[i]);
                model = glm::scale(model, glm::vec3(treeScale[i]));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &model[0][0]);
                glBindVertexArray(treeVAO[i % 2]);
                glDrawArrays(GL_TRIANGLES, 0, treeVertexCount[i % 2]);
            }
        }

        if (gameState == PLAYING) {
            glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.3f, 0.5f, 0.9f);
            glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.0f);
            glm::mat4 playerModel = glm::translate(glm::mat4(1.0f), playerPos);
            playerModel = glm::scale(playerModel, glm::vec3(IRONMAN_SCALE));
            glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &playerModel[0][0]);
            glBindVertexArray(playerVAO);
            glDrawArrays(GL_TRIANGLES, 0, playerVertexCount);

            for (const auto& obs : obstacles) {
                if (obs.active) {
                    glUniform3fv(glGetUniformLocation(mainProgram, "objectColor"), 1, &obs.color[0]);
                    glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.0f);

                    glm::mat4 m = glm::translate(glm::mat4(1.0f), obs.position);
                    m = glm::scale(m, obs.scale * glm::vec3(0.15f));
                    glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &m[0][0]);
                    glBindVertexArray(cubeVAO);
                    glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
                }
            }

            for (const auto& col : collectibles) {
                if (col.active) {
                    glUniform3fv(glGetUniformLocation(mainProgram, "objectColor"), 1, &col.color[0]);
                    glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.8f);

                    glm::mat4 m = glm::translate(glm::mat4(1.0f), col.position);
                    m = glm::rotate(m, currentTime * 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                    m = glm::scale(m, col.scale * glm::vec3(0.12f));
                    glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &m[0][0]);
                    glBindVertexArray(sphereVAO);
                    glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
                }
            }

            glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.3f, 0.25f, 0.2f);
            glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 0.8f);

            for (const auto& wall : walls) {
                glm::mat4 m = glm::translate(glm::mat4(1.0f), wall.position);
                m = glm::scale(m, wall.scale * glm::vec3(0.15f));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &m[0][0]);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            }
        }

        // Renderizar o sol (apenas esfera amarela, sem bloom)
        glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 1.0f, 1.0f, 0.2f);
        glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 2.0f);
        glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos);
        sunModel = glm::scale(sunModel, glm::vec3(sunScale));
        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &sunModel[0][0]);
        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);

        // ImGui UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (gameState == MENU) {
            ImGui::SetNextWindowPos(ImVec2(currentWidth / 2.0f - 300, currentHeight / 2.0f - 250));
            ImGui::SetNextWindowSize(ImVec2(600, 500));
            ImGui::Begin("Menu Principal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::SetWindowFontScale(2.5f);
            ImGui::Text("CORRIDA 3D");
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::Text("OBJETIVO:");
            ImGui::BulletText("Desvie dos CACTOS VERMELHOS!");
            ImGui::BulletText("Colete MOEDAS DOURADAS (+10 pontos)");

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::Text("CONTROLES:");
            ImGui::BulletText("WASD - Mover jogador");
            ImGui::BulletText("Setas - Rotacionar camera");
            ImGui::BulletText("F11 - Tela cheia");

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetWindowFontScale(1.5f);
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Pressione ESPACO para iniciar!");
            ImGui::SetWindowFontScale(1.0f);

            ImGui::End();
        }
        else if (gameState == PLAYING) {
            // HUD no canto superior direito
            ImGui::SetNextWindowPos(ImVec2(currentWidth - 250, 10));
            ImGui::SetNextWindowSize(ImVec2(240, 100));
            ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

            ImGui::SetWindowFontScale(1.5f);
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "SCORE: %d", score);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "RECORDE: %d", highScore);
            ImGui::SetWindowFontScale(1.0f);

            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::SetNextWindowSize(ImVec2(300, 140));
            ImGui::Begin("Sol", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::Text("Ajuste a posição do sol:");
            ImGui::SliderFloat("X", &sunPos.x, -50.0f, 50.0f);
            ImGui::SliderFloat("Y", &sunPos.y, 1.0f, 50.0f);
            ImGui::SliderFloat("Z", &sunPos.z, -50.0f, 50.0f);
            ImGui::SliderFloat("Tamanho", &sunScale, 0.5f, 5.0f);
            ImGui::End();
        }
        else if (gameState == GAME_OVER) {
            ImGui::SetNextWindowPos(ImVec2(currentWidth / 2.0f - 250, currentHeight / 2.0f - 200));
            ImGui::SetNextWindowSize(ImVec2(500, 400));
            ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::SetWindowFontScale(2.5f);
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "GAME OVER!");
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetWindowFontScale(1.8f);
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "Pontuacao: %d", score);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Recorde: %d", highScore);
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::SetWindowFontScale(1.3f);
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "R - Jogar novamente");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "M - Voltar ao menu");
            ImGui::SetWindowFontScale(1.0f);

            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(10, 160));
ImGui::SetNextWindowSize(ImVec2(350, 400));
ImGui::Begin("Teste de Tamanho/Posição", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &playerVAO);
    glDeleteBuffers(1, &playerVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteProgram(mainProgram);
    glDeleteProgram(depthProgram);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
    glfwTerminate();

    return 0;
}