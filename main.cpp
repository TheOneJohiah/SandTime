#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int width = 600;
const int height = 300;
const int pixelSize = 3; // Adjust the size of each pixel
bool leftMousePressed = false;
bool rightMousePressed = false;
// Constants for controlling sand amount
const int minSandAmount = 1;
const int maxSandAmount = 10;
int sandAmount = 1; // Initial sand amount

enum PixelType {
    EMPTY,
    SAND
};

std::vector<std::vector<PixelType>> grid(height, std::vector<PixelType>(width, EMPTY));

void handleInput(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        leftMousePressed = (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED);
        rightMousePressed = (event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_PRESSED);
    } else if (event.type == SDL_MOUSEWHEEL) {
        // Handle mouse wheel scrolling to adjust sand amount
        if (event.wheel.y > 0 && sandAmount < maxSandAmount) {
            sandAmount++;
        } else if (event.wheel.y < 0 && sandAmount > minSandAmount) {
            sandAmount--;
        }
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

void drawGrid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderClear(renderer);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            SDL_Rect pixelRect = {j * pixelSize, i * pixelSize, pixelSize, pixelSize};

            // Slight randomization of color
            int colorVariation = std::rand() % 21 - 10; // Range: -10 to 10
            SDL_SetRenderDrawColor(renderer, (grid[i][j] == SAND) ? 255 : 0, (grid[i][j] == SAND) ? 220 + colorVariation : 0, (grid[i][j] == SAND) ? 180 + colorVariation : 0, 255);

            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }

    SDL_RenderPresent(renderer);
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

int main(int argv, char** args) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Failed to initialize SDL" << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Falling Sand Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * pixelSize, height * pixelSize, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create SDL window" << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create SDL renderer" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    initializeGrid();

    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            handleInput(event);
        }

        // Get cursor position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Convert cursor position to grid indices
        int gridX = static_cast<int>(mouseX / pixelSize);
        int gridY = static_cast<int>(mouseY / pixelSize);

        // Draw or remove sand based on mouse input and sand amount
        if (leftMousePressed && gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            // Place sand based on the sand amount
            for (int i = std::max(0, gridY - sandAmount / 2); i < std::min(height, gridY + sandAmount / 2 + 1); ++i) {
                for (int j = std::max(0, gridX - sandAmount / 2); j < std::min(width, gridX + sandAmount / 2 + 1); ++j) {
                    grid[i][j] = SAND;
                }
            }
        } else if (rightMousePressed && gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            grid[gridY][gridX] = EMPTY;
        }

        // Simulate and draw falling sand
        simulateFallingSand();
        drawGrid(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}