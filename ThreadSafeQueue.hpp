#pragma once
#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue{
public:
	void push(const T& value){
		std::lock_guard<std::mutex> lock(mtx);
		q.push(value);
		cv.notify_one();
	}
	void pop(){
		std::lock_guard<std::mutex> lock(mtx);
		q.pop();
	}

	void wait_and_pop(T& out){
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [this]{return !q.empty();});
		out = std::move(q.front());
		q.pop();
	}

	size_t size(){
		std::lock_guard<std::mutex> lock(mtx);
		return q.size();
	}
	bool empty(){
		std::lock_guard<std::mutex> lock(mtx);
		return q.empty();
	}

	T front(){
		std::lock_guard<std::mutex> lock(mtx);
		return q.front();
	}

private:
	std::queue<T> q;
	std::condition_variable cv;
	mutable std::mutex mtx;
};
