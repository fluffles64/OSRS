#include "game.h"

class OSRS : public tfg::net::client_interface<GameMsg>
{
public:
	bool OnUserCreate()
	{
		if (!init()) {
			return false;
		}
		if (Connect("127.0.0.1", 60000)) {
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

		// Wait for connection text until connection is successful
		if (bWaitingForConnection) {
			waitForConnection();
			return true;
		}

		// Handle events
		handleEvents();

		// Shop logic
		shopLogic();

		// Rock mining
		rockMining(deltaTime);

		// Player movement
		playerMovement();

		// Update all client's objects locally
		updateClientObjects(deltaTime);

		// Rendering
		render();

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

	if (!demo.OnUserCreate()) {
		std::cerr << "Either failed to initialize or client couldn't connect to server" << std::endl;
		return 1;
	}

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