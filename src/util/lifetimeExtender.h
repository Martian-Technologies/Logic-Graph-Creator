#ifndef lifetimeExtender_h
#define lifetimeExtender_h

class LifetimeExtender {
public:
	template <typename T>
	inline void push(std::shared_ptr<T> lifetime) {
		lifetimes.push_back(std::static_pointer_cast<void>(lifetime));
	}
	inline void flush() {
		lifetimes.clear();
	}

private:
	std::vector<std::shared_ptr<void>> lifetimes;
};

#endif
