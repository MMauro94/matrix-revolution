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
		std::deque<std::pair<std::string, std::function<void()>>> queue;
		std::mutex queue_mutex;
		std::condition_variable condition;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

		void loop(const std::string &threadName) {
			while (true) {
				std::function<void()> job;
				std::string jobName;
				{
					std::cout << threadName + " is waiting...\n";
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					condition.wait(lock, [=] { return !queue.empty(); });
					std::pair<std::string, std::function<void()>> &pair = queue.back();
					jobName = pair.first;
					job = pair.second;
					queue.pop_back();
				}
				std::cout << threadName + " is executing job " + jobName + "...\n";
				job();
			}
		}

#pragma clang diagnostic pop

	public:
		explicit ThreadPool(unsigned threadCount) : threadCount(threadCount) {
		}

		unsigned getThreadCount() const {
			return this->threadCount;
		}

		void add(std::string jobName, const std::function<void()> &runnable) {
			{
				std::unique_lock<std::mutex> lock(this->queue_mutex);
				this->queue.push_front(std::make_pair(jobName, runnable));
			}
			condition.notify_one();
		}

		ThreadPool *start() {
			if (!this->threads.empty()) {
				Utils::error("Pool already started!");
			}
			for (unsigned i = 0; i < this->threadCount; i++) {
				const unsigned threadIndex = i;
				this->threads.emplace_back([=] { loop("Thread " + std::to_string(threadIndex)); });
			}

			return this;
		}
};

#endif //MATRIX_THREADPOOL_H
