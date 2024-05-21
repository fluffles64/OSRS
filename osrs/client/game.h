#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include <string>

#pragma region Variables
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int PLAYER_SIZE = 20;
const int BLOCK_SIZE = 20;
const float PLAYER_SPEED = 200.0f; // Pixels/second

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
SDL_Rect playerRect = { WINDOW_WIDTH / 2 - PLAYER_SIZE / 2 - 60, WINDOW_HEIGHT / 2 - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE };
SDL_Texture* shopImageTexture = NULL;

float playerPosX = playerRect.x;
float playerPosY = playerRect.y;
float playerVelX = 0.0f;
float playerVelY = 0.0f;

int oreCount = 0;
float miningSpeed = 1.0f;
bool shopOpen = false;
bool spacePressed = false;
#pragma endregion

#pragma region Init/Close
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("O.S.R.S", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

    // Three ../ for the .exe to work, two for the local windows debugger
    const std::string fontPath = "../../../media/Ac437_IBM_VGA_9x8.ttf";
    //const std::string fontPath = "../../media/Ac437_IBM_VGA_9x8.ttf";
    font = TTF_OpenFont(fontPath.c_str(), BLOCK_SIZE);
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    return true;
}

void close() {
    if (shopImageTexture != NULL) {
        SDL_DestroyTexture(shopImageTexture);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
#pragma endregion

#pragma region Rendering
SDL_Texture* loadTexture(const std::string& path) {
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    }
    else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

void renderText(Uint16 unicodeValue, SDL_Rect rect) {
    SDL_Color color = { 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderGlyph_Solid(font, unicodeValue, color);
    if (textSurface == nullptr) {
        std::cerr << "Failed to render text! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "Failed to create texture from surface! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }
    SDL_RenderCopy(renderer, textTexture, NULL, &rect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderBorders() {
    for (int x = 0; x < WINDOW_WIDTH; x += BLOCK_SIZE) {
        renderText(0x2593, { x, 0, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int x = 0; x < WINDOW_WIDTH; x += BLOCK_SIZE) {
        renderText(0x2593, { x, WINDOW_HEIGHT - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int y = 0; y < WINDOW_HEIGHT; y += BLOCK_SIZE) {
        renderText(0x2593, { 0, y, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int y = 0; y < WINDOW_HEIGHT; y += BLOCK_SIZE) {
        renderText(0x2593, { WINDOW_WIDTH - BLOCK_SIZE, y, BLOCK_SIZE, BLOCK_SIZE });
    }
}

void renderShop() {
    renderText(0x2502, { 60, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x2502, { 70, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x01, { 100, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x2502, { 150, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x2502, { 160, 20, BLOCK_SIZE, BLOCK_SIZE });

    renderText(0x2514, { 60, 40, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x2514, { 70, 30, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x2518, { 150, 30, BLOCK_SIZE, BLOCK_SIZE });
    renderText(0x2518, { 160, 40, BLOCK_SIZE, BLOCK_SIZE });

    for (int x = 80; x < 160; x += BLOCK_SIZE) {
        renderText(0x2500, { x, 30, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int x = 80; x < 160; x += BLOCK_SIZE) {
        renderText(0x2500, { x, 40, BLOCK_SIZE, BLOCK_SIZE });
    }
}

void renderRock() {
    renderText(0x1E, { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2), (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE });
}

void renderOreCounter() {
    std::string oreText = "Ore: " + std::to_string(oreCount);

    // Set the color for the text
    SDL_Color textColor = { 255, 255, 255 };

    // Render the text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, oreText.c_str(), textColor);
    if (textSurface == nullptr) {
        std::cerr << "Failed to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    // Create texture from surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "Failed to create texture from surface! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    // Get the width and height of the rendered text
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    // Set the position and size of the rendered text
    SDL_Rect destRect = { WINDOW_WIDTH - textWidth - 30, 30, textWidth - 5, textHeight - 5 };

    // Copy texture to renderer
    SDL_RenderCopy(renderer, textTexture, NULL, &destRect);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderWorld() {
    renderBorders();
    renderShop();
    renderRock();
    renderOreCounter();
}

void renderPlayer() {
    renderText(0x01, playerRect);
}
#pragma endregion

#pragma region Movement/Input/Collisions
bool AABB(const SDL_Rect& rectA, const SDL_Rect& rectB) {
    return rectA.x < rectB.x + rectB.w &&
        rectA.x + rectA.w > rectB.x &&
        rectA.y < rectB.y + rectB.h &&
        rectA.y + rectA.h > rectB.y;
}

void handleInput(float deltaTime) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            close();
            exit(0);
        }
    }

    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    if (shopOpen) {
        if (currentKeyStates[SDL_SCANCODE_SPACE] && !spacePressed) {
            spacePressed = true;
            shopOpen = false;
            SDL_DestroyTexture(shopImageTexture);
            shopImageTexture = NULL;
        }
        else if (!currentKeyStates[SDL_SCANCODE_SPACE]) {
            spacePressed = false;
        }

        if (currentKeyStates[SDL_SCANCODE_1]) {
            if (miningSpeed < 10.0f && oreCount >= 15)
                miningSpeed = 10.0f;
        }
        else if (currentKeyStates[SDL_SCANCODE_2]) {
            if (miningSpeed < 31.4159f && oreCount >= 500)
                miningSpeed = 31.4159f;
        }
        else if (currentKeyStates[SDL_SCANCODE_3]) {
            if (miningSpeed < 100.0f && oreCount >= 2000)
                miningSpeed = 100.0f;
        }
        else if (currentKeyStates[SDL_SCANCODE_4]) {
            if (miningSpeed < 1024.0f && oreCount >= 4090)
                miningSpeed = 1024.0f;
        }
        else if (currentKeyStates[SDL_SCANCODE_5]) {
            if (miningSpeed < 10000.0f && oreCount >= 100000)
                miningSpeed = 10000.0f;
        }
        return;
    }

    playerVelX = 0.0f;
    playerVelY = 0.0f;

    if (currentKeyStates[SDL_SCANCODE_W]) {
        playerVelY = -PLAYER_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_S]) {
        playerVelY = PLAYER_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_A]) {
        playerVelX = -PLAYER_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_D]) {
        playerVelX = PLAYER_SPEED;
    }

    static Uint32 lastSpacePress = 0;
    Uint32 currentTicks = SDL_GetTicks();

    SDL_Rect shopRect = { 70 - 5, 30 - 5, 100 + 10, 20 + 10 };

    if (currentKeyStates[SDL_SCANCODE_SPACE] && !spacePressed) {
        spacePressed = true;

        if (currentTicks - lastSpacePress > 500) {
            lastSpacePress = currentTicks;

            if (AABB(playerRect, shopRect)) {
                shopOpen = !shopOpen;

                if (shopOpen) {
                    shopImageTexture = loadTexture("../../media/shop.png");
                    if (shopImageTexture == NULL) {
                        std::cerr << "Failed to load shop image!" << std::endl;
                        shopOpen = false;
                    }
                }
                else {
                    SDL_DestroyTexture(shopImageTexture);
                    shopImageTexture = NULL;
                }
            }
        }
    }
    else if (!currentKeyStates[SDL_SCANCODE_SPACE]) {
        spacePressed = false;
    }

    SDL_Rect rockRect = { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2) - 5, (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2) - 5, BLOCK_SIZE + 10, BLOCK_SIZE + 10 };
    if (currentKeyStates[SDL_SCANCODE_SPACE] && AABB(playerRect, rockRect)) {
        static float accumulatedTime = 0.0f;
        accumulatedTime += deltaTime;

        while (accumulatedTime >= (1.0f / miningSpeed)) {
            oreCount++;
            accumulatedTime -= (1.0f / miningSpeed);
        }
    }
}

void movePlayer(float deltaTime) {
    float newPosX = playerPosX + playerVelX * deltaTime;
    float newPosY = playerPosY + playerVelY * deltaTime;

    // Boundary collision detection
    if (newPosX < BLOCK_SIZE) {
        newPosX = BLOCK_SIZE;
    }
    if (newPosX > WINDOW_WIDTH - BLOCK_SIZE - PLAYER_SIZE) {
        newPosX = WINDOW_WIDTH - BLOCK_SIZE - PLAYER_SIZE;
    }
    if (newPosY < BLOCK_SIZE) {
        newPosY = BLOCK_SIZE;
    }
    if (newPosY > WINDOW_HEIGHT - BLOCK_SIZE - PLAYER_SIZE) {
        newPosY = WINDOW_HEIGHT - BLOCK_SIZE - PLAYER_SIZE;
    }

    // Shop and rock collision detection
    SDL_Rect shopRect = { 70, 30, 100, 20 };
    SDL_Rect rockRect = { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2), (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE };

    if (!AABB({ static_cast<int>(newPosX), static_cast<int>(newPosY), playerRect.w, playerRect.h }, shopRect) &&
        !AABB({ static_cast<int>(newPosX), static_cast<int>(newPosY), playerRect.w, playerRect.h }, rockRect)) {
        playerPosX = newPosX;
        playerPosY = newPosY;
    }

    playerRect.x = static_cast<int>(playerPosX);
    playerRect.y = static_cast<int>(playerPosY);
}
#pragma endregion