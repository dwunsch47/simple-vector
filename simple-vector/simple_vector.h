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
    
    SimpleVector(SimpleVector&& other) : items_(other.items_.Release()) {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }
    
    SimpleVector& operator=(SimpleVector&& other) {
            items_.swap(other.items_);
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {
        std::generate(items_.Get(), items_.Get() + size_, [] { return Type(); });
    }
    
    SimpleVector(ReserveProxyObj other) {
        Reserve(other.capacity_to_reserve_);
    }
 
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size) {
        if (size == 0) {
            return;
        }
        std::fill(items_.Get(), items_.Get() + size, value);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
        size_ = init.size();
        capacity_ = init.size();
    }
    
    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        swap(tmp);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            ArrayPtr<Type> tmp(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), tmp.Get());
            items_.swap(tmp);
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0 ? true : false);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range of vector");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range of vector");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            std::generate(end(), begin() + new_size, []{return Type{};});
            size_ = new_size;
        } else if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, (capacity_ * 2));
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            std::generate(tmp.Get() + size_ + 1, tmp.Get() + new_capacity, []{ return Type{};});
            items_.swap(tmp);
            capacity_ = new_capacity;
            size_ = new_size;
        }
    }
    
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            ArrayPtr<Type> tmp(size_ + 1);
            std::copy(items_.Get(), items_.Get() + size_, tmp.Get());
            tmp[size_] = item;
            items_.swap(tmp);
            capacity_ = ++size_ * 2;
        } else {
            items_[size_] = item;
            ++size_;
        }
    }
    
    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            ArrayPtr<Type> tmp(size_ + 1);
            std::move(items_.Get(), items_.Get() + size_, tmp.Get());
            tmp[size_] = std::move(item);
            items_.swap(tmp);
            capacity_ = ++size_ * 2;
        } else {
            items_[size_] = std::move(item);
            ++size_;
        }
    }
    
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }
    
    Iterator Insert(ConstIterator pos, const Type& item) {
        assert(pos >= begin() && pos <= end());
        if (pos == end()){
            PushBack(item);
			return end() - 1;
        } else if (size_ == capacity_) {
            SimpleVector<Type> tmp(size_ + 1);
            auto it = std::copy(begin(), Iterator(pos), tmp.begin());
            *it = item;
            std::copy(Iterator(pos), end(), it + 1);
            swap(tmp);
            capacity_ = (size_ - 1) * 2;
            return ++it;
        } else {
            std::copy_backward(Iterator(pos), end(), end() + 1);
            *(Iterator(pos)) = item;
            ++size_;
            return Iterator(pos);
        }
    }
    
    Iterator Insert(ConstIterator pos, Type&& item) {
        assert(pos >= begin() && pos <= end());
        if (pos == end()){
            PushBack(std::move(item));
			return end() - 1;
        } else if (size_ == capacity_) {
            SimpleVector<Type> tmp(size_ + 1);
            auto it = std::move(begin(), Iterator(pos), tmp.begin());
            *it = std::move(item);
            std::move(Iterator(pos), end(), it + 1);
            swap(tmp);
            capacity_ = (size_ - 1) * 2;
            return ++it;
        } else {
            std::move_backward(Iterator(pos), end(), end() + 1);
            *(Iterator(pos)) = std::move(item);
            ++size_;
            return Iterator(pos);
        }
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
            SimpleVector<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.begin());
            items_.swap(tmp.items_);
            capacity_ = new_capacity;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
    
private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template<typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) && (lhs.GetSize() == rhs.GetSize());
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
