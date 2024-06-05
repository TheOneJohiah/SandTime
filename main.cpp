#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int width = 400;
const int height = 300;
const int pixelSize = 3; // Adjust the size of each pixel
bool leftMousePressed = false;
bool rightMousePressed = false;
// Constants for controlling draw amount
const int minDrawAmount = 1;
const int maxDrawAmount = 15;
int drawAmount = 1; // Initial draw amount
int gridX;
int gridY;
int selectedMat = 2;

enum PixelType {
    EMPTY,
    SAND,
    WATER,
    STONE
};

std::vector<std::vector<PixelType>> grid(height, std::vector<PixelType>(width, EMPTY));

void handleInput(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        leftMousePressed = (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED);
        rightMousePressed = (event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_PRESSED);
    } else if (event.type == SDL_MOUSEWHEEL) {
        // Handle mouse wheel scrolling to adjust draw amount
        if (event.wheel.y > 0 && drawAmount < maxDrawAmount) {
            drawAmount += 2;
        } else if (event.wheel.y < 0 && drawAmount > minDrawAmount) {
            drawAmount -= 2;
        }
    }
    else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_1:
            selectedMat = 1;
            break;
        case SDLK_2:
            selectedMat = 2;
            break;
        case SDLK_3:
            selectedMat = 3;
            break;
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
            SDL_Rect pixelRect = { j * pixelSize, i * pixelSize, pixelSize, pixelSize };


            int colorVariation = std::rand() % 21 - 10;
            switch (grid[i][j]) {
                case SAND:
                    // Slight randomization of color
                    SDL_SetRenderDrawColor(renderer, 255, 220, 180 + colorVariation, 255);
                    break;
                case EMPTY:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    break;
                case WATER:
                    SDL_SetRenderDrawColor(renderer, 10, 40, 220 + colorVariation, 200);
                    break;
                case STONE:
                    SDL_SetRenderDrawColor(renderer, 150, 130, 180 + colorVariation, 255);
                    break;
                default:
                    // code block
                    break;
            }
            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }
    SDL_Rect pixelRect = { (gridX - drawAmount / 2) * pixelSize, (gridY - drawAmount / 2) * pixelSize, drawAmount  * pixelSize, drawAmount  * pixelSize };
    SDL_SetRenderDrawColor(renderer, 0, 60, 240, 150);
    SDL_RenderFillRect(renderer, &pixelRect);
    SDL_RenderPresent(renderer);
}

void simulateFallingSand() {
    for (int i = height - 2; i >= 0; --i) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] == SAND) {
                if (grid[i + 1][j] == EMPTY) {
                    std::swap(grid[i][j], grid[i + 1][j]);
                } else {
                    int direction = (j == 0) ? 1 : ((j == width - 1) ? -1: ((std::rand() % 2 == 0) ? 1 : -1));
                    if (grid[i + 1][j + direction] == EMPTY) {
                        std::swap(grid[i][j], grid[i + 1][j + direction]);
                    }
                }
            }
        }
    }
}
void optimisedFallingSand() {
    for (int j = 0; j < width; j++) {
        for (int i = height - 2; i >= 0; --i) {
            if (grid[i][j] == SAND) {
                if (grid[i + 1][j] == EMPTY || grid[i + 1][j] == WATER) {
                    for (int k = i + 1; k >=0; k++)
                    {
                        if ((grid[k][j] == EMPTY || grid[i + 1][j] == WATER) || k == 0)
                        {
                            std::swap(grid[i][j], grid[k][j]);
                            i = k;
                            break;
                        }
                    }
                }
                else {
                    int direction = (j == 0) ? 1 : ((j == width - 1) ? -1 : ((std::rand() % 2 == 0) ? 1 : -1));
                    if (grid[i + 1][j + direction] == EMPTY) {
                        std::swap(grid[i][j], grid[i + 1][j + direction]);
                    }
                }
            }
        }
    }
}

//when more than 20 pixels wide of water is placed down, it will fill left first (because the loop starts on left most pixel)
//this is done to increase performance, but can be removed if nessasary. (the abs() functions dictate this behaviour)
void waterPhysics() {
    for (int i = height - 2; i >= 0; --i) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] == WATER) {
                // Randomly choose left or right direction
                int direction = (std::rand() % 2 == 0) ? 1 : -1;

                // Search further to the sides until an empty cell is found
                int J = j;
                while (J >= 0 && J < width) {
                    if (grid[i + 1][J] == EMPTY)
                    {
                        std::swap(grid[i][j], grid[i + 1][J]);
                        break;
                    }
                    else if (abs(j - J) > 20 && grid[i][J] == EMPTY)
                    {
                        std::swap(grid[i][j], grid[i][J]);
                        break;
                    }
                    else if (grid[i + 1][J] != WATER || abs(j-J) > 20)
                    {
                        break;
                    }
                    J += direction;
                }if (j > 0 && j < width - 1) {
                    if (grid[i][j + direction] == EMPTY)
                    {
                        std::swap(grid[i][j], grid[i][j + direction]);
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

    SDL_Window* window = SDL_CreateWindow("Particle Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * pixelSize, height * pixelSize, SDL_WINDOW_SHOWN);
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
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);

    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            handleInput(event);
        }

        // Simulate and draw falling sand
        //simulateFallingSand();
        optimisedFallingSand();

        waterPhysics();


        // Get cursor position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Convert cursor position to grid indices
        gridX = static_cast<int>(mouseX / pixelSize);
        gridY = static_cast<int>(mouseY / pixelSize);

        // Draw or remove pixels based on mouse input and draw amount
        if (leftMousePressed && gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            // Place pixles based on the draw amount
            for (int i = std::max(0, gridY - drawAmount / 2); i < std::min(height, gridY + drawAmount / 2 + 1); ++i) {
                for (int j = std::max(0, gridX - drawAmount / 2); j < std::min(width, gridX + drawAmount / 2 + 1); ++j) {
                    grid[i][j] = static_cast<PixelType>(selectedMat);
                }
            }
        } else if (rightMousePressed && gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            // Place empty based on the draw amount
            for (int i = std::max(0, gridY - drawAmount / 2); i < std::min(height, gridY + drawAmount / 2 + 1); ++i) {
                for (int j = std::max(0, gridX - drawAmount / 2); j < std::min(width, gridX + drawAmount / 2 + 1); ++j) {
                    grid[i][j] = EMPTY;
                }
            }
        }

        drawGrid(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}