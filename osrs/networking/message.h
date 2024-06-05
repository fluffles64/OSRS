#pragma once
#include "common.h"

/// <summary>
/// Copyright 2018 - 2021 OneLoneCoder.com
/// The messages have two components: Header and body.
/// The header includes an id and the size of the entire message (including the header).
/// The body is essentially the payload of the message. It can also be non-existent (0 bytes).
/// Header is always sent first, as it has a fixed size.
/// The id of the header uses an enum class to validate the accuracy of the headers at compile time.
/// Similarly, templates (enum classes) are used to make the framework fit any kind of game, instead
/// of creating hundreds of message types.
/// The size uses uint32_t instead of size(T) to prevent issues with servers being 64bits and
/// clients being 32 bits or vice versa.
/// </summary>

namespace tfg
{
	namespace net
	{
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

			// Returns size of the entire message (header + body)
			size_t size() const
			{
				return sizeof(message_header<T>) + body.size();
			}

			// Override that shows message id and size with std::cout support
			friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
			{
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

			 /// These two overloaded operators push/pop Plain Old Data (POD) into/out of the
			 /// message buffer, respectively.
			 /// They first check if the data being pushed/extracted is of a copyable type.
			 /// Then they cache the size of the vector. The >> operator does it at the end of the vector where
			 /// the pulled data starts. Then they resize/shrink the vector. Resizing it (<<) can introduce overhead.
			 /// The << operator copies the data into the newly allocated vector space, while the >> operator copies the
			 /// data from the vector into the user variable. After modifying the message buffer, they both
			 /// recalculate the message size.
			
			template<typename DataType>
			friend message<T>& operator << (message<T>& msg, const DataType& data)
			{			
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be copied");
				size_t i = msg.body.size();
				msg.body.resize(msg.body.size() + sizeof(DataType));
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
				msg.header.size = msg.size();
				return msg;
			}

			template<typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed");
				size_t i = msg.body.size() - sizeof(DataType);
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
				msg.body.resize(i);
				msg.header.size = msg.size();
				return msg;
			}
		};

		///	In order to know where the messages come from, we will create owned messages. These messages 
		/// are identical to regular message, but they are always associated with
		///	a connection. On a server, the owner would be the client that sent the message,
		///	on a client the owner would be the server.

		template <typename T>
		class connection;

		template <typename T>
		struct owned_message
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> msg;

			// Overload the << operator to enable printing owned_message<T> objects
			friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};
	}
}