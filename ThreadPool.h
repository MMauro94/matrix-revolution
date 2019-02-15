//
// Created by MMarco on 12/02/2019.
//

#ifndef MATRIX_THREADPOOL_H
#define MATRIX_THREADPOOL_H

#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <functional>
#include <deque>
#include <condition_variable>
#include "Utils.h"

class ThreadPool {
	private:
		unsigned threadCount;
		std::vector<std::thread> threads;
		std::deque<std::function<void()>> queue;
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop = false;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

		void loop() {
			while (!this->stop) {
				std::function<void()> job;
				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					condition.wait(lock, [=] { return !queue.empty(); });
					job = queue.back();
					queue.pop_back();
				}
				job();
			}
		}

#pragma clang diagnostic pop

	public:
		explicit ThreadPool(unsigned threadCount) : threadCount(threadCount) {
		}

		virtual ~ThreadPool() {
			this->stop = true;
			for (auto &t : this->threads) {
				t.join();
			}
		}

		unsigned getThreadCount() const {
			return this->threadCount;
		}

		void add(const std::function<void()> &runnable) {
			{
				std::unique_lock<std::mutex> lock(this->queue_mutex);
				this->queue.push_front(runnable);
			}
			condition.notify_one();
		}

		ThreadPool *start() {
			if (!this->threads.empty()) {
				Utils::error("Pool already started!");
			}
			for (unsigned i = 0; i < this->threadCount; i++) {
				const unsigned threadIndex = i;
				this->threads.emplace_back([=] { loop(); });
			}

			return this;
		}
};

#endif //MATRIX_THREADPOOL_H
