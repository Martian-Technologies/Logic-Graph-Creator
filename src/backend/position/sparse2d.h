#ifndef sparse2d_h
#define sparse2d_h

#include "position.h"
#include "../../external/hash_table5.hpp"

/*
Should be defined as:
template <class T>
class Sparse2d {
public:
	Sparse2d();
	Sparse2d(const Sparse2d& other);
	Sparse2d<T>& operator=(const Sparse2d<T>& other)
	T* get(const Position& position);
	const T* get(const Position& position) const;
	void insert(const Position& position, const T& value);
	void remove(const Position& position)
}
*/

template <class T> class Sparse2dArray;

template <class T>
using Sparse2d = Sparse2dArray<T>;


template <class T>
class Sparse2dArray {
public:
	inline T* get(const Position& position);
	inline const T* get(const Position& position) const;
	inline unsigned int size() const { return data.size(); }

	inline void insert(const Position& position, const T& value);
	inline void remove(const Position& position);

private:
    emhash5::HashMap<int, T> data;
    inline int cantorPair(const Position& pos) const { return pos.y + ((pos.x + pos.y) * (pos.x + pos.y + 1) / 2); }
};

template<class T>
T* Sparse2dArray<T>::get(const Position& position) {
    int key = cantorPair(position);
    if (data.contains(key)) {
        return &data.at(key);
    } else return nullptr;
}

template<class T>
const T* Sparse2dArray<T>::get(const Position& position) const {
    int key = cantorPair(position);
    if (data.contains(key)) {
        return &data.at(key);
    } else return nullptr;
}

template<class T>
void Sparse2dArray<T>::insert(const Position& position, const T& value) {
    int key = cantorPair(position);
    data.emplace_unique(key, value);
}

template<class T>
void Sparse2dArray<T>::remove(const Position& position) {
    int key = cantorPair(position);
    data.erase(key);
}

#endif /* sparse2d_h */
