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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============= CONFIGURAÇÕES =============
const std::string BASE_PATH = "C:/Projetos/modeltest/external/";
const std::string IRONMAN_MODEL = BASE_PATH + "models/IronMan/IronMan.obj";
const std::string CLOUD_MODELS[5] = {
    BASE_PATH + "models/cloud/altostratus00.obj",
    BASE_PATH + "models/cloud/altostratus01.obj",
    BASE_PATH + "models/cloud/cumulus00.obj",
    BASE_PATH + "models/cloud/cumulus01.obj",
    BASE_PATH + "models/cloud/cumulus02.obj"
};
const std::string TREE_MODEL = BASE_PATH + "models/white_oak/white_oak.obj";
const std::string GROUND_TEXTURE = BASE_PATH + "textures/concretewall.jpg";

const float IRONMAN_SCALE = 0.005f;
const unsigned int SHADOW_SIZE = 2500;
const bool FULLSCREEN = false;
const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 900;
const float CAMERA_DISTANCE = 10.0f;
const float CAMERA_HEIGHT = 7.0f;
const float FOV_DEGREES = 60.0f;
const int GROUND_SIZE = 120;
const float GROUND_QUAD_SIZE = 1.5f;

enum GameState { MENU, PLAYING, GAME_OVER };

struct GameObject {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
    bool active;
    int type;
};

// ============= VARIÁVEIS GLOBAIS =============
int currentWidth = WINDOW_WIDTH;
int currentHeight = WINDOW_HEIGHT;
GameState gameState = MENU;

glm::vec3 playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
glm::vec3 playerVelocity = glm::vec3(0.0f);
float playerSpeed = 0.05f;
float playerRotation = 0.0f;
float playerTilt = 0.0f;
float runAnimationTime = 0.0f;
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
const int MAX_OBJECTS = 25;
float spawnTimer = 0.0f;
float spawnInterval = 1.2f;

GLuint playerVAO, playerVBO, cubeVAO, cubeVBO, sphereVAO, sphereVBO, groundVAO, groundVBO;
GLuint cloudVAO[5], cloudVBO[5], treeVAO, treeVBO;
int playerVertexCount = 0, cubeVertexCount = 0, sphereVertexCount = 0;
int cloudVertexCount[5] = { 0 }, treeVertexCount = 0;

GLuint depthMapFBO, depthMap, groundTexture;
glm::vec3 sunPos(5.0f, 40.0f, 10.0f);
float sunScale = 2.0f;

const int NUM_CLOUDS = 10;
glm::vec3 cloudPos[NUM_CLOUDS] = {
    {-30.0f, 30.0f, -40.0f}, {-15.0f, 30.0f, -20.0f}, {0.0f, 30.0f, 0.0f},
    {15.0f, 30.0f, 20.0f}, {30.0f, 30.0f, 40.0f}, {-25.0f, 30.0f, 35.0f},
    {-10.0f, 30.0f, 15.0f}, {10.0f, 30.0f, -15.0f}, {25.0f, 30.0f, -35.0f},
    {5.0f, 30.0f, 25.0f}
};
float cloudScale[NUM_CLOUDS] = { 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f, 0.12f };

const int NUM_TREES = 20;
glm::vec3 treePos[NUM_TREES] = {
    {-12.0f, 0.0f, -40.0f}, {-12.0f, 0.0f, -30.0f}, {-12.0f, 0.0f, -20.0f},
    {-12.0f, 0.0f, -10.0f}, {-12.0f, 0.0f, 0.0f}, {-12.0f, 0.0f, 10.0f},
    {-12.0f, 0.0f, 20.0f}, {-12.0f, 0.0f, 30.0f}, {-12.0f, 0.0f, 40.0f},
    {-12.0f, 0.0f, 50.0f}, {12.0f, 0.0f, -40.0f}, {12.0f, 0.0f, -30.0f},
    {12.0f, 0.0f, -20.0f}, {12.0f, 0.0f, -10.0f}, {12.0f, 0.0f, 0.0f},
    {12.0f, 0.0f, 10.0f}, {12.0f, 0.0f, 20.0f}, {12.0f, 0.0f, 30.0f},
    {12.0f, 0.0f, 40.0f}, {12.0f, 0.0f, 50.0f}
};
float treeScale[NUM_TREES] = {
    0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f,
    0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f, 0.008f
};

// ============= FUNÇÕES AUXILIARES =============
GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Falha ao carregar textura: " << filename << std::endl;
        return 0;
    }

    GLenum format = (channels == 1) ? GL_RED : (channels == 3) ? GL_RGB : GL_RGBA;

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

