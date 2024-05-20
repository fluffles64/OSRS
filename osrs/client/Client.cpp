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

		mapObjects[0].nUniqueID = 0;
		mapObjects[0].vPos = { 60.0f, 200.0f };

		mapObjects[3453450].nUniqueID = 1234;
		mapObjects[3453450].vPos = { 70.0f, 200.0f };

		//if (Connect("127.0.0.1", 60000))
		//{
		//	return true;
		//}

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		// Check for incoming network messages


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
		mapObjects[nPlayerID].vVel = { 0.0f, 0.0f };

		if (currentKeyStates[SDL_SCANCODE_W]) {
			mapObjects[nPlayerID].vVel += { 0.0f, -1.0f };
		}
		if (currentKeyStates[SDL_SCANCODE_S]) {
			mapObjects[nPlayerID].vVel += { 0.0f, +1.0f };
		}
		if (currentKeyStates[SDL_SCANCODE_A]) {
			mapObjects[nPlayerID].vVel += { -1.0f, 0.0f };
		}
		if (currentKeyStates[SDL_SCANCODE_D]) {
			mapObjects[nPlayerID].vVel += { +1.0f, 0.0f };
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