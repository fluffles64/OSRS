#pragma once
#include "common.h"
#include "tsqueue.h"
#include "message.h"
#include "connection.h"

/// <summary>
/// Copyright 2018 - 2021 OneLoneCoder.com
/// The server defines a template class server_interface for creating server instances.
/// It implements methods for managing client connections, sending messages,
/// and handling incoming message packets using a thread-safe queue.
/// </summary>

namespace tfg
{
	namespace net
	{
		template<typename T>
		class server_interface
		{
		public:
			server_interface(uint16_t port) : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

			}

			virtual ~server_interface()
			{
				Stop();
			}

			// Start the server
			bool Start()
			{
				try
				{
					/// <summary>
					/// Issue a task to the asio context - This is important
					/// as it will prime the context with "work", and stop it
					/// from exiting immediately. Since this is a server, we 
					/// want it primed ready to handle clients trying to
					/// connect. The order of these two lines is relevant
					/// because if we launch the context before giving
					/// it some work it could close in some cases.
					/// </summary>

					WaitForClientConnection();

					// Launch the asio context in its own thread
					m_threadContext = std::thread([this]() { m_asioContext.run(); });
				}
				catch (std::exception& e)
				{
					// Something prohibited the server from listening
					std::cerr << "[SERVER] Exception: " << e.what() << "\n";
					return false;
				}

				// Careful with too many debug messages, it can slog down the server
				std::cout << "[SERVER] Started!\n";
				return true;
			}

			// Stop the server
			void Stop()
			{
				// Request the context to close
				m_asioContext.stop();

				// Tidy up the context thread
				if (m_threadContext.joinable()) m_threadContext.join();

				std::cout << "[SERVER] Stopped!\n";
			}

			// ASYNC - Instruct asio to wait for connection
			void WaitForClientConnection()
			{
				// Prime context with an instruction to wait until a socket connects. This is the purpose
				// of an "acceptor" object. It will provide a unique socket for each incoming connection attempt
				m_asioAcceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						// Triggered by incoming connection request
						if (!ec)
						{
							std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

							// Temporarily create a new connection to handle this client 
							std::shared_ptr<connection<T>> newconn =
								std::make_shared<connection<T>>(connection<T>::owner::server,
									m_asioContext, std::move(socket), m_qMessagesIn);

							// Give the server a chance to deny connection
							if (OnClientConnect(newconn))
							{
								// Connection allowed, so add to container of new connections
								m_deqConnections.push_back(std::move(newconn));

								// Issue a task to the connection's ASIO context to sit and wait for bytes to arrive
								m_deqConnections.back()->ConnectToClient(this, nIDCounter++);
								std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";
							}
							else
							{
								std::cout << "[-----] Connection Denied\n";

								// Connection will go out of scope with no pending tasks, so will
								// get destroyed automatically due to the use of smart pointers
							}
						}
						else
						{
							std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
						}

						// Prime the asio context with more work - again simply wait for another connection
						WaitForClientConnection();
					});
			}

			// Send a message to a specific client
			void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{
				// Check if client is legitimate...
				if (client && client->IsConnected())
				{
					client->Send(msg);
				}
				else
				{
					// If we cant communicate with client then we may as well remove the client
					OnClientDisconnect(client);
					client.reset();

					// Then physically remove it from the container
					m_deqConnections.erase(
						std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
				}
			}

			// Send a message to all clients
			void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
			{
				bool bInvalidClientExists = false;

				// Iterate through all clients in container
				for (auto& client : m_deqConnections)
				{
					// Check if client is connected
					if (client && client->IsConnected())
					{
						if (client != pIgnoreClient)
							client->Send(msg);
					}
					else
					{
						// The client couldn't be contacted, so assume it has disconnected
						OnClientDisconnect(client);
						client.reset();

						// Set this flag to remove dead clients from container
						bInvalidClientExists = true;
					}
				}

				// Remove dead clients
				if (bInvalidClientExists)
					m_deqConnections.erase(
						std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
			}

			// Force server to respond to incoming messages
			void Update(size_t nMaxMessages = -1, bool bWait = false)
			{
				// Wait until the client sends a message so that the server doesnt use 100% of the CPU core
				if (bWait) m_qMessagesIn.wait();

				// Process as many messages as it can up to the specified value
				size_t nMessageCount = 0;
				while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
				{
					// Grab the front message
					auto msg = m_qMessagesIn.pop_front();

					// Pass to message handler
					OnMessage(msg.remote, msg.msg);
					nMessageCount++;
				}
			}

		protected:
			// The server class should override these functions to implement custom functionalities
			virtual bool OnClientConnect(std::shared_ptr<connection<T>> client) { return false; }
			virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client) {}
			virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg) {}

		public:
			virtual void OnClientValidated(std::shared_ptr<connection<T>> client) {}

		protected:
			// Thread safe queue for incoming message packets
			tsqueue<owned_message<T>> m_qMessagesIn;

			// Container of active validated connections
			std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

			// Order of declaration is important. It is also the order of initialisation
			asio::io_context m_asioContext;
			std::thread m_threadContext;

			// Handles new incoming connection attempts
			asio::ip::tcp::acceptor m_asioAcceptor;

			// Identifier of the clients
			uint32_t nIDCounter = 10000;
		};
	}
}