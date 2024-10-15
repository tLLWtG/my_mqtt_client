#ifndef __UTIL_H
#define __UTIL_H

#include <Arduino.h>
#include <initializer_list>

String genClientID();

byte* encodeRemainingLength(int remaining_length);

int encodeStringWithLen(String str, byte* byte_array, int start_index);

template <typename T>
class myVector {
  private:
    T* data;
    size_t length;
    size_t capacity;

    void resize(size_t newCapacity) {
        if (newCapacity < length) return;
        T* newData = new T[newCapacity];
        for (size_t i = 0; i < length; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

  public:
    myVector() : data(nullptr), length(0), capacity(0) {
        resize(1);
    }

    myVector(std::initializer_list<T> initList) : data(nullptr), length(0), capacity(0) {
        resize(initList.size());
        for (const T& value : initList) {
            push_back(value);
        }
    }

    myVector(const myVector<T>& other) : data(nullptr), length(0), capacity(0) {
        resize(other.capacity);
        length = other.length;
        for (size_t i = 0; i < length; ++i) {
            data[i] = other.data[i];
        }
    }

    myVector& operator=(const myVector<T>& other) {
        if (this != &other) {
            delete[] data;
            resize(other.capacity);
            length = other.length;
            for (size_t i = 0; i < length; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    ~myVector() {
        delete[] data;
    }

    void push_back(const T& value) {
        if (length == capacity) {
            resize(capacity * 2);
        }
        data[length++] = value;
    }

    size_t size() const {
        return length;
    }

    size_t get_capacity() const {
        return capacity;
    }

    T& operator[](size_t index) {
        if (index >= length) {
            static T dummy;
            return dummy;
        }
        return data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= length) {
            static T dummy;
            return dummy;
        }
        return data[index];
    }

    void pop_back() {
        if (length > 0) {
            --length;
        }
    }

    void clear() {
        length = 0;
    }
};

#endif
