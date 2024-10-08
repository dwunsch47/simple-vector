#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t cap) : capacity_to_reserve_(cap) {}
    size_t capacity_to_reserve_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(SimpleVector&& other) : items_(other.items_.Release()),
        size_(std::exchange(other.size_, 0)),
        capacity_(std::exchange(other.capacity_, 0)) {}

    SimpleVector& operator=(SimpleVector&& other) {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }

    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {
        std::generate(begin(), end(), [] { return Type(); });
    }

    SimpleVector(ReserveProxyObj other) {
        Reserve(other.capacity_to_reserve_);
    }

    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        if (size == 0) {
            return;
        }
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) : items_(other.size_), size_(other.size_), capacity_(other.capacity_) {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return (size_ == 0 ? true : false);
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range of vector");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range of vector");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            Reserve(std::max(new_size, (capacity_ * 2)));
        }
        if ((size_ < new_size) && (new_size < capacity_) || new_size > capacity_) {
            std::generate(end(), begin() + new_size, [] {return Type{}; });
        }
        size_ = new_size;
    }

    void PushBack(const Type& item) {
        *(PreInsert(end())) = item;
    }

    void PushBack(Type&& item) {
        *(PreInsert(end())) = std::move(item);
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& item) {
        Iterator new_pos = PreInsert(pos);
        *new_pos = item;
        return new_pos;
    }

    Iterator Insert(ConstIterator pos, Type&& item) {
        Iterator new_pos = PreInsert(pos);
        *new_pos = std::move(item);
        return new_pos;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        std::move(Iterator(pos) + 1, end(), Iterator(pos));
        --size_;
        return Iterator(pos);
    }


    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void swap(SimpleVector&& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            items_.swap(tmp);
            capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    Iterator PreInsert(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        Iterator result = (Iterator)pos;
        if (pos == end()) {
            if (size_ == capacity_) {
                Reserve(std::max(static_cast<size_t>(1), (capacity_ * 2)));
            }
            result = end();
        }
        else if (size_ < capacity_) {
            std::move_backward((Iterator)pos, end(), end() + 1);
        }
        else {
            result = std::move_backward((Iterator)pos, end(), end() + 1);
            --result;
        }
        ++size_;
        return result;
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template<typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template<typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template<typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template<typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
