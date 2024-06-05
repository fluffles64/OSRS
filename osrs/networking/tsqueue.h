#pragma once
#include "common.h"

/// <summary>
/// Copyright 2018 - 2021 OneLoneCoder.com
/// Implements a thread-safe queue using locks for inter-thread communication.
/// Provides methods for pushing, popping, and accessing items in the queue.
/// Includes a condition variable to avoid busy-waiting when the queue is empty.
/// </summary>

namespace tfg
{
	namespace net
	{
		template<typename T>
		class tsqueue
		{
		public:
			tsqueue() = default;
			tsqueue(const tsqueue<T>&) = delete;
			virtual ~tsqueue() { clear(); }

		public:
			// Return and maintain item at front of queue
			const T& front()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.front();
			}

			// Return and maintain item at back of queue
			const T& back()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.back();
			}

			// Add an item to the back of queue
			void push_back(const T& item)
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				deqQueue.emplace_back(std::move(item));

				// Notify the condition variable
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			// Add an item to back of queue
			void push_front(const T& item)
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				deqQueue.emplace_front(std::move(item));

				// Notify the condition variable
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			// Return true if queue is empty
			bool empty()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.empty();
			}

			// Return number of items in queue
			size_t count()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.size();
			}

			// Clear queue
			void clear()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				deqQueue.clear();
			}

			// Remove and return item at front of queue
			T pop_front()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				auto t = std::move(deqQueue.front());
				deqQueue.pop_front();
				return t;
			}

			// Remove and return item at back of queue
			T pop_back()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				auto t = std::move(deqQueue.back());
				deqQueue.pop_back();
				return t;
			}

			/// Prevents the threads from idling at 100% CPU usage by putting them to sleep until an item is written to the queue.
			/// If the queue is empty, locks the calling process. Wake-up signals include calls to push_back and push_front,
			/// the only methods to insert items into the queue, along with notify_one, and spurious wake-up,
			/// a phenomenon particularly on Windows, where the condition variable wakes up erroneously.
			/// However, this is handled by looping back to check the state of the empty flag before going back to sleep.

			void wait()
			{
				while (empty())
				{
					std::unique_lock<std::mutex> ul(muxBlocking);
					cvBlocking.wait(ul);
				}
			}

		protected:
			std::mutex muxQueue;
			std::deque<T> deqQueue;
			std::condition_variable cvBlocking;
			std::mutex muxBlocking;
		};
	}
}