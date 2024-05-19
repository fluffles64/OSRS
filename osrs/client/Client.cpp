#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int PLAYER_SIZE = 20;
const int BLOCK_SIZE = 20;
const int PLAYER_SPEED = 2;

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

    const std::string fontPath = "../../media/Ac437_IBM_VGA_9x8-2x.ttf";
    font = TTF_OpenFont(fontPath.c_str(), 24);
    if (font == nullptr) {
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

    // Render top border
    for (int x = 0; x < WINDOW_WIDTH; x += BLOCK_SIZE) {
        SDL_Color color = { 255, 255, 255 };
        char blockChar = 8;
        SDL_Surface* textSurface = TTF_RenderGlyph_Blended(font, blockChar, color);
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
        SDL_Rect textRect = { x, 0, BLOCK_SIZE, BLOCK_SIZE };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Clean up resources
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Render bottom border
    for (int x = 0; x < WINDOW_WIDTH; x += BLOCK_SIZE) {
        SDL_Color color = { 255, 255, 255 };
        char blockChar = 8;
        SDL_Surface* textSurface = TTF_RenderGlyph_Blended(font, blockChar, color);
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
        SDL_Rect textRect = { x, WINDOW_HEIGHT - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Clean up resources
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Render left border
    for (int y = 0; y < WINDOW_HEIGHT; y += BLOCK_SIZE) {
        SDL_Color color = { 255, 255, 255 };
        char blockChar = 8;
        SDL_Surface* textSurface = TTF_RenderGlyph_Blended(font, blockChar, color);
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
        SDL_Rect textRect = { 0, y, BLOCK_SIZE, BLOCK_SIZE };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Clean up resources
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Render right border
    for (int y = 0; y < WINDOW_HEIGHT; y += BLOCK_SIZE) {
        SDL_Color color = { 255, 255, 255 };
        char blockChar = 8;
        SDL_Surface* textSurface = TTF_RenderGlyph_Blended(font, blockChar, color);
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
        SDL_Rect textRect = { WINDOW_WIDTH - BLOCK_SIZE, y, BLOCK_SIZE, BLOCK_SIZE };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Clean up resources
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Render player as an ASCII character
    SDL_Color color = { 255, 255, 255 };
    char playerChar = 1;
    SDL_Surface* textSurface = TTF_RenderGlyph_Blended(font, playerChar, color);
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
    SDL_Rect textRect = { playerRect.x, playerRect.y, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    // Clean up resources
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
        // Add a short delay to control the speed
        SDL_Delay(10);
    }

    close();
    return 0;
}