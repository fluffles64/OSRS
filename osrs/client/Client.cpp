#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

const int WINDOW_WIDTH = 480;
const int WINDOW_HEIGHT = 480;
const int PLAYER_SIZE = 20;
const int BLOCK_SIZE = 20;
const int PLAYER_SPEED = 1;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
SDL_Rect playerRect = { WINDOW_WIDTH / 2 - PLAYER_SIZE / 2, WINDOW_HEIGHT / 2 - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE };

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Simple Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    font = TTF_OpenFont("arial.ttf", 24);
    if (font == NULL) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

void close() {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            close();
            exit(0);
        }
    }

    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    if (currentKeyStates[SDL_SCANCODE_W]) {
        if (playerRect.y > BLOCK_SIZE)
            playerRect.y -= PLAYER_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_S]) {
        if (playerRect.y + playerRect.h < WINDOW_HEIGHT - BLOCK_SIZE)
            playerRect.y += PLAYER_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_A]) {
        if (playerRect.x > BLOCK_SIZE)
            playerRect.x -= PLAYER_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_D]) {
        if (playerRect.x + playerRect.w < WINDOW_WIDTH - BLOCK_SIZE)
            playerRect.x += PLAYER_SPEED;
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render blocks as borders
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect topBorder = { 0, 0, WINDOW_WIDTH, BLOCK_SIZE };
    SDL_Rect bottomBorder = { 0, WINDOW_HEIGHT - BLOCK_SIZE, WINDOW_WIDTH, BLOCK_SIZE };
    SDL_Rect leftBorder = { 0, 0, BLOCK_SIZE, WINDOW_HEIGHT };
    SDL_Rect rightBorder = { WINDOW_WIDTH - BLOCK_SIZE, 0, BLOCK_SIZE, WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &topBorder);
    SDL_RenderFillRect(renderer, &bottomBorder);
    SDL_RenderFillRect(renderer, &leftBorder);
    SDL_RenderFillRect(renderer, &rightBorder);

    // Render player
    SDL_Color color = { 255, 255, 0 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, ":)", color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { playerRect.x, playerRect.y, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* args[]) {
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return 1;
    }

    bool quit = false;
    while (!quit) {
        handleInput();
        render();
        SDL_Delay(10); // Add a short delay to control the speed
    }

    close();
    return 0;
}