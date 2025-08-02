#ifndef evalConfig_h
#define evalConfig_h

#include <atomic>
#include <functional>
#include <vector>
#include <mutex>

class EvalConfig {
public:
	EvalConfig() = default;

	inline long long int getTargetTickrate() const {
		return targetTickrate.load();
	}

	inline void setTargetTickrate(long long int tickrate) {
		targetTickrate.store(tickrate);
		notifySubscribers();
	}

	inline bool isTickrateLimiterEnabled() const {
		return tickrateLimiter.load();
	}

	inline void setTickrateLimiter(bool enabled) {
		tickrateLimiter.store(enabled);
		notifySubscribers();
	}

	inline bool isRunning() const {
		return running.load();
	}

	inline void setRunning(bool run) {
		running.store(run);
		notifySubscribers();
	}

	inline bool isRealistic() const {
		return realistic.load();
	}

	inline void setRealistic(bool value) {
		realistic.store(value);
		notifySubscribers();
	}
	inline void subscribe(std::function<void()> callback) {
		std::lock_guard<std::mutex> lock(subscribersMutex);
		subscribers.push_back(callback);
	}

	inline void unsubscribe() {
		std::lock_guard<std::mutex> lock(subscribersMutex);
		subscribers.clear();
	}

private:
	std::atomic<long long int> targetTickrate = 0; // in ticks per minute
	std::atomic<bool> tickrateLimiter = true;
	std::atomic<bool> running = false;
	std::atomic<bool> realistic = false;

	std::vector<std::function<void()>> subscribers;
	std::mutex subscribersMutex;

	void notifySubscribers() {
		std::lock_guard<std::mutex> lock(subscribersMutex);
		for (const auto& callback : subscribers) {
			callback();
		}
	}
};

#endif // evalConfig_h