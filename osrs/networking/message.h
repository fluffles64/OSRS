#pragma once
#include "common.h"

namespace tfg
{
	namespace net
	{
		// Message Header is sent at start of all messages. The template allows us
		// to use "enum class" to ensure that the messages are valid at compile time
		// We are using a uin32_t for the size instead of size(T) becuase if the server is 64 bytes
		// and the client 32 or vice versa it can cause problems. (we cant guarantee that
		// size T is the same on a 32 or 64 byte computer).
		//Furthermore uint32_t should be more than sufficient.
		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message
		{
			message_header<T> header{};
			std::vector<uint8_t> body;

			// Returns size of entire message packet in bytes
			size_t size() const {
				return sizeof(message_header<T>) + body.size();
			}

			// Override for std:cout compatibility - produces friendly description of message
			friend std::ostream& operator << (std::ostream& os, const message<T>& msg) {
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

			// Pushes any POD-like data into the message buffer
			template<typename DataType>
			friend message<T>& operator << (message<T>& msg, const DataType& data) {
				// Check that the type of data being pushed is copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be copied");

				// Cache current size of vector, as this will be the point we insert the data
				size_t i = msg.body.size();

				// Resize the vector by the size of the data being pushed
				// This can introduce a bit of overhead, as each time we add something
				// to the body vector it needs to be resized. However, it shouldn't grow linearily
				// and in practice, it will have minimum overhead.
				msg.body.resize(msg.body.size() + sizeof(DataType));

				// Physically copy the data into the newly allocated vector space
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

				// Recalculate the message size
				msg.header.size = msg.size();

				// Return the target message so it can be chained
				return msg;
			}

			template<typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data) {
				// Check that the type of the data being pushed is copyable
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed");

				// Cache the location towards the end of the vector where the pulled data starts
				size_t i = msg.body.size() - sizeof(DataType);

				//Physically copy the data from the vector into the user variable
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

				// Shrink the vector to remove read bytes, and reset end position
				// Unlike with resize, shrinking doesnt introduce overhead.
				msg.body.resize(i);

				// Recalculate the message size
				msg.header.size = msg.size();

				// Return the target message so it can be chained
				return msg;
			}
		};

		// An "owned" message is identical to a regular message, but it is associated with
		// a connection. On a server, the owner would be the client that sent the message, 
		// on a client the owner would be the server.

		// Forward declare the connection
		template <typename T>
		class connection;

		template <typename T>
		struct owned_message
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> msg;

			// Again, a friendly string maker
			friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};
	}
}