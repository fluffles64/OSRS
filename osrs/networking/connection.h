#pragma once
#include "common.h"
#include "tsqueue.h"
#include "message.h"

/// <summary>
/// Copyright 2018 - 2021 OneLoneCoder.com
/// The connection class is responsible for managing network connections.
/// It defines an owner enum to identify the connection owner (server or client).
/// It then handles asynchronous I/O operations and manages message exchange with clients.
/// </summary>

namespace tfg
{
	namespace net
	{
		template<typename T>
		class server_interface;

		template<typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{
		public:
			// Connections are owned by either a server or a client
			enum class owner
			{
				server,
				client
			};

			// The constructor specifies the owner, connects to a context, transfers the socket, and provides a reference to the incoming message queue.
			connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn) : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
			{
				m_nOwnerType = parent;

				// Construct validation check data
				if (m_nOwnerType == owner::server)
				{
					// Connection is server -> client, construct random data for the client to transform and send back for validation
					m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

					// Pre-calculate the result for checking when the client responds
					m_nHandshakeCheck = scramble(m_nHandshakeOut);
				}
				else
				{
					// Connection is client -> server, so we have nothing to define
					m_nHandshakeIn = 0;
					m_nHandshakeOut = 0;
				}
			}

			virtual ~connection()
			{

			}

			// Provides clients a way to know about other clients
			uint32_t GetID() const
			{
				return id;
			}

		public:
			void ConnectToClient(tfg::net::server_interface<T>* server, uint32_t uid = 0)
			{
				if (m_nOwnerType == owner::server)
				{
					if (m_socket.is_open())
					{
						id = uid;

						// A client has attempted to connect to the server, but we wish the client to first validate itself
						WriteValidation();

						// Issue a task to sit and wait asynchronously for the validation data to be sent back from the client
						ReadValidation(server);
					}
				}
			}

			void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
			{
				// Only clients can connect to servers
				if (m_nOwnerType == owner::client)
				{
					// Request ASIO attempts to connect to an endpoint
					asio::async_connect(m_socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
						{
							if (!ec)
							{
								// First thing server will do now is send a packet to be validated so wait for that and respond
								ReadValidation();
							}
						});
				}
			}

			void Disconnect()
			{
				if (IsConnected())
					asio::post(m_asioContext, [this]() { m_socket.close(); });
			}

			bool IsConnected() const
			{
				return m_socket.is_open();
			}

		public:
			// Send a message, connections are one-to-one so no need to specifiy the target
			void Send(const message<T>& msg)
			{
				asio::post(m_asioContext,
					[this, msg]()
					{
						/// If the queue has a message in it, then we must 
						/// assume that it is in the process of asynchronously being written.
						/// Either way add the message to the queue to be output. If no messages
						/// were available to be written, then start the process of writing the
						/// message at the front of the queue.

						bool bWritingMessage = !m_qMessagesOut.empty();
						m_qMessagesOut.push_back(msg);
						if (!bWritingMessage)
						{
							WriteHeader();
						}
					});
			}

		private:
			// Prime context to read a message header
			void ReadHeader()
			{
				asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// We need to take into account that the message has the header and the body size
							if (m_msgTemporaryIn.header.size > 8)
							{
								m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size - 8);
								ReadBody();
							}
							else
							{
								AddToIncomingMessageQueue();
							}
						}
						else
						{
							std::cout << "[" << id << "] Read Header Fail.\n";
							m_socket.close();
						}
					});
			}

			// Prime context ready to read a message body
			void ReadBody()
			{
				// If this method was called, a header has already been read, and that header requests we read a body.
				// The space for that body has already been allocated in the temporary message object, so just wait for the bytes to arrive.
				asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// Add the whole message to incoming queue
							AddToIncomingMessageQueue();
						}
						else
						{
							std::cout << "[" << id << "] Read Body Fail.\n";
							m_socket.close();
						}
					});
			}

			// Prime context to write a message header
			void WriteHeader()
			{
				// If this function is called, we know the outgoing message queue must have at least one message to send.
				// Allocate a transmission buffer to hold the message, and issue ASIO to send those bytes
				asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// No error, so check if the message header just sent also has a message body
							if (m_qMessagesOut.front().body.size() > 0)
							{
								// It does, so issue the task to write the body bytes
								WriteBody();
							}
							else
							{
								// It didnt, so we are done with this message. Remove it from the outgoing message queue
								m_qMessagesOut.pop_front();

								// If the queue is not empty, there are more messages to send
								if (!m_qMessagesOut.empty())
								{
									WriteHeader();
								}
							}
						}
						else
						{
							// ASIO failed to write the message, so close the socket
							std::cout << "[" << id << "] Write Header Fail.\n";
							m_socket.close();
						}
					});
			}

			// Prime context to write a message body
			void WriteBody()
			{
				// If this method was called, a header has just been sent, and that header indicated a body existed for this message.
				// Fill a transmission buffer with the body data, and send it
				asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// Sending was successful, so we are done with the message
							m_qMessagesOut.pop_front();

							// If the queue still has messages in it, then issue the task to send the next message's header
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
						else
						{
							// Sending failed, see WriteHeader() equivalent for description :P
							std::cout << "[" << id << "] Write Body Fail.\n";
							m_socket.close();
						}
					});
			}

			// Once a full message is received, add it to the incoming queue
			void AddToIncomingMessageQueue()
			{
				// Shove it in the queue, converting it to an "owned message", by initialising it with a shared pointer from this connection object
				if (m_nOwnerType == owner::server)
					m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
				else
					m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

				// Prime ASIO context to receive the next message
				ReadHeader();
			}

			// "Encrypt" data to validate clients with a handshake
			// TODO: This isn't very secure at all, needs a revision
			// One of those constants could have the version number so we can also stop outdated client versions from talking to newer servers
			uint64_t scramble(uint64_t nInput)
			{
				uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
				out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
				return out ^ 0xC0DEFACE12345678;
			}

			// Used by both client and server to write validation packet
			void WriteValidation()
			{
				asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// Validation data sent, clients should sit and wait for a response/closure
							if (m_nOwnerType == owner::client)
								ReadHeader();
						}
						else
						{
							m_socket.close();
						}
					});
			}

			void ReadValidation(tfg::net::server_interface<T>* server = nullptr)
			{
				asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
					[this, server](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_nOwnerType == owner::server)
							{
								// Compare sent data to actual solution
								if (m_nHandshakeIn == m_nHandshakeCheck)
								{
									// Client has provided valid solution, so allow it to connect properly
									std::cout << "Client Validated" << std::endl;
									server->OnClientValidated(this->shared_from_this());

									// Sit waiting to receive data
									ReadHeader();
								}
								else
								{
									// Client gave incorrect data, so disconnect
									std::cout << "Client Disconnected (Fail Validation)" << std::endl;
									m_socket.close();
								}
							}
							else
							{
								// Connection is a client, so solve puzzle
								m_nHandshakeOut = scramble(m_nHandshakeIn);

								// Write the result
								WriteValidation();
							}
						}
						else
						{
							std::cout << "Client Disconnected (ReadValidation)" << std::endl;
							m_socket.close();
						}
					});
			}

		protected:
			// Each connection has a unique socket to a remote
			asio::ip::tcp::socket m_socket;

			// This context is shared with the whole asio instance, we only want a single context for the server
			asio::io_context& m_asioContext;

			// This queue holds all messages to be sent to the remote side of this connection
			tsqueue<message<T>> m_qMessagesOut;

			// This queue holds all messages that have been received from the remote side of this connection
			tsqueue<owned_message<T>>& m_qMessagesIn;

			// Incoming messages are constructed asynchronously, so we will store the part assembled message here, until it is ready
			message<T> m_msgTemporaryIn;

			// The owner decides how some of the connection behaves
			owner m_nOwnerType = owner::server;
			uint32_t id = 0;

			// Handshake validation
			uint64_t m_nHandshakeOut = 0;
			uint64_t m_nHandshakeIn = 0;
			uint64_t m_nHandshakeCheck = 0;
		};
	}
}