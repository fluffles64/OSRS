#pragma once
#include <unordered_map>
#include "common.h"

class Server : public tfg::net::server_interface<GameMsg>
{
public:
	Server(uint16_t nPort) : tfg::net::server_interface<GameMsg>(nPort)
	{
		InitializeColors();
	}

	std::unordered_map<uint32_t, sPlayerDescription> m_mapPlayerRoster;
	std::vector<uint32_t> m_vGarbageIDs;

private:
	std::vector<Color> m_vAvailableColors;
	std::unordered_map<uint32_t, Color> m_mapPlayerColors;

	void InitializeColors() {
		m_vAvailableColors = {
			{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0},
			{255, 0, 255}, {0, 255, 255}, {128, 0, 0}, {0, 128, 0},
			{0, 0, 128}, {128, 128, 0}, {128, 0, 128}, {0, 128, 128},
			{192, 192, 192}, {128, 128, 128}, {64, 64, 64}
		};
	}

	Color AssignColor() {
		if (m_vAvailableColors.empty()) {
			// Default to white if no colors are available
			return { 255, 255, 255 };
		}

		// Shuffle the available colors
		std::random_shuffle(m_vAvailableColors.begin(), m_vAvailableColors.end());

		// Take the last color from the shuffled list
		Color color = m_vAvailableColors.back();
		m_vAvailableColors.pop_back();

		return color;
	}

	void ReleaseColor(Color color) {
		m_vAvailableColors.push_back(color);
	}

protected:
	bool OnClientConnect(std::shared_ptr<tfg::net::connection<GameMsg>> client) override
	{
		// For now we will allow all 
		return true;
	}

	void OnClientValidated(std::shared_ptr<tfg::net::connection<GameMsg>> client) override
	{
		// Client passed validation check, so send them a message informing
		// them they can continue to communicate
		tfg::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Client_Accepted;
		client->Send(msg);
	}

	void OnClientDisconnect(std::shared_ptr<tfg::net::connection<GameMsg>> client) override
	{
		if (client)
		{
			if (m_mapPlayerRoster.find(client->GetID()) == m_mapPlayerRoster.end())
			{
				// client never added to roster, so just let it disappear
			}
			else
			{
				auto& pd = m_mapPlayerRoster[client->GetID()];
				std::cout << "[UNGRACEFUL REMOVAL]:" + std::to_string(pd.nUniqueID) + "\n";
				ReleaseColor(pd.nColor);
				m_mapPlayerRoster.erase(client->GetID());
				m_vGarbageIDs.push_back(client->GetID());
			}
		}

	}

	void OnMessage(std::shared_ptr<tfg::net::connection<GameMsg>> client, tfg::net::message<GameMsg>& msg) override
	{
		if (!m_vGarbageIDs.empty())
		{
			for (auto pid : m_vGarbageIDs)
			{
				tfg::net::message<GameMsg> m;
				m.header.id = GameMsg::Game_RemovePlayer;
				m << pid;
				std::cout << "Removing " << pid << "\n";
				MessageAllClients(m);
			}
			m_vGarbageIDs.clear();
		}

		switch (msg.header.id)
		{
			case GameMsg::Client_RegisterWithServer:
			{
				sPlayerDescription desc;
				msg >> desc;
				desc.nUniqueID = client->GetID();
				desc.nColor = AssignColor();
				m_mapPlayerRoster.insert_or_assign(desc.nUniqueID, desc);

				tfg::net::message<GameMsg> msgSendID;
				msgSendID.header.id = GameMsg::Client_AssignID;
				msgSendID << desc.nUniqueID;
				MessageClient(client, msgSendID);

				tfg::net::message<GameMsg> msgAddPlayer;
				msgAddPlayer.header.id = GameMsg::Game_AddPlayer;
				msgAddPlayer << desc;
				MessageAllClients(msgAddPlayer);

				for (const auto& player : m_mapPlayerRoster)
				{
					tfg::net::message<GameMsg> msgAddOtherPlayers;
					msgAddOtherPlayers.header.id = GameMsg::Game_AddPlayer;
					msgAddOtherPlayers << player.second;
					MessageClient(client, msgAddOtherPlayers);
				}
				break;
			}

			case GameMsg::Client_UnregisterWithServer:
			{
				break;
			}

			case GameMsg::Game_UpdatePlayer:
			{
				// Simply bounce update to everyone except incoming client
				MessageAllClients(msg, client);
				break;
			}
		}
	}
};

int main()
{
	Server server(60000);
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}
	return 0;
}