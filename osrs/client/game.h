#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include "../server/common.h"

#pragma region Variables
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int PLAYER_SIZE = 20;
const int BLOCK_SIZE = 20;
const float PLAYER_SPEED = 200.0f;
const Uint8* currentKeyStates;
const char* iconPath = "../../../media/icon.png";
const std::string shopImagePath = "../../../media/shop.png";
const std::string scoreboardImagePath = "../../../media/scoreboard.png";
const std::string fontPath = "../../../media/Ac437_IBM_VGA_9x8.ttf";
const std::unordered_map<int, float> SHOP_SPEEDS = {
    {1, 10.0f},
    {2, 31.4159f},
    {3, 100.0f},
    {4, 1024.0f},
    {5, 10000.0f}
};
const std::unordered_map<int, int> SHOP_COSTS = {
    {1, 15},
    {2, 500},
    {3, 2000},
    {4, 4090},
    {5, 100000}
};
const SDL_Rect rockRect = { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2) - 5, (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2) - 5, BLOCK_SIZE + 10, BLOCK_SIZE + 10 };
const SDL_Rect shopRect = { 70 - 5, 30 - 5, 100 + 10, 20 + 10 };
const SDL_Rect scoreboardRect = { WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 };

SDL_Rect playerRect = { WINDOW_WIDTH / 2 - PLAYER_SIZE / 2 - 60, WINDOW_HEIGHT / 2 - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE };
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
SDL_Texture* shopImageTexture = NULL;
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
const std::vector<std::pair<Mix_Chunk*&, std::string>> soundFiles = {
    {miningSound, "../../../media/sound/breakingStone.wav"},
    {oreObtainedSound1, "../../../media/sound/stone1.wav"},
    {oreObtainedSound2, "../../../media/sound/stone2.wav"},
    {oreObtainedSound3, "../../../media/sound/stone3.wav"},
    {oreObtainedSound4, "../../../media/sound/stone4.wav"},
    {levelupSound, "../../../media/sound/levelup.wav"},
    {shopOpenSound, "../../../media/sound/chestopen.wav"},
    {shopCloseSound, "../../../media/sound/chestclosed.wav"},
    {haggleSound1, "../../../media/sound/haggle1.wav"},
    {haggleSound2, "../../../media/sound/haggle2.wav"},
    {haggleSound3, "../../../media/sound/haggle3.wav"}
};

bool shopOpen = false;
bool spacePressed = false;
bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool key4Pressed = false;
bool key5Pressed = false;
bool bWaitingForConnection = true;

std::unordered_map<uint32_t, sPlayerDescription> mapObjects;
uint32_t nPlayerID = 0;
sPlayerDescription descPlayer;
#pragma endregion

#pragma region Helper Methods
bool AABB(const SDL_Rect& rectA, const SDL_Rect& rectB) {
    return rectA.x < rectB.x + rectB.w &&
        rectA.x + rectA.w > rectB.x &&
        rectA.y < rectB.y + rectB.h &&
        rectA.y + rectA.h > rectB.y;
}

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

SDL_Color colorToSDLColor(const Color& color) {
    SDL_Color sdlColor;
    sdlColor.r = color.r;
    sdlColor.g = color.g;
    sdlColor.b = color.b;
    sdlColor.a = 255;
    return sdlColor;
}

SDL_Point getTextSize(const std::string& text) {
    int textWidth, textHeight;
    TTF_SizeText(font, text.c_str(), &textWidth, &textHeight);
    return { textWidth, textHeight };
}

void renderChar(Uint16 unicodeValue, SDL_Rect rect, Color color = { 255, 255, 255 }) {
    SDL_Color sdlColor = colorToSDLColor(color);
    SDL_Surface* textSurface = TTF_RenderGlyph_Solid(font, unicodeValue, sdlColor);
    if (textSurface == nullptr) {
        std::cerr << "Failed to render char! SDL_ttf Error: " << TTF_GetError() << std::endl;
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

void renderText(const std::string& text, SDL_Rect rect, Color color = { 255, 255, 255 }) {
    SDL_Color textColor = colorToSDLColor(color);

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (textSurface == nullptr) {
        std::cerr << "Failed to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "Failed to create texture from surface! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect destRect = { rect.x, rect.y, rect.w, rect.h };

    SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
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

    SDL_Surface* iconSurface = IMG_Load(iconPath);
    if (iconSurface == NULL) {
        std::cerr << "Failed to load icon image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetWindowIcon(window, iconSurface);

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

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

    auto loadSound = [&](Mix_Chunk*& sound, const std::string& path) {
        sound = Mix_LoadWAV(path.c_str());
        if (sound == nullptr) {
            std::cerr << "Failed to load sound effect: " << path << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
            return false;
        }
        return true;
    };

    for (const auto& soundFile : soundFiles) {
        if (!loadSound(soundFile.first, soundFile.second)) {
            return false;
        }
    }

    Mix_VolumeChunk(miningSound, MIX_MAX_VOLUME / 1.5f); // 66% volume
    Mix_VolumeChunk(oreObtainedSound1, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(oreObtainedSound2, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(oreObtainedSound3, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(oreObtainedSound4, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(levelupSound, MIX_MAX_VOLUME / 10); // 10% volume
    Mix_VolumeChunk(shopOpenSound, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(shopCloseSound, MIX_MAX_VOLUME / 10); // 10% volume
    Mix_VolumeChunk(haggleSound1, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(haggleSound2, MIX_MAX_VOLUME / 5); // 20% volume
    Mix_VolumeChunk(haggleSound3, MIX_MAX_VOLUME / 5); // 20% volume

    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    return true;
}

void close() {
    auto destroyTexture = [](SDL_Texture*& texture) {
        if (texture != NULL) {
            SDL_DestroyTexture(texture);
            texture = NULL;
        }
    };

    auto freeSound = [](Mix_Chunk*& sound) {
        if (sound != NULL) {
            Mix_FreeChunk(sound);
            sound = NULL;
        }
    };

    auto closeFont = [](TTF_Font*& font) {
        if (font != NULL) {
            TTF_CloseFont(font);
            font = NULL;
        }
    };

    // Destroy shop image texture
    destroyTexture(shopImageTexture);

    // Free loaded sound effects
    freeSound(miningSound);
    freeSound(oreObtainedSound1);
    freeSound(oreObtainedSound2);
    freeSound(oreObtainedSound3);
    freeSound(oreObtainedSound4);
    freeSound(levelupSound);
    freeSound(shopOpenSound);
    freeSound(shopCloseSound);
    freeSound(haggleSound1);
    freeSound(haggleSound2);
    freeSound(haggleSound3);

    // Close the font
    closeFont(font);

    // Destroy the renderer
    if (renderer != NULL) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }

    // Destroy the window
    if (window != NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    // Quit SDL subsystems
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
#pragma endregion

#pragma region Rendering
void renderWorld() {
    // Borders
    for (int x = 0; x < WINDOW_WIDTH; x += BLOCK_SIZE) {
        renderChar(0x2593, { x, 0, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int x = 0; x < WINDOW_WIDTH; x += BLOCK_SIZE) {
        renderChar(0x2593, { x, WINDOW_HEIGHT - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int y = 0; y < WINDOW_HEIGHT; y += BLOCK_SIZE) {
        renderChar(0x2593, { 0, y, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int y = 0; y < WINDOW_HEIGHT; y += BLOCK_SIZE) {
        renderChar(0x2593, { WINDOW_WIDTH - BLOCK_SIZE, y, BLOCK_SIZE, BLOCK_SIZE });
    }

    // Shop
    renderChar(0x2502, { 60, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x2502, { 70, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x01, { 100, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x2502, { 150, 20, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x2502, { 160, 20, BLOCK_SIZE, BLOCK_SIZE });

    renderChar(0x2514, { 60, 40, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x2514, { 70, 30, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x2518, { 150, 30, BLOCK_SIZE, BLOCK_SIZE });
    renderChar(0x2518, { 160, 40, BLOCK_SIZE, BLOCK_SIZE });

    for (int x = 80; x < 160; x += BLOCK_SIZE) {
        renderChar(0x2500, { x, 30, BLOCK_SIZE, BLOCK_SIZE });
    }
    for (int x = 80; x < 160; x += BLOCK_SIZE) {
        renderChar(0x2500, { x, 40, BLOCK_SIZE, BLOCK_SIZE });
    }

    // Rock
    renderChar(0x1E, { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2), (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE });
}

void renderOreCounter(uint32_t oreCount) {
    std::string oreText = "Ore: " + std::to_string(oreCount);
    SDL_Point textSize = getTextSize(oreText);
    SDL_Rect rect = { WINDOW_WIDTH - textSize.x - 30, 30, textSize.x - 5, textSize.y - 5 };
    renderText(oreText, rect);
}

void renderScoreboard(const std::unordered_map<uint32_t, sPlayerDescription>& mapObjects) {
    static SDL_Texture* scoreboardTexture = loadTexture(scoreboardImagePath);
    if (scoreboardTexture == nullptr) {
        std::cerr << "Failed to load scoreboard image!" << std::endl;
        return;
    }

    SDL_RenderCopy(renderer, scoreboardTexture, NULL, &scoreboardRect);

    std::vector<std::pair<uint32_t, sPlayerDescription>> players(mapObjects.begin(), mapObjects.end());
    std::sort(players.begin(), players.end(), [](const auto& a, const auto& b) {
        if (a.second.nOreCount == b.second.nOreCount) {
            // Order by ID if ore count is the same
            return a.first < b.first;
        }
        // Otherwise, order by ore count
        return a.second.nOreCount > b.second.nOreCount;
    });

    int displayCount = std::min(static_cast<int>(players.size()), 5);

    for (int i = 0; i < displayCount; ++i) {
        const auto& player = players[i];
        SDL_Color playerColor = colorToSDLColor(player.second.nColor);
        SDL_Rect charRect = { scoreboardRect.x + 20, scoreboardRect.y + 20 + i * 40, BLOCK_SIZE, BLOCK_SIZE };

        // Render player character
        renderChar(0x263A, charRect, player.second.nColor);

        // Render player ID
        std::string idText = std::to_string(player.first);
        int idTextWidth = idText.length() * BLOCK_SIZE;
        SDL_Rect idRect = { charRect.x + 50, charRect.y, idTextWidth, BLOCK_SIZE };
        renderText(idText, idRect, player.second.nColor);

        // Render player ore count
        std::string oreText = std::to_string(player.second.nOreCount);
        int oreTextWidth = oreText.length() * BLOCK_SIZE;
        SDL_Rect oreRect = { scoreboardRect.x + scoreboardRect.w - oreTextWidth - 20, charRect.y, oreTextWidth, BLOCK_SIZE };
        renderText(oreText, oreRect, player.second.nColor);
    }
}

void render() {
    // Clear render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Render world elements
    renderWorld();
    renderOreCounter(mapObjects[nPlayerID].nOreCount);

    // Render objects of all clients
    for (auto& object : mapObjects)
    {
        // Render players
        renderChar(0x01, { (int)object.second.vPos.x, (int)object.second.vPos.y, BLOCK_SIZE, BLOCK_SIZE }, object.second.nColor);
    }

    // Render image if shop is open
    if (shopOpen && shopImageTexture != NULL) {
        SDL_RenderCopy(renderer, shopImageTexture, NULL, NULL);
    }

    // Render scoreboard if shop isn't open
    if (!shopOpen && currentKeyStates[SDL_SCANCODE_TAB]) {
        renderScoreboard(mapObjects);
    }
    SDL_RenderPresent(renderer);
}
#pragma endregion

#pragma region Gameplay
void waitForConnection() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    std::string waitingText = "Waiting for connection...";
    SDL_Point textSize = getTextSize(waitingText);
    SDL_Rect rect = { (WINDOW_WIDTH - textSize.x) / 2, (WINDOW_HEIGHT - textSize.y) / 2, textSize.x, textSize.y };
    renderText("Waiting for connection...", rect);
    SDL_RenderPresent(renderer);
}

void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            close();
            exit(0);
        }
    }

    currentKeyStates = SDL_GetKeyboardState(NULL);
    playerRect = { (int)mapObjects[nPlayerID].vPos.x, (int)mapObjects[nPlayerID].vPos.y, BLOCK_SIZE, BLOCK_SIZE };
}

void shopLogic() {
    static bool keyPressed[6] = { false };
    if (currentKeyStates[SDL_SCANCODE_SPACE] && !spacePressed) {
        spacePressed = true;

        if (shopOpen) {
            // Close the shop if it's already open
            shopOpen = false;
            SDL_DestroyTexture(shopImageTexture);
            shopImageTexture = nullptr;
            Mix_PlayChannel(-1, shopCloseSound, 0);
        }
        else {
            // Check if the player is touching the shop
            if (AABB(playerRect, shopRect)) {
                shopOpen = true;
                shopImageTexture = loadTexture(shopImagePath);
                Mix_PlayChannel(-1, shopOpenSound, 0);
                if (shopImageTexture == nullptr) {
                    std::cerr << "Failed to load shop image!" << std::endl;
                    shopOpen = false;
                }
            }
        }
    }
    else if (!currentKeyStates[SDL_SCANCODE_SPACE]) {
        spacePressed = false;
    }

    if (shopOpen) {
        int randomSoundIndex = rand() % 3;
        Mix_Chunk* haggleSounds[3] = { haggleSound1, haggleSound2, haggleSound3 };

        for (int i = 1; i <= SHOP_SPEEDS.size(); ++i) {
            SDL_Scancode keyCode = static_cast<SDL_Scancode>(SDL_SCANCODE_1 + i - 1);
            if (currentKeyStates[keyCode]) {
                if (!keyPressed[i]) {
                    keyPressed[i] = true;
                    float speed = SHOP_SPEEDS.at(i);
                    int cost = SHOP_COSTS.at(i);
                    if (mapObjects[nPlayerID].fMiningSpeed < speed && mapObjects[nPlayerID].nOreCount >= cost) {
                        mapObjects[nPlayerID].fMiningSpeed = speed;
                        mapObjects[nPlayerID].nOreCount -= cost;
                        Mix_PlayChannel(-1, levelupSound, 0);
                    }
                    else {
                        Mix_PlayChannel(-1, haggleSounds[randomSoundIndex], 0);
                    }
                }
            }
            else {
                keyPressed[i] = false;
            }
        }
    }
}

void rockMining(float deltaTime) {
    static bool isMining = false;
    static float accumulatedTime = 0.0f;

    if (currentKeyStates[SDL_SCANCODE_SPACE] && AABB(playerRect, rockRect)) {
        accumulatedTime += deltaTime;

        if (!isMining) {
            // Start playing the mining sound
            Mix_PlayChannel(-1, miningSound, 0);
            isMining = true;
        }

        if (accumulatedTime >= (1.0f / mapObjects[nPlayerID].fMiningSpeed)) {
            mapObjects[nPlayerID].nOreCount++;
            accumulatedTime -= (1.0f / mapObjects[nPlayerID].fMiningSpeed);

            // Stop mining sound
            Mix_HaltChannel(-1);

            // Play a random ore obtained sound
            int randomSoundIndex = rand() % 4;
            Mix_Chunk* oreObtainedSounds[4] = { oreObtainedSound1, oreObtainedSound2, oreObtainedSound3, oreObtainedSound4 };
            Mix_PlayChannel(-1, oreObtainedSounds[randomSoundIndex], 0);

            // Reset mining sound
            Mix_PlayChannel(-1, miningSound, 0);
        }
    }
    else {
        if (isMining) {
            // Stop the mining sound when not mining
            Mix_HaltChannel(-1);
            isMining = false;
        }
        // Reset accumulated time if not mining
        accumulatedTime = 0.0f;
    }
}

void playerMovement() {
    mapObjects[nPlayerID].vVel = { 0.0f, 0.0f };

    if (currentKeyStates[SDL_SCANCODE_W] && !shopOpen) {
        mapObjects[nPlayerID].vVel += { 0.0f, -200.0f };
    }
    if (currentKeyStates[SDL_SCANCODE_S] && !shopOpen) {
        mapObjects[nPlayerID].vVel += { 0.0f, +200.0f };
    }
    if (currentKeyStates[SDL_SCANCODE_A] && !shopOpen) {
        mapObjects[nPlayerID].vVel += { -200.0f, 0.0f };
    }
    if (currentKeyStates[SDL_SCANCODE_D] && !shopOpen) {
        mapObjects[nPlayerID].vVel += { +200.0f, 0.0f };
    }

    if (mapObjects[nPlayerID].vVel.mag2() > 0)
        mapObjects[nPlayerID].vVel = mapObjects[nPlayerID].vVel.norm() * 200.0f;
}

void updateClientObjects(float deltaTime) {
    for (auto& object : mapObjects)
    {
        sVector2 vPotentialPosition = object.second.vPos + object.second.vVel * deltaTime;

        if (vPotentialPosition.x < BLOCK_SIZE) {
            vPotentialPosition.x = BLOCK_SIZE;
        }
        if (vPotentialPosition.x > WINDOW_WIDTH - BLOCK_SIZE - PLAYER_SIZE) {
            vPotentialPosition.x = WINDOW_WIDTH - BLOCK_SIZE - PLAYER_SIZE;
        }
        if (vPotentialPosition.y < BLOCK_SIZE) {
            vPotentialPosition.y = BLOCK_SIZE;
        }
        if (vPotentialPosition.y > WINDOW_HEIGHT - BLOCK_SIZE - PLAYER_SIZE) {
            vPotentialPosition.y = WINDOW_HEIGHT - BLOCK_SIZE - PLAYER_SIZE;
        }

        // Shop and rock collision detection
        SDL_Rect shopRect = { 70, 30, 100, 20 };
        SDL_Rect rockRect = { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2), (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE };

        if (!AABB({ static_cast<int>(vPotentialPosition.x), static_cast<int>(vPotentialPosition.y), playerRect.w, playerRect.h }, shopRect) &&
            !AABB({ static_cast<int>(vPotentialPosition.x), static_cast<int>(vPotentialPosition.y), playerRect.w, playerRect.h }, rockRect)) {
            object.second.vPos = vPotentialPosition;
        }
    }
}
#pragma endregion