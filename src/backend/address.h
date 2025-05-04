#ifndef address_h
#define address_h

#include "position/position.h"

class Address {
public:
	Address() { }
	Address(Position position) : addresses({ position }) {}
	Address(std::vector<Position> positions) : addresses(positions) { }
	// check for equality
	inline bool operator==(const Address& other) const {
		if (addresses.size() != other.addresses.size()) return false;
		for (size_t i = 0; i < addresses.size(); i++) {
			if (addresses[i] != other.addresses[i]) return false;
		}
		return true;
	}
	inline int size() const { return addresses.size(); }
	inline Position getPosition(int index) const { return addresses[index]; }

	inline void addBlockId(Position position) { addresses.push_back(position); }

	inline void nestPosition(Position position) {
		addresses.insert(addresses.begin(), position);
	}
	inline std::vector<Position> getPositions() const {
		return addresses;
	}
	inline Address chopLastPosition() const {
		if (addresses.size() == 0) return Address();
		std::vector<Position> newAddresses = addresses;
		newAddresses.pop_back();
		return Address(newAddresses);
	}

private:
	std::vector<Position> addresses;

};

template<>
struct std::hash<Address> {
	size_t operator()(const Address& address) const {
		size_t hash = 0;
		for (const auto& pos : address.getPositions()) {
			hash ^= std::hash<Position>()(pos) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}
		return hash;
	}
};

#endif /* address_h */
