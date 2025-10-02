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

// ============= CONFIGURAÇÕES =============
const std::string MODEL_PATH = "C:/Projetos/modeltest/external/models/teapot.obj";
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

// CONFIGURAÇÃO DE TELA CHEIA
// true = tela cheia, false = janela
const bool FULLSCREEN = false;
const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 900;

// CONFIGURAÇÕES DO CENÁRIO
const float WALL_LENGTH = 100.0f;  // Comprimento das paredes laterais
const float WALL_HEIGHT = 4.0f;     // Altura das paredes
const float WALL_THICKNESS = 0.5f;  // Espessura das paredes
const float LANE_WIDTH = 18.0f;     // Largura da pista (distância entre paredes)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
// Variáveis da tela
int currentWidth = WINDOW_WIDTH;
int currentHeight = WINDOW_HEIGHT;

// Variáveis do jogador
glm::vec3 playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
float playerSpeed = 0.05f;
float gameSpeed = 0.08f;

// Variáveis do jogo
int score = 0;
int highScore = 0;
bool gameOver = false;
float gameTime = 0.0f;

// Variáveis da câmera
float cameraDistance = 10.0f;
float cameraHeight = 5.0f;
float cameraYaw = -90.0f;
float cameraPitch = -20.0f;
float cameraRotSpeed = 0.8f;

// Objetos do jogo
std::vector<GameObject> obstacles;
std::vector<GameObject> collectibles;
const int MAX_OBJECTS = 15;
float spawnTimer = 0.0f;
float spawnInterval = 1.5f;

std::vector<Wall> walls;

// Geometria
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
GLuint groundTexture, coinTexture;

// ============= ATUALIZAÇÃO HUD =============
void updateWindowTitle(GLFWwindow* window, int score, int highScore) {
    std::string title = "Corrida 3D - Score: " + std::to_string(score) +
        " | High Score: " + std::to_string(highScore) +
        " | Controles: WASD + Setas | F11 = Tela Cheia";
    glfwSetWindowTitle(window, title.c_str());
}

// ============= CALLBACK PARA TECLAS =============
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Alternar tela cheia com F11
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

// ============= GERAÇÃO DE PRIMITIVAS =============
void generateCube(std::vector<float>& vertices) {
    float s = 1.0f;
    float cubeData[] = {
        -s,-s, s,  0, 0, 1,  0,0,
         s,-s, s,  0, 0, 1,  1,0,
         s, s, s,  0, 0, 1,  1,1,
        -s,-s, s,  0, 0, 1,  0,0,
         s, s, s,  0, 0, 1,  1,1,
        -s, s, s,  0, 0, 1,  0,1,

         s,-s,-s,  0, 0,-1,  0,0,
        -s,-s,-s,  0, 0,-1,  1,0,
        -s, s,-s,  0, 0,-1,  1,1,
         s,-s,-s,  0, 0,-1,  0,0,
        -s, s,-s,  0, 0,-1,  1,1,
         s, s,-s,  0, 0,-1,  0,1,

         s,-s, s,  1, 0, 0,  0,0,
         s,-s,-s,  1, 0, 0,  1,0,
         s, s,-s,  1, 0, 0,  1,1,
         s,-s, s,  1, 0, 0,  0,0,
         s, s,-s,  1, 0, 0,  1,1,
         s, s, s,  1, 0, 0,  0,1,

        -s,-s,-s, -1, 0, 0,  0,0,
        -s,-s, s, -1, 0, 0,  1,0,
        -s, s, s, -1, 0, 0,  1,1,
        -s,-s,-s, -1, 0, 0,  0,0,
        -s, s, s, -1, 0, 0,  1,1,
        -s, s,-s, -1, 0, 0,  0,1,

        -s, s, s,  0, 1, 0,  0,0,
         s, s, s,  0, 1, 0,  1,0,
         s, s,-s,  0, 1, 0,  1,1,
        -s, s, s,  0, 1, 0,  0,0,
         s, s,-s,  0, 1, 0,  1,1,
        -s, s,-s,  0, 1, 0,  0,1,

        -s,-s,-s,  0,-1, 0,  0,0,
         s,-s,-s,  0,-1, 0,  1,0,
         s,-s, s,  0,-1, 0,  1,1,
        -s,-s,-s,  0,-1, 0,  0,0,
         s,-s, s,  0,-1, 0,  1,1,
        -s,-s, s,  0,-1, 0,  0,1,
    };

    vertices.assign(cubeData, cubeData + sizeof(cubeData) / sizeof(float));
}

void generateSphere(std::vector<float>& vertices, int segments = 20) {
    float radius = 1.0f;
    std::vector<float> tempVerts;

    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0 * M_PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            float u = (float)lon / segments;
            float v = (float)lat / segments;

            tempVerts.push_back(x * radius);
            tempVerts.push_back(y * radius);
            tempVerts.push_back(z * radius);
            tempVerts.push_back(x);
            tempVerts.push_back(y);
            tempVerts.push_back(z);
            tempVerts.push_back(u);
            tempVerts.push_back(v);
        }
    }

    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = (lat * (segments + 1)) + lon;
            int second = first + segments + 1;

            int idx1 = first * 8;
            int idx2 = second * 8;
            int idx3 = (first + 1) * 8;
            int idx4 = (second + 1) * 8;

            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx1 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx2 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx3 + i]);

            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx3 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx2 + i]);
            for (int i = 0; i < 8; i++) vertices.push_back(tempVerts[idx4 + i]);
        }
    }
}

// ============= TEXTURAS =============
GLuint generateGrassTexture() {
    int width = 256, height = 256;
    std::vector<unsigned char> data(width * height * 3);

    glm::vec3 grass1(0.4f, 0.7f, 0.3f);
    glm::vec3 grass2(0.35f, 0.65f, 0.25f);
    glm::vec3 grass3(0.45f, 0.75f, 0.35f);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;

            int block = ((x / 16) + (y / 16)) % 3;
            glm::vec3 color;

            if (block == 0) color = grass1;
            else if (block == 1) color = grass2;
            else color = grass3;

            float noise = (rand() % 100) / 1000.0f;
            color += glm::vec3(noise);

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
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(0.0f);
                }

                if (idx.texcoord_index >= 0) {
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                }
                else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
            }
        }
    }
    return true;
}

void generateGround(std::vector<float>& vertices, int size = 30, float quadSize = 1.0f) {
    for (int x = -size; x < size; ++x) {
        for (int z = -size; z < size; ++z) {
            float x0 = x * quadSize;
            float x1 = (x + 1) * quadSize;
            float z0 = z * quadSize;
            float z1 = (z + 1) * quadSize;

            float u0 = x + size;
            float u1 = x + size + 1;
            float v0 = z + size;
            float v1 = z + size + 1;

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

void updateGame(float deltaTime, GLFWwindow* window) {
    if (gameOver) return;

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
                gameOver = true;
                if (score > highScore) highScore = score;
                updateWindowTitle(window, score, highScore);
                std::cout << "\n╔═══════════════════════════╗" << std::endl;
                std::cout << "║      GAME OVER!           ║" << std::endl;
                std::cout << "║                           ║" << std::endl;
                std::cout << "║  Score: " << score << " pontos      ║" << std::endl;
                std::cout << "║  High Score: " << highScore << "       ║" << std::endl;
                std::cout << "║                           ║" << std::endl;
                std::cout << "║  Pressione R para jogar   ║" << std::endl;
                std::cout << "╚═══════════════════════════╝" << std::endl;
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
                updateWindowTitle(window, score, highScore);
                std::cout << "★ MOEDA! +10 pontos | Score: " << score << std::endl;
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

    if (gameOver) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            gameOver = false;
            score = 0;
            gameTime = 0.0f;
            playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
            obstacles.clear();
            collectibles.clear();
            updateWindowTitle(window, score, highScore);
            std::cout << "\n=== NOVO JOGO ===\n" << std::endl;
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    currentWidth = width;
    currentHeight = height;
}

void checkShaderCompile(GLuint shader, const char* type) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << type << " error:\n" << infoLog << std::endl;
    }
}

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

    // Configurar janela ou tela cheia
    if (FULLSCREEN) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        window = glfwCreateWindow(mode->width, mode->height, "Corrida 3D - CG UFCA", monitor, nullptr);
        currentWidth = mode->width;
        currentHeight = mode->height;
    }
    else {
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Corrida 3D - CG UFCA", nullptr, nullptr);
        currentWidth = WINDOW_WIDTH;
        currentHeight = WINDOW_HEIGHT;
    }

    if (!window) {
        std::cerr << "Falha ao criar janela\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);  // Callback para F11
    updateWindowTitle(window, 0, 0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    if (!loadOBJ(MODEL_PATH, playerVertices)) {
        std::cerr << "Falha ao carregar modelo - usando cubo\n";
        generateCube(playerVertices);
    }
    playerVertexCount = playerVertices.size() / 8;

    generateCube(cubeVertices);
    cubeVertexCount = cubeVertices.size() / 8;

    generateSphere(sphereVertices, 15);
    sphereVertexCount = sphereVertices.size() / 8;

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

    // ============= PAREDES AJUSTADAS =============
    // As paredes agora cobrem toda a extensão da pista
    float halfWidth = LANE_WIDTH / 2.0f;
    float wallStartZ = -WALL_LENGTH / 2.0f;  // Começa bem atrás
    float wallCenterZ = 0.0f;                 // Centro da pista

    walls = {
        // Parede esquerda (X negativo)
        {
            glm::vec3(-halfWidth, WALL_HEIGHT / 2.0f, wallCenterZ),
            glm::vec3(WALL_THICKNESS, WALL_HEIGHT, WALL_LENGTH)
        },
        // Parede direita (X positivo)
        {
            glm::vec3(halfWidth, WALL_HEIGHT / 2.0f, wallCenterZ),
            glm::vec3(WALL_THICKNESS, WALL_HEIGHT, WALL_LENGTH)
        },
        // Parede de trás (opcional - pode remover se quiser)
        {
            glm::vec3(0.0f, WALL_HEIGHT / 2.0f, wallStartZ),
            glm::vec3(LANE_WIDTH + WALL_THICKNESS * 2, WALL_HEIGHT, WALL_THICKNESS)
        }
    };

    groundTexture = generateGrassTexture();
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
    std::cout << "╠═════════════════════════════════════════╣" << std::endl;
    std::cout << "║  CONTROLES:                             ║" << std::endl;
    std::cout << "║    WASD - Mover jogador                 ║" << std::endl;
    std::cout << "║    Setas - Rotacionar camera            ║" << std::endl;
    std::cout << "║    F11 - Alternar tela cheia            ║" << std::endl;
    std::cout << "║    R - Reiniciar apos Game Over         ║" << std::endl;
    std::cout << "║                                         ║" << std::endl;
    std::cout << "║  OBJETIVO:                              ║" << std::endl;
    std::cout << "║    Desvie dos CACTOS VERMELHOS!         ║" << std::endl;
    std::cout << "║    Colete MOEDAS DOURADAS! (+10pts)     ║" << std::endl;
    std::cout << "╚═════════════════════════════════════════╝\n" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window);
        updateGame(deltaTime, window);

        glm::vec3 lightPos(5.0f, 20.0f, 10.0f);

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

        glm::mat4 groundModel = glm::mat4(1.0f);
        renderDepth(groundModel, groundVAO, groundVertices.size() / 8);

        glm::mat4 playerModel = glm::translate(glm::mat4(1.0f), playerPos);
        playerModel = glm::scale(playerModel, glm::vec3(0.15f));
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

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Usar dimensões dinâmicas para viewport
        glViewport(0, 0, currentWidth, currentHeight);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float yawRad = glm::radians(cameraYaw);
        float pitchRad = glm::radians(cameraPitch);
        glm::vec3 cameraDir(cos(yawRad) * cos(pitchRad), sin(pitchRad), sin(yawRad) * cos(pitchRad));
        cameraDir = glm::normalize(cameraDir);

        glm::vec3 cameraPos = playerPos - cameraDir * cameraDistance + glm::vec3(0.0f, cameraHeight, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, playerPos, glm::vec3(0.0f, 1.0f, 0.0f));

        // Atualizar projeção com aspect ratio dinâmico
        float aspectRatio = (float)currentWidth / (float)currentHeight;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

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

        glUniformMatrix4fv(glGetUniformLocation(mainProgram, "model"), 1, GL_FALSE, &groundModel[0][0]);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, groundVertices.size() / 8);

        glUniform1i(glGetUniformLocation(mainProgram, "useTexture"), 0);

        glUniform3f(glGetUniformLocation(mainProgram, "objectColor"), 0.3f, 0.5f, 0.9f);
        glUniform1f(glGetUniformLocation(mainProgram, "brightness"), 1.0f);
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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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