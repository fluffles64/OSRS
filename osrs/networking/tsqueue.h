#pragma once
#include "common.h"

namespace tfg
{
	namespace net
	{
		// We are going to implement the thread safe queue using locks. There are lock-free
		// ts queues out there, but this way it is simpler.

		template<typename T>
		class tsqueue
		{
		public:
			tsqueue() = default;
			tsqueue(const tsqueue<T>&) = delete;
			virtual ~tsqueue() { clear(); }

		public:
			// Returns and maintains item at front of queue
			const T& front()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.front();
			}

			// Returns and maintains item at back of queue
			const T& back()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.back();
			}

			// Adds an item to back of queue
			void push_back(const T& item)
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				deqQueue.emplace_back(std::move(item));

				// We've added an item to the queue, signal the condition variable to wake up
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			// Adds an item to back of queue
			void push_front(const T& item)
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				deqQueue.emplace_front(std::move(item));

				// We've added an item to the queue, signal the condition variable to wake up
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			// Returns true if queue has no items
			bool empty()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.empty();
			}

			// Returns number of items in queue
			size_t count()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				return deqQueue.size();
			}

			// Clears queue
			void clear()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				deqQueue.clear();
			}

			// Removes and returns item at front of queue
			T pop_front()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				auto t = std::move(deqQueue.front());
				deqQueue.pop_front();
				return t;
			}

			// Removes and returns item from back of queue
			T pop_back()
			{
				std::lock_guard<std::mutex> lock(muxQueue);
				auto t = std::move(deqQueue.back());
				deqQueue.pop_back();
				return t;
			}

			// Prevent idling at a 100% CPU by sleeping the threads until an item has been written to the queue.
			void wait()
			{
				// If the queue is empty, lock the calling process
				// Only two things can signal our condition variable to wake up:
				// 1) Our calls at push_back and push_front with notify_one, which are the only way our queue object has to insert items into the queue
				// 2) A phenomena called spurious wake up, notorious on Windows.
				//	this phenomena makes it so that the condition var will sometimes erroneously wake up
				// but the way this is set uo it will go back around to the while loop, check the state of
				// the empty flag and continue, at which point it will go back to sleep
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
