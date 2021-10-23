#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <memory>
#include <algorithm>

template <typename T, typename D = std::default_delete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, D()){}

    template<class D_>
    UniquePtr(T* ptr, D_&& deleter) : ptr_(ptr, std::forward<D_>(deleter)){}

    template <class U, class E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : ptr_(other.Release(), std::forward<E>(other.GetDeleter())){}

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (other.Get() == ptr_.GetFirst()) {
            return *this;
        }
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<D>(other.GetDeleter());
        return *this;
    }

    template <class U, class E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        if (other.Get() == ptr_.GetFirst()) {
            return *this;
        }
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<E>(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* tmp_ptr = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return tmp_ptr;
    }
    void Reset(T* ptr = nullptr) noexcept {
        T* tmp_ptr = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (tmp_ptr) {
            ptr_.GetSecond()(tmp_ptr);
        }
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }
    D& GetDeleter() {
        return ptr_.GetSecond();
    }
    const D& GetDeleter() const {
        return ptr_.GetSecond();
    }
    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference<T>::type
    operator*() const {
        return *(ptr_.GetFirst());
    }

    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, D> ptr_;
};

template <typename T, typename D>
class UniquePtr<T[], D> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, D()){}

    template<class D_>
    UniquePtr(T* ptr, D_&& deleter) : ptr_(CompressedPair<T*, D_>(ptr, std::forward<D_>(deleter))){}

    template <class U, class E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept {
        ptr_ = CompressedPair<U*, E>(other.Release(), std::forward<E>(other.GetDeleter()));
    }

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <class U, class E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        if (other.Get() == Get()) {
            return *this;
        }
        Reset(other.Release());
        ptr_.GetSecond() = std::forward<E>(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* tmp_ptr = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return tmp_ptr;
    }
    void Reset(T* ptr = nullptr) noexcept {
        T* tmp_ptr = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (tmp_ptr) {
            ptr_.GetSecond()(tmp_ptr);
        }
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    typename std::add_lvalue_reference<T>::type
    operator[](size_t i) const {
        return ptr_.GetFirst()[i];
    }

    T* Get() const {
        return ptr_.GetFirst();
    }
    D& GetDeleter() {
        return ptr_.GetSecond();
    }
    const D& GetDeleter() const {
        return ptr_.GetSecond();
    }
    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *(ptr_.GetFirst());
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, D> ptr_;
};
