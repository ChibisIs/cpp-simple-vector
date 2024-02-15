#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility> 


#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity)
        :capacity_(capacity)
    {

    }
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type{})
    {

    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size), size_(size), capacity_(size)
    {
        std::fill(items_.Get(), items_.Get() + size, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other)
        :items_(other.GetSize()), size_(other.GetSize()), capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector(SimpleVector&& other)
        :items_(other.GetSize())
    {
        swap(other);
    }

    explicit SimpleVector(ReserveProxyObj reserve_items)
    {
        Reserve(reserve_items.GetCapacity());
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
        return size_ == 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&items_ != &rhs.items_) {
            SimpleVector<Type> items_copy(rhs);
            swap(items_copy);
        }
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }

        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
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
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);
            Fill(new_items.Get(), new_items.Get() + new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            size_ = new_size;
            capacity_ = new_capacity;

        }
        else if (new_size <= capacity_ && new_size > size_) {
            Fill(end(), end() + new_size);
            size_ = new_size;
        }
        else if (new_size <= capacity_ && new_size < size_) {
            size_ = new_size;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            size_t new_size = size_ + 1;
            Resize(new_size);
            items_[size_ - 1] = item;
        }
        else {
            items_[size_] = item;
            ++size_;
        }
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            size_t new_size = size_ + 1;
            Resize(new_size);
            items_[size_ - 1] = std::move(item);
        }
        else {
            items_[size_] = std::move(item);
            ++size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos <= end() && pos >= begin());

        size_t index = pos - items_.Get();
        if (size_ == capacity_) {
            size_t new_size = size_ + 1;
            Resize(new_size);
            std::copy_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[index] = value;
        }
        else {
            std::copy_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[index] = value;
            ++size_;
        }
        return &items_[index];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos <= end() && pos >= begin());

        size_t index = pos - items_.Get();
        if (size_ == capacity_) {
            size_t new_size = size_ + 1;
            Resize(new_size);
            std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[index] = std::move(value);
        }
        else {
            std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[index] = std::move(value);
            ++size_;
        }
        return &items_[index];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());

        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos < end() && pos >= begin());

        size_t index = pos - items_.Get();
        ArrayPtr<Type> new_items(size_);
        std::move(items_.Get(), items_.Get() + index, new_items.Get());
        std::move(items_.Get() + index + 1, items_.Get() + size_, new_items.Get() + index);
        items_.swap(new_items);
        --size_;
        return &items_[index];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        items_.swap(other.items_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(items_.Get(), items_.Get() + size_, new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void Fill(Iterator first, Iterator last) {
        for (auto it = first; it != last; ++it) {
            *it = std::move(Type());
        }
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());;
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}