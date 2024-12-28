#define SDL_MAIN_HANDLED

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int COLS = 10;
const int ROWS = 20;
const int BLOCK_SIZE = 30;

struct Block {
    int x, y;
    SDL_Color color;
};

struct Tetromino {
    int shape[4][4];    // Tetrmino will be in matrix 4 x 4
    SDL_Color color;
};

                                // Tetraminos

vector<Tetromino> tetrominos = {
    {{ {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, {255, 0, 0}},

 //                                 ####

    {{ {1, 1}, {1, 1}, {0, 0}, {0, 0} }, {0, 255, 0}},

 //                                   ##
 //                                   ##

    {{ {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, {0, 0, 255}},

 //                                  ##
 //                                   ##

    {{ {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, {255, 255, 0}},

 //                                   ##
 //                                  ##

    {{ {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, {255, 165, 0}},

 //                                  ###
 //                                  #

    {{ {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, {0, 255, 255}},

 //                                  ###
 //                                    #

    {{ {0, 1, 1, 1}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, {128, 0, 128}}

 //                                  ###
 //                                 #
};

int field[ROWS][COLS] = { 0 }; // field for game

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Mix_Chunk* blockSound = nullptr;
Mix_Chunk* lineSound = nullptr;

void init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, COLS * BLOCK_SIZE, ROWS * BLOCK_SIZE, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    blockSound = Mix_LoadWAV("block.wav");
    lineSound = Mix_LoadWAV("line.wav");
    srand(time(0));
}

void cleanUp() {
    Mix_FreeChunk(blockSound);
    Mix_FreeChunk(lineSound);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void drawBlock(SDL_Renderer* renderer, Block block) {
    SDL_Rect rect = {block.x * BLOCK_SIZE, block.y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_SetRenderDrawColor(renderer, block.color.r, block.color.g, block.color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void drawTetromino(SDL_Renderer* renderer, Tetromino tetromino, int offsetX, int offsetY) {
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (tetromino.shape[y][x] == 1) {
                Block block = {offsetX + x, offsetY + y, tetromino.color};
                drawBlock(renderer, block);
            }
        }
    }
}

bool checkCollision(int field[ROWS][COLS], Tetromino tetromino, int offsetX, int offsetY) {
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (tetromino.shape[y][x] == 1) {
                int px = offsetX + x;
                int py = offsetY + y;
                if (px < 0 || px >= COLS || py >= ROWS) {
                    return true;
                }
                if (py >= 0 && field[py][px] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

                    // LINES


void clearLines(int field[ROWS][COLS]) {
    for (int y = ROWS - 1; y >= 0; --y) {
        bool fullLine = true;
        for (int x = 0; x < COLS; ++x) {
            if (field[y][x] == 0) {
                fullLine = false;
                break;
            }
        }

                         // all lines down


        if (fullLine) {
            Mix_PlayChannel(-1, lineSound, 0);
            for (int yy = y; yy > 0; --yy) {
                for (int x = 0; x < COLS; ++x) {
                    field[yy][x] = field[yy - 1][x];
                }
            }

                             // remove top line


            for (int x = 0; x < COLS; ++x) {
                field[0][x] = 0;
            }
            ++y; // Check this line again
        }
    }
}

                    // rotate tetramino 90*(clock direction)

void rotateTetromino(Tetromino& tetromino) {
    int tempShape[4][4];
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            tempShape[x][3 - y] = tetromino.shape[y][x];
        }
    }
                    // input result(rotation) in tetramino

    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            tetromino.shape[y][x] = tempShape[y][x];
        }
    }
}

int main() {
    init();
    bool quit = false;
    SDL_Event e;

    Tetromino currentTetromino = tetrominos[rand() % tetrominos.size()];
    int currentX = COLS / 2 - 2;
    int currentY = 0;
    bool isFalling = true;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT) {
                    if (!checkCollision(field, currentTetromino, currentX - 1, currentY)) {
                        currentX--;
                    }
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    if (!checkCollision(field, currentTetromino, currentX + 1, currentY)) {
                        currentX++;
                    }
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    rotateTetromino(currentTetromino);
                    if (checkCollision(field, currentTetromino, currentX, currentY)) {
                        rotateTetromino(currentTetromino);
                        rotateTetromino(currentTetromino);
                        rotateTetromino(currentTetromino);
                    }
                }
            }
        }

        if (isFalling) {
            if (!checkCollision(field, currentTetromino, currentX, currentY + 1)) {
                currentY++;
            } else {
                Mix_PlayChannel(-1, blockSound, 0);
                for (int row = 0; row < 4; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        if (currentTetromino.shape[row][col] == 1) {
                            field[currentY + row][currentX + col] = 1;
                        }
                    }
                }
                clearLines(field);
                currentTetromino = tetrominos[rand() % tetrominos.size()];
                currentX = COLS / 2 - 2;
                currentY = 0;

                if (checkCollision(field, currentTetromino, currentX, currentY)) {
                    quit = true;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < ROWS; ++y) {
            for (int x = 0; x < COLS; ++x) {
                if (field[y][x] != 0) {
                    Block block = {x, y, {255, 255, 255}};
                    drawBlock(renderer, block);
                }
            }
        }

        drawTetromino(renderer, currentTetromino, currentX, currentY);
        SDL_RenderPresent(renderer);
        SDL_Delay(370);
    }
    cleanUp();
    return 0;
}