void checkShaderCompile(GLuint shader, const char* type) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << type << " shader error: " << infoLog << std::endl;
    }
}

void checkProgramLink(GLuint program) {
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program link error: " << infoLog << std::endl;
    }
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

// ============= GERAÇÃO DE GEOMETRIA =============
void generateCube(std::vector<float>& vertices) {
    float s = 1.0f;
    float cubeData[] = {
        -s,-s, s, 0,0,1, 0,0,  s,-s, s, 0,0,1, 1,0,  s, s, s, 0,0,1, 1,1,
        -s,-s, s, 0,0,1, 0,0,  s, s, s, 0,0,1, 1,1, -s, s, s, 0,0,1, 0,1,
         s,-s,-s, 0,0,-1, 0,0, -s,-s,-s, 0,0,-1, 1,0, -s, s,-s, 0,0,-1, 1,1,
         s,-s,-s, 0,0,-1, 0,0, -s, s,-s, 0,0,-1, 1,1,  s, s,-s, 0,0,-1, 0,1,
         s,-s, s, 1,0,0, 0,0,  s,-s,-s, 1,0,0, 1,0,  s, s,-s, 1,0,0, 1,1,
         s,-s, s, 1,0,0, 0,0,  s, s,-s, 1,0,0, 1,1,  s, s, s, 1,0,0, 0,1,
        -s,-s,-s, -1,0,0, 0,0, -s,-s, s, -1,0,0, 1,0, -s, s, s, -1,0,0, 1,1,
        -s,-s,-s, -1,0,0, 0,0, -s, s, s, -1,0,0, 1,1, -s, s,-s, -1,0,0, 0,1,
        -s, s, s, 0,1,0, 0,0,  s, s, s, 0,1,0, 1,0,  s, s,-s, 0,1,0, 1,1,
        -s, s, s, 0,1,0, 0,0,  s, s,-s, 0,1,0, 1,1, -s, s,-s, 0,1,0, 0,1,
        -s,-s,-s, 0,-1,0, 0,0,  s,-s,-s, 0,-1,0, 1,0,  s,-s, s, 0,-1,0, 1,1,
        -s,-s,-s, 0,-1,0, 0,0,  s,-s, s, 0,-1,0, 1,1, -s,-s, s, 0,-1,0, 0,1
    };
    vertices.assign(cubeData, cubeData + sizeof(cubeData) / sizeof(float));
}

void generateSphere(std::vector<float>& vertices, int segments = 20) {
    float radius = 1.0f;
    std::vector<float> temp;

    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * (float)M_PI / segments;
        float sinTheta = sin(theta), cosTheta = cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * (float)M_PI / segments;
            float sinPhi = sin(phi), cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            float u = (float)lon / segments;
            float v = (float)lat / segments;

            temp.insert(temp.end(), { x * radius, y * radius, z * radius, x, y, z, u, v });
        }
    }

    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = (lat * (segments + 1) + lon) * 8;
            int second = first + (segments + 1) * 8;

            for (int i = 0; i < 8; i++) vertices.push_back(temp[first + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(temp[second + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(temp[first + 8 + i]);

            for (int i = 0; i < 8; i++) vertices.push_back(temp[first + 8 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(temp[second + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(temp[second + 8 + i]);
        }
    }
}

void generateGround(std::vector<float>& vertices, int size = GROUND_SIZE, float quadSize = GROUND_QUAD_SIZE) {
    for (int x = -size; x < size; ++x) {
        for (int z = -size; z < size; ++z) {
            float x0 = x * quadSize, x1 = (x + 1) * quadSize;
            float z0 = z * quadSize, z1 = (z + 1) * quadSize;
            float u0 = x + size, u1 = x + size + 1.0f;
            float v0 = z + size, v1 = z + size + 1.0f;

            vertices.insert(vertices.end(), {
                x0, 0, z0, 0,1,0, u0, v0,  x1, 0, z0, 0,1,0, u1, v0,  x1, 0, z1, 0,1,0, u1, v1,
                x0, 0, z0, 0,1,0, u0, v0,  x1, 0, z1, 0,1,0, u1, v1,  x0, 0, z1, 0,1,0, u0, v1
                });
        }
    }
}

void setupShadowMapping() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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

bool loadOBJ(const std::string& path, std::vector<float>& vertices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        if (!err.empty()) std::cerr << "Erro ao carregar " << path << ": " << err << std::endl;
        return false;
    }

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
                    vertices.insert(vertices.end(), { 0.0f, 1.0f, 0.0f });
                }

                if (idx.texcoord_index >= 0) {
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                }
                else {
                    vertices.insert(vertices.end(), { 0.0f, 0.0f });
                }
            }
        }
    }
    return true;
}

void setupVAO(GLuint& vao, GLuint& vbo, const std::vector<float>& data) {
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
}

// ============= LÓGICA DO JOGO =============
void spawnObject(int type) {
    if (type == 0) {
        int numObstacles = 1 + (rand() % 3);
        for (int i = 0; i < numObstacles; ++i) {
            GameObject obj;
            float baseX = (rand() % 5 - 2) * 2.5f;
            obj.position = glm::vec3(baseX + (i - numObstacles / 2.0f) * 2.0f, 0.7f, -35.0f);
            obj.active = true;
            obj.type = 0;
            obj.scale = glm::vec3(0.8f, 1.5f, 0.8f);
            obj.color = glm::vec3(0.85f, 0.2f, 0.15f);
            obstacles.push_back(obj);
        }
    }
    else {
        GameObject obj;
        obj.position = glm::vec3((rand() % 7 - 3) * 2.0f, 0.7f, -35.0f);
        obj.active = true;
        obj.type = 1;
        obj.scale = glm::vec3(0.8f);
        obj.color = glm::vec3(1.0f, 0.84f, 0.0f);
        collectibles.push_back(obj);
    }
}

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
            if (glm::length(obs.position - playerPos) < 1.2f) {
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

            if (glm::length(col.position - playerPos) < 1.2f) {
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
        window = glfwCreateWindow(mode->width, mode->height, "Corrida 3D", monitor, nullptr);
        currentWidth = mode->width;
        currentHeight = mode->height;
    }
    else {
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Corrida 3D", nullptr, nullptr);
    }

    if (!window) {
        std::cerr << "Falha ao criar janela\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD\n";
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);

    std::vector<float> playerVertices, cubeVertices, sphereVertices, groundVertices;
    std::vector<float> cloudVertices[5], treeVertices;

    if (!loadOBJ(IRONMAN_MODEL, playerVertices)) {
        std::cerr << "Modelo Iron Man não encontrado, usando cubo\n";
        generateCube(playerVertices);
    }
    playerVertexCount = playerVertices.size() / 8;

    generateCube(cubeVertices);
    cubeVertexCount = cubeVertices.size() / 8;

    generateSphere(sphereVertices, 15);
    sphereVertexCount = sphereVertices.size() / 8;

    generateGround(groundVertices);

    setupVAO(playerVAO, playerVBO, playerVertices);
    setupVAO(cubeVAO, cubeVBO, cubeVertices);
    setupVAO(sphereVAO, sphereVBO, sphereVertices);
    setupVAO(groundVAO, groundVBO, groundVertices);

    for (int i = 0; i < 5; ++i) {
        if (loadOBJ(CLOUD_MODELS[i], cloudVertices[i])) {
            cloudVertexCount[i] = cloudVertices[i].size() / 8;
            setupVAO(cloudVAO[i], cloudVBO[i], cloudVertices[i]);
        }
    }

    if (loadOBJ(TREE_MODEL, treeVertices)) {
        treeVertexCount = treeVertices.size() / 8;
        setupVAO(treeVAO, treeVBO, treeVertices);
    }

    groundTexture = loadTexture(GROUND_TEXTURE.c_str());
    setupShadowMapping();

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

    GLuint depthVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(depthVert, 1, &depthVS, nullptr);
    glCompileShader(depthVert);
    checkShaderCompile(depthVert, "Depth VS");

    GLuint depthFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(depthFrag, 1, &depthFS, nullptr);
    glCompileShader(depthFrag);

    GLuint depthProgram = glCreateProgram();
    glAttachShader(depthProgram, depthVert);
    glAttachShader(depthProgram, depthFrag);
    glLinkProgram(depthProgram);
    checkProgramLink(depthProgram);
    glDeleteShader(depthVert);
    glDeleteShader(depthFrag);

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
    
    vec3 ambient = 0.4 * color;
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * color;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 specular = 0.6 * pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0);
    
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    
    FragColor = vec4(lighting * brightness, 1.0);
}
)";

    GLuint mainVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(mainVert, 1, &mainVS, nullptr);
    glCompileShader(mainVert);
    checkShaderCompile(mainVert, "Main VS");

    GLuint mainFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mainFrag, 1, &mainFS, nullptr);
    glCompileShader(mainFrag);
    checkShaderCompile(mainFrag, "Main FS");

    GLuint mainProgram = glCreateProgram();
    glAttachShader(mainProgram, mainVert);
    glAttachShader(mainProgram, mainFrag);
    glLinkProgram(mainProgram);
    checkProgramLink(mainProgram);
    glDeleteShader(mainVert);
    glDeleteShader(mainFrag);

    float lastTime = glfwGetTime();

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

        glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        auto renderDepth = [&](const glm::mat4& model, GLuint vao, int count) {
            glUniformMatrix4fv(glGetUniformLocation(depthProgram, "model"), 1, GL_FALSE, &model[0][0]);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, count);
            };

        if (gameState == PLAYING) {
            renderDepth(glm::mat4(1.0f), groundVAO, groundVertices.size() / 8);

            glm::mat4 playerModel = glm::scale(glm::translate(glm::mat4(1.0f), playerPos), glm::vec3(IRONMAN_SCALE));
            renderDepth(playerModel, playerVAO, playerVertexCount);

            for (const auto& obs : obstacles) {
                if (obs.active) {
                    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), obs.position), obs.scale * 0.15f);
                    renderDepth(m, cubeVAO, cubeVertexCount);
                }
            }

            for (const auto& col : collectibles) {
                if (col.active) {
                    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), col.position), col.scale * 0.12f);
                    renderDepth(m, sphereVAO, sphereVertexCount);
                }
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

            cameraPos = playerPos - cameraDir * cameraDistance + glm::vec3(0.0f, cameraHeight, 0.0f);
            if (cameraPos.y < 2.0f) {
                cameraPos = playerPos - cameraDir * 3.0f + glm::vec3(0.0f, cameraHeight, 0.0f);
                if (cameraPos.y < 0.5f) cameraPos.y = 0.5f;
                view = glm::lookAt(cameraPos, playerPos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else {
                view = glm::lookAt(cameraPos, playerPos, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }
        else {
            cameraPos = glm::vec3(0.0f, 5.0f, 10.0f);
            view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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

        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, groundVertices.size() / 8);

        glUniform1i(glGetUniformLocation(mainProgram, "useTexture"), 0);

        for (int i = 0; i < NUM_CLOUDS; ++i) {
            if (cloudVertexCount[i % 5] > 0) {
                glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.95f, 0.95f, 1.0f);
                glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 2.0f);
                glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), cloudPos[i]), glm::vec3(cloudScale[i]));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &model[0][0]);
                glBindVertexArray(cloudVAO[i % 5]);
                glDrawArrays(GL_TRIANGLES, 0, cloudVertexCount[i % 5]);
            }
        }

        if (treeVertexCount > 0) {
            glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.6f, 0.5f, 0.3f);
            glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.2f);
            for (int i = 0; i < NUM_TREES; ++i) {
                glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), treePos[i]), glm::vec3(treeScale[i]));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &model[0][0]);
                glBindVertexArray(treeVAO);
                glDrawArrays(GL_TRIANGLES, 0, treeVertexCount);
            }
        }

        if (gameState == PLAYING) {
            glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.3f, 0.5f, 0.9f);
            glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.0f);
            glm::mat4 playerModel = glm::scale(glm::translate(glm::mat4(1.0f), playerPos), glm::vec3(IRONMAN_SCALE));
            glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &playerModel[0][0]);
            glBindVertexArray(playerVAO);
            glDrawArrays(GL_TRIANGLES, 0, playerVertexCount);

            for (const auto& obs : obstacles) {
                if (obs.active) {
                    glUniform3fv(glGetUniformLocation(mainProgram, "objectColor"), 1, &obs.color[0]);
                    glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.0f);
                    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), obs.position), obs.scale * 0.15f);
                    glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &m[0][0]);
                    glBindVertexArray(cubeVAO);
                    glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
                }
            }

            for (const auto& col : collectibles) {
                if (col.active) {
                    glUniform3fv(glGetUniformLocation(mainProgram, "objectColor"), 1, &col.color[0]);
                    glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.8f);
                    glm::mat4 m = glm::rotate(glm::translate(glm::mat4(1.0f), col.position),
                        currentTime * 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                    m = glm::scale(m, col.scale * 0.12f);
                    glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &m[0][0]);
                    glBindVertexArray(sphereVAO);
                    glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
                }
            }
        }

        glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 1.0f, 1.0f, 0.2f);
        glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 2.0f);
        glm::mat4 sunModel = glm::scale(glm::translate(glm::mat4(1.0f), sunPos), glm::vec3(sunScale));
        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &sunModel[0][0]);
        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (gameState == MENU) {
            ImGui::SetNextWindowPos(ImVec2(currentWidth / 2.0f - 300, currentHeight / 2.0f - 250));
            ImGui::SetNextWindowSize(ImVec2(600, 500));
            ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

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