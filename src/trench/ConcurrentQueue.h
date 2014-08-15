/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <derevenetc@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <trench/config.h>

#include <condition_variable>
#include <mutex>
#include <queue>

namespace trench {

template<class T, class Queue = std::queue<T>>
class ConcurrentQueue {
	Queue queue_;
	std::mutex mutex_;
	std::condition_variable condition_;

public:
        void push(T element) {
                std::lock_guard<std::mutex> lock(mutex_);

                queue_.push(std::move(element));
		condition_.notify_one();
        }

	T pop() {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this] { return !queue_.empty(); });

                auto result = std::move(queue_.front());
                queue_.pop();

                return result;
	}
};

} // namespace trench
