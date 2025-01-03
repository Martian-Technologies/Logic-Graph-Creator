#ifndef sparse2d_h
#define sparse2d_h

#include <unordered_map>

#include "position.h"

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
	inline unsigned int size() const {return data.size();}
    
    inline void insert(const Position& position, const T& value);
    inline void remove(const Position& position);

private:
    std::unordered_map<Position, T> data;

    typedef typename std::unordered_map<Position, T>::iterator iterator;
    typedef typename std::unordered_map<Position, T>::const_iterator const_iterator;
};

template<class T>
T* Sparse2dArray<T>::get(const Position& position) {
    iterator iter = data.find(position);
    if (iter == data.end()) {
        return nullptr;
    } else {
        return &iter->second;
    }
}

template<class T>
const T* Sparse2dArray<T>::get(const Position& position) const {
    const_iterator iter = data.find(position);
    if (iter == data.end()) {
        return nullptr;
    } else {
        return &iter->second;
    }
}

template<class T>
void Sparse2dArray<T>::insert(const Position& position, const T& value) {
    iterator iter = data.find(position);
    if (iter == data.end()) {
        data.insert(std::make_pair(position, value));
    } else {
        iter->second = value;
    }
}

template<class T>
void Sparse2dArray<T>::remove(const Position& position) {
    iterator iter = data.find(position);
    if (iter != data.end())
        data.erase(iter);
}

#endif /* sparse2d_h */
