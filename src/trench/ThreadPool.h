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

#include "ConcurrentQueue.h"

#include <functional>
#include <iostream>
#include <thread>

namespace trench {

template<class Job = std::function<void()>, class Queue = ConcurrentQueue<std::function<bool()>>>
class ThreadPool {
	Queue queue_;
	std::vector<std::thread> threads_;
public:
	ThreadPool(): ThreadPool(std::thread::hardware_concurrency()) {}

	explicit
	ThreadPool(std::size_t nthreads) {
		threads_.reserve(nthreads);

		for (std::size_t i = 0; i < nthreads; ++i) {
			threads_.push_back(std::thread([this] { while (queue_.pop()()) {} }));
		}
	}

	~ThreadPool() {
		for (std::size_t i = 0; i < threads_.size(); ++i) {
			queue_.push([] { return false; });
		}
		for (auto &thread : threads_) {
			thread.join();
		}
	}

	ThreadPool(ThreadPool &&) = delete;
	ThreadPool &operator=(ThreadPool &&) = delete;

	void schedule(Job job) {
		assert(!threads_.empty());
		queue_.push([fun = std::move(job)] {
			fun();
			return true;
		});
	}
};

} // namespace trench
