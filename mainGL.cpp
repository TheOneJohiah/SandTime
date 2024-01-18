#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int width = 300;
const int height = 200;
const float pixelSize = 5.0f; // Adjust the size of each pixel
bool leftMousePressed = false;
bool rightMousePressed = false;

enum PixelType {
    EMPTY,
    SAND
};

std::vector<std::vector<PixelType>> grid(height, std::vector<PixelType>(width, EMPTY));

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        leftMousePressed = (action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rightMousePressed = (action == GLFW_PRESS);
    }
}

void initializeGrid() {
    std::srand(std::time(0));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            grid[i][j] = (std::rand() % 2 == 0) ? EMPTY : SAND;
        }
    }
}

void drawGrid(GLFWwindow* window) {
    // Draw using VBOs and VAOs
    GLuint vertexArray, vertexBuffer;
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    // Convert grid data to vertex data
    std::vector<float> vertices;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            float x = (2.0f * j / width - 1.0f) * pixelSize;
            float y = (2.0f * i / height - 1.0f) * pixelSize;
            float color = (grid[i][j] == SAND) ? 1.0f : 0.0f;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(color);
        }
    }

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Define vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load and use a simple shader program
    const char* vertexShaderSource = R"(
        #version 460 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in float aColor;
        out float color;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            color = aColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 460 core
        in float color;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(color, color, color, 1.0);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);

    // Draw
    glPointSize(pixelSize);
    glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &vertexBuffer);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void simulateFallingSand() {
    for (int i = height - 2; i >= 0; --i) {
        for (int j = 0; j < width; ++j) {
            if (grid[i][j] == SAND) {
                if (grid[i + 1][j] == EMPTY) {
                    std::swap(grid[i][j], grid[i + 1][j]);
                } else {
                    int direction = (std::rand() % 2 == 0) ? 1 : -1;
                    if (grid[i + 1][j + direction] == EMPTY) {
                        std::swap(grid[i][j], grid[i + 1][j + direction]);
                    }
                }
            }
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (4.6 in this case)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(width * pixelSize, height * pixelSize, "Falling Sand Simulation", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    initializeGrid();

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black clear color
        glClear(GL_COLOR_BUFFER_BIT);

        // Get cursor position
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Convert cursor position to grid indices
        int gridX = static_cast<int>((mouseX + 1.0) / 2.0 * width);
        int gridY = static_cast<int>((mouseY + 1.0) / 2.0 * height);

        // Draw or remove sand based on mouse input
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            grid[gridY][gridX] = SAND;
        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            grid[gridY][gridX] = EMPTY;
        }

        // Simulate and draw falling sand
        simulateFallingSand();
        drawGrid(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}