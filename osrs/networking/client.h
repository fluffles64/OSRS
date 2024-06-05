#pragma once
#include "common.h"
#include "message.h"
#include "tsqueue.h"
#include "connection.h"

/// <summary>
/// Copyright 2018 - 2021 OneLoneCoder.com
/// The client class is responsible for connecting to a server in the framework.
/// It implements methods for asynchronous I/O operations and message exchange with the server.
/// </summary>

namespace tfg
{
	namespace net
	{
		template <typename T>
		class client_interface
		{
		public:
			client_interface() : m_socket(m_context)
			{
				// Initialise the socket with the io context, so it has work to do
			}

			virtual ~client_interface()
			{
				// If the client is destroyed, always try and disconnect from the server
				Disconnect();
			}

		public:
			// Connect to server with hostname/ip and port
			bool Connect(const std::string& host, const uint16_t port)
			{
				try
				{
					// Resolve hostname/ip-address into tangiable physical address
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

					// Create connection
					m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);

					// Tell the connection object to connect to server
					m_connection->ConnectToServer(endpoints);

					// Start Context Thread
					thrContext = std::thread([this]() { m_context.run(); });
				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}

				return true;
			}

			// Disconnect from server
			void Disconnect()
			{
				if (IsConnected())
				{
					m_connection->Disconnect();
				}

				// Stop the ASIO context and its thread
				m_context.stop();
				if (thrContext.joinable())
					thrContext.join();

				// Destroy the connection object
				m_connection.release();
			}

			// Check if client is actually connected to a server
			bool IsConnected()
			{
				if (m_connection)
					return m_connection->IsConnected();
				else
					return false;
			}

			void Send(const message<T>& msg)
			{
				if (IsConnected())
					m_connection->Send(msg);
			}

			// Retrieve queue of messages from the server
			tsqueue<owned_message<T>>& Incoming()
			{
				return m_qMessagesIn;
			}

		protected:
			// ASIO context
			asio::io_context m_context;

			// Give the context a thread of its own to execute its work commands
			std::thread thrContext;

			// Hardware socket that is connected to the server
			asio::ip::tcp::socket m_socket;

			// The client has a single instance of a connection object, which handles data transfer
			std::unique_ptr<connection<T>> m_connection;

		private:
			// This is the thread safe queue of incoming messages from the server
			tsqueue<owned_message<T>> m_qMessagesIn;
		};
	}
}