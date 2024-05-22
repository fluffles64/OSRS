#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include "../server/common.h"

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

// Networking
std::unordered_map<uint32_t, sPlayerDescription> mapObjects;
uint32_t nPlayerID = 0;
sPlayerDescription descPlayer;
bool bWaitingForConnection = true;
float deltaTime = 0.0f;

Mix_Chunk* miningSound = nullptr;
Mix_Chunk* oreObtainedSound1 = nullptr;
Mix_Chunk* oreObtainedSound2 = nullptr;
Mix_Chunk* oreObtainedSound3 = nullptr;
Mix_Chunk* oreObtainedSound4 = nullptr;
Mix_Chunk* levelupSound = nullptr;
Mix_Chunk* shopOpenSound = nullptr;
Mix_Chunk* shopCloseSound = nullptr;
Mix_Chunk* haggleSound1 = nullptr;
Mix_Chunk* haggleSound2 = nullptr;
Mix_Chunk* haggleSound3 = nullptr;

bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool key4Pressed = false;
bool key5Pressed = false;
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

    SDL_Surface* iconSurface = IMG_Load("../../../media/icon.png");
    if (iconSurface == NULL) {
        std::cerr << "Failed to load icon image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetWindowIcon(window, iconSurface);

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    const std::string fontPath = "../../../media/Ac437_IBM_VGA_9x8.ttf";
    font = TTF_OpenFont(fontPath.c_str(), BLOCK_SIZE);
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    miningSound = Mix_LoadWAV("../../../media/sound/breakingStone.wav");
    if (miningSound == nullptr) {
        std::cerr << "Failed to load mining sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    oreObtainedSound1 = Mix_LoadWAV("../../../media/sound/stone1.wav");
    if (oreObtainedSound1 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    oreObtainedSound2 = Mix_LoadWAV("../../../media/sound/stone2.wav");
    if (oreObtainedSound2 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    oreObtainedSound3 = Mix_LoadWAV("../../../media/sound/stone3.wav");
    if (oreObtainedSound3 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    oreObtainedSound4 = Mix_LoadWAV("../../../media/sound/stone4.wav");
    if (oreObtainedSound4 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    levelupSound = Mix_LoadWAV("../../../media/sound/levelup.wav");
    if (levelupSound == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    shopOpenSound = Mix_LoadWAV("../../../media/sound/chestopen.wav");
    if (shopOpenSound == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    shopCloseSound = Mix_LoadWAV("../../../media/sound/chestclosed.wav");
    if (shopCloseSound == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    haggleSound1 = Mix_LoadWAV("../../../media/sound/haggle1.wav");
    if (haggleSound1 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    haggleSound2 = Mix_LoadWAV("../../../media/sound/haggle2.wav");
    if (haggleSound2 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    haggleSound3 = Mix_LoadWAV("../../../media/sound/haggle3.wav");
    if (haggleSound3 == nullptr) {
        std::cerr << "Failed to load ore obtained sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    Mix_VolumeChunk(miningSound, MIX_MAX_VOLUME / 1.5f); // 66% volume
    Mix_VolumeChunk(oreObtainedSound1, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(oreObtainedSound2, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(oreObtainedSound3, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(oreObtainedSound4, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(levelupSound, MIX_MAX_VOLUME / 10); // 10% volume
    Mix_VolumeChunk(shopOpenSound, MIX_MAX_VOLUME / 5); // 10% volume
    Mix_VolumeChunk(shopCloseSound, MIX_MAX_VOLUME / 10); // 10% volume
    Mix_VolumeChunk(haggleSound1, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(haggleSound2, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(haggleSound3, MIX_MAX_VOLUME / 5); // 20% volume

    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

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

SDL_Color colorToSDLColor(const Color& color) {
    SDL_Color sdlColor;
    sdlColor.r = color.r;
    sdlColor.g = color.g;
    sdlColor.b = color.b;
    sdlColor.a = 255;
    return sdlColor;
}

void renderText(Uint16 unicodeValue, SDL_Rect rect, Color color) {
    SDL_Surface* textSurface = TTF_RenderGlyph_Solid(font, unicodeValue, colorToSDLColor(color));
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

void renderOreCounter(uint32_t oreCount) {
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
#pragma endregion