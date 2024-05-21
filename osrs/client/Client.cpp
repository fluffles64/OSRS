#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
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

public:
	bool OnUserCreate()
	{
		// TODO: init() here

		if (Connect("127.0.0.1", 60000))
		{
			return true;
		}

		return false;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		// Check for incoming network messages
		if (IsConnected()) {
			while (!Incoming().empty()) {
				auto msg = Incoming().pop_front().msg;

				switch (msg.header.id)
				{
					case(GameMsg::Client_Accepted):
					{
						std::cout << "Server accepted client - you're in!\n";
						tfg::net::message<GameMsg> msg;
						msg.header.id = GameMsg::Client_RegisterWithServer;
						// TODO: Might want to also reset ore and mining speed here
						descPlayer.vPos = { 60.0f, 200.0f };
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

		//TODO: add shop and mining functionalities back
		//if (shopOpen) {
		//	if (currentKeyStates[SDL_SCANCODE_SPACE] && !spacePressed) {
		//		spacePressed = true;
		//		shopOpen = false;
		//		SDL_DestroyTexture(shopImageTexture);
		//		shopImageTexture = NULL;
		//	}
		//	else if (!currentKeyStates[SDL_SCANCODE_SPACE]) {
		//		spacePressed = false;
		//	}

		//	if (currentKeyStates[SDL_SCANCODE_1]) {
		//		if (mapObjects[nPlayerID].fMiningSpeed < 10.0f && mapObjects[nPlayerID].nOreCount >= 15)
		//			mapObjects[nPlayerID].fMiningSpeed = 10.0f;
		//	}
		//	else if (currentKeyStates[SDL_SCANCODE_2]) {
		//		if (mapObjects[nPlayerID].fMiningSpeed < 31.4159f && mapObjects[nPlayerID].nOreCount >= 500)
		//			mapObjects[nPlayerID].fMiningSpeed = 31.4159f;
		//	}
		//	else if (currentKeyStates[SDL_SCANCODE_3]) {
		//		if (mapObjects[nPlayerID].fMiningSpeed < 100.0f && mapObjects[nPlayerID].nOreCount >= 2000)
		//			mapObjects[nPlayerID].fMiningSpeed = 100.0f;
		//	}
		//	else if (currentKeyStates[SDL_SCANCODE_4]) {
		//		if (mapObjects[nPlayerID].fMiningSpeed < 1024.0f && mapObjects[nPlayerID].nOreCount >= 4090)
		//			mapObjects[nPlayerID].fMiningSpeed = 1024.0f;
		//	}
		//	else if (currentKeyStates[SDL_SCANCODE_5]) {
		//		if (mapObjects[nPlayerID].fMiningSpeed < 10000.0f && oreCount >= 100000)
		//			mapObjects[nPlayerID].fMiningSpeed = 10000.0f;
		//	}
		//	return;
		//}

		mapObjects[nPlayerID].vVel = { 0.0f, 0.0f };

		if (currentKeyStates[SDL_SCANCODE_W]) {
			mapObjects[nPlayerID].vVel += { 0.0f, -200.0f };
		}
		if (currentKeyStates[SDL_SCANCODE_S]) {
			mapObjects[nPlayerID].vVel += { 0.0f, +200.0f };
		}
		if (currentKeyStates[SDL_SCANCODE_A]) {
			mapObjects[nPlayerID].vVel += { -200.0f, 0.0f };
		}
		if (currentKeyStates[SDL_SCANCODE_D]) {
			mapObjects[nPlayerID].vVel += { +200.0f, 0.0f };
		}

		if (mapObjects[nPlayerID].vVel.mag2() > 0)
			mapObjects[nPlayerID].vVel = mapObjects[nPlayerID].vVel.norm() * 200.0f;

		// Update objects locally
		for (auto& object : mapObjects)
		{
			sVector2 vPotentialPosition = object.second.vPos + object.second.vVel * fElapsedTime;

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

		// Render world
		renderWorld();

		// Render objects
		for (auto& object : mapObjects)
		{
			// TODO: do the rect thing from before here too
			//renderPlayer();
			// WARNING: careful with the int cast, check it later
			renderText(0x01, { (int)object.second.vPos.x, (int)object.second.vPos.y, BLOCK_SIZE, BLOCK_SIZE });
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

	while (!quit) {
		Uint32 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;
		demo.OnUserUpdate(deltaTime);
	}
	close();
	return 0;
}