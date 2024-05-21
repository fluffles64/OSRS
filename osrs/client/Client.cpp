#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include "game.h"
#include "../server/common.h"

class OSRS : public tfg::net::client_interface<GameMsg>
{
private:
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

public:
	bool OnUserCreate()
	{
		// Seed the random number generator
		srand(static_cast<unsigned int>(time(nullptr)));
		// TODO: init() here
		// Load sound effects
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

		Mix_VolumeChunk(miningSound, MIX_MAX_VOLUME / 1.5f); // 75% volume
		Mix_VolumeChunk(oreObtainedSound1, MIX_MAX_VOLUME / 5); // 20% volume
		Mix_VolumeChunk(oreObtainedSound2, MIX_MAX_VOLUME / 5); // 20% volume
		Mix_VolumeChunk(oreObtainedSound3, MIX_MAX_VOLUME / 5); // 20% volume
		Mix_VolumeChunk(oreObtainedSound4, MIX_MAX_VOLUME / 5); // 20% volume
		Mix_VolumeChunk(levelupSound, MIX_MAX_VOLUME / 10); // 20% volume

		if (Connect("127.0.0.1", 60000))
		{
			return true;
		}

		return false;
	}

	bool OnUserUpdate(float deltaTime)
	{
		// Check for incoming network messages
		if (IsConnected()) {
			while (!Incoming().empty()) {
				auto msg = Incoming().pop_front().msg;

				switch (msg.header.id)
				{
					case(GameMsg::Client_Accepted):
					{
						std::cout << "Server accepted client - Welcome!\n";
						tfg::net::message<GameMsg> msg;
						msg.header.id = GameMsg::Client_RegisterWithServer;
						// TODO: Might want to also reset ore and mining speed here
						descPlayer.vPos = { 60.0f, 200.0f };
						descPlayer.fMiningSpeed = 1.0f;
						descPlayer.nOreCount = 0;
						msg << descPlayer;
						Send(msg);
						break;
					}

					case(GameMsg::Client_AssignID):
					{
						// Server is assigning us OUR id
						msg >> nPlayerID;
						std::cout << "Assigned Client ID = " << nPlayerID << "\n";
						break;
					}

					case(GameMsg::Game_AddPlayer):
					{
						sPlayerDescription desc;
						msg >> desc;
						mapObjects.insert_or_assign(desc.nUniqueID, desc);

						if (desc.nUniqueID == nPlayerID)
						{
							// Now we exist in game world
							bWaitingForConnection = false;
						}
						break;
					}

					case(GameMsg::Game_RemovePlayer):
					{
						uint32_t nRemovalID = 0;
						msg >> nRemovalID;
						mapObjects.erase(nRemovalID);
						break;
					}

					case(GameMsg::Game_UpdatePlayer):
					{
						sPlayerDescription desc;
						msg >> desc;
						mapObjects.insert_or_assign(desc.nUniqueID, desc);
						break;
					}
				}
			}
		}

		// Waiting for connection text until connection is successful
		if (bWaitingForConnection)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			SDL_Color textColor = { 255, 255, 255, 255 };

			SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Waiting for connection...", textColor);
			if (textSurface == nullptr) {
				return false;
			}

			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			if (textTexture == nullptr) {
				SDL_FreeSurface(textSurface);
				return false;
			}

			int textWidth = textSurface->w;
			int textHeight = textSurface->h;

			SDL_FreeSurface(textSurface);

			SDL_Rect textRect;
			textRect.x = (WINDOW_WIDTH - textWidth) / 2;
			textRect.y = (WINDOW_HEIGHT - textHeight) / 2;
			textRect.w = textWidth;
			textRect.h = textHeight;

			SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
			SDL_DestroyTexture(textTexture);
			SDL_RenderPresent(renderer);

			return true;
		}

		// Control of Player Object
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				close();
				exit(0);
			}
		}
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		SDL_Rect playerRect = { (int)mapObjects[nPlayerID].vPos.x, (int)mapObjects[nPlayerID].vPos.y, BLOCK_SIZE, BLOCK_SIZE };

		// Shop logic
		if (currentKeyStates[SDL_SCANCODE_SPACE] && !spacePressed) {
			spacePressed = true;

			if (shopOpen) {
				// Close the shop if it's already open
				shopOpen = false;
				SDL_DestroyTexture(shopImageTexture);
				shopImageTexture = NULL;
			}
			else {
				// Check if the player is touching the shop
				SDL_Rect shopRect = { 70 - 5, 30 - 5, 100 + 10, 20 + 10 };

				if (AABB(playerRect, shopRect)) {
					shopOpen = true;
					shopImageTexture = loadTexture("../../../media/shop.png");
					if (shopImageTexture == NULL) {
						std::cerr << "Failed to load shop image!" << std::endl;
						shopOpen = false;
					}
				}
			}
		}
		else if (!currentKeyStates[SDL_SCANCODE_SPACE]) {
			spacePressed = false;
		}

		// Adjust mining speed when the shop is open
		if (shopOpen) {
			if (currentKeyStates[SDL_SCANCODE_1]) {
				if (mapObjects[nPlayerID].fMiningSpeed < 10.0f && mapObjects[nPlayerID].nOreCount >= 15) {
					mapObjects[nPlayerID].fMiningSpeed = 10.0f;
					mapObjects[nPlayerID].nOreCount = 0;
					Mix_PlayChannel(-1, levelupSound, 0);
				}
			}
			else if (currentKeyStates[SDL_SCANCODE_2]) {
				if (mapObjects[nPlayerID].fMiningSpeed < 31.4159f && mapObjects[nPlayerID].nOreCount >= 500) {
					mapObjects[nPlayerID].fMiningSpeed = 31.4159f;
					mapObjects[nPlayerID].nOreCount = 0;
					Mix_PlayChannel(-1, levelupSound, 0);
				}
			}
			else if (currentKeyStates[SDL_SCANCODE_3]) {
				if (mapObjects[nPlayerID].fMiningSpeed < 100.0f && mapObjects[nPlayerID].nOreCount >= 2000) {
					mapObjects[nPlayerID].fMiningSpeed = 100.0f;
					mapObjects[nPlayerID].nOreCount = 0;
					Mix_PlayChannel(-1, levelupSound, 0);
				}
			}
			else if (currentKeyStates[SDL_SCANCODE_4]) {
				if (mapObjects[nPlayerID].fMiningSpeed < 1024.0f && mapObjects[nPlayerID].nOreCount >= 4090) {
					mapObjects[nPlayerID].fMiningSpeed = 1024.0f;
					mapObjects[nPlayerID].nOreCount = 0;
					Mix_PlayChannel(-1, levelupSound, 0);
				}
			}
			else if (currentKeyStates[SDL_SCANCODE_5]) {
				if (mapObjects[nPlayerID].fMiningSpeed < 10000.0f && mapObjects[nPlayerID].nOreCount >= 100000) {
					mapObjects[nPlayerID].fMiningSpeed = 10000.0f;
					mapObjects[nPlayerID].nOreCount = 0;
					Mix_PlayChannel(-1, levelupSound, 0);
				}
			}
		}

		// Rock mining
		SDL_Rect rockRect = { (WINDOW_WIDTH / 2) - (BLOCK_SIZE / 2) - 5, (WINDOW_HEIGHT / 2) - (BLOCK_SIZE / 2) - 5, BLOCK_SIZE + 10, BLOCK_SIZE + 10 };
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

		// Player movement
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

		// Update objects locally
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

			// TODO: may need to add player rect to player description
			//playerRect.x = static_cast<int>(vPotentialPosition.x);
			//playerRect.y = static_cast<int>(vPotentialPosition.y);
		}

		// Clear render
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		// Render world and ore count
		renderWorld();
		renderOreCounter(mapObjects[nPlayerID].nOreCount);

		// Render objects
		for (auto& object : mapObjects)
		{
			// TODO: do the rect thing from before here too
			//renderPlayer();
			// WARNING: careful with the int cast, check it later
			renderText(0x01, { (int)object.second.vPos.x, (int)object.second.vPos.y, BLOCK_SIZE, BLOCK_SIZE }, object.second.nColor);
		}

		// Render image
		if (shopOpen && shopImageTexture != NULL) {
			SDL_RenderCopy(renderer, shopImageTexture, NULL, NULL);
		}
		SDL_RenderPresent(renderer);

		// Send player description
		tfg::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_UpdatePlayer;
		msg << mapObjects[nPlayerID];
		Send(msg);
		return true;
	}
};

int main(int argc, char* args[]) {
	OSRS demo;
	if (!init()) {
		std::cerr << "Failed to initialize!" << std::endl;
		return 1;
	}
	demo.OnUserCreate();

	bool quit = false;
	Uint32 lastTime = SDL_GetTicks();
	int frameCount = 0;

	const int FPS = 144;
	const int frameDelay = 1000 / FPS;

	while (!quit) {
		Uint32 frameStart = SDL_GetTicks();

		Uint32 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		demo.OnUserUpdate(deltaTime);

		Uint32 frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime) {
			SDL_Delay(frameDelay - frameTime);
		}

		// Calculate FPS
		frameCount++;
		if (frameCount % 100 == 0) {
			float averageFPS = 1000.0f / (SDL_GetTicks() - lastTime);
			std::string windowTitle = "O.S.R.S | FPS: " + std::to_string(static_cast<int>(averageFPS));
			SDL_SetWindowTitle(window, windowTitle.c_str());
		}
	}
	close();
	return 0;
}
