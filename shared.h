#pragma once

#include "sw_fwd.h"  // Forward declaration
#include <memory>
#include <cstddef>  // std::nullptr_t

class ControlBlockBase {
private:
    std::size_t strong_ = 1;
    std::size_t weak_ = 0;
public:
    void AddStrong() {
        ++strong_;
    }
    void AddWeak() {
        ++weak_;
    }
    bool RemoveStrong() {
        --strong_;
        if (strong_ == 0) {
            OnZeroStrong();
        }
        return strong_ + weak_ == 0;
    }
    bool RemoveWeak() {
        --weak_;
        return strong_ + weak_ == 0;
    }

    std::size_t GetStrong() {
        return strong_;
    }
    std::size_t GetWeak() {
        return weak_;
    }

    virtual void OnZeroStrong() {}
    virtual ~ControlBlockBase() = default;
};

template <class T>
class ControlBlockPointer : public ControlBlockBase {
private:
    T* ptr_ = nullptr;

public:
    ControlBlockPointer(T* ptr) : ControlBlockBase(), ptr_(ptr){}

    void OnZeroStrong() override {
        std::default_delete<T>()(ptr_);
        ptr_ = nullptr;
    }

    T* Get() {
        return ptr_;
    }
    const T* Get() const {
        return ptr_;
    }
};

template <class T>
class ControlBlockEmplace : public ControlBlockBase {
private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
public:
    template <class ...Args>
    ControlBlockEmplace(Args&&... args) {
        new (&storage_) T(std::forward<Args>(args)...);
    }

    T* Get() {
        return reinterpret_cast<T*>(&storage_);
    }
    const T* Get() const {
        return reinterpret_cast<T*>(&storage_);
    }

    void OnZeroStrong() override {
        Get()->~T();
    }
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : ptr_(nullptr), block_(nullptr){}

    SharedPtr(std::nullptr_t) : ptr_(nullptr), block_(nullptr){}

    explicit SharedPtr(T* ptr) : ptr_(ptr), block_(new ControlBlockPointer<T>(ptr)){}

    explicit SharedPtr(T* ptr, ControlBlockBase* block) : ptr_(ptr), block_(block) {}

    SharedPtr(const SharedPtr& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddStrong();
        }
    }

    SharedPtr(SharedPtr&& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddStrong();
        }
        other.Reset();
    }

    template<class U>
    explicit SharedPtr(U* ptr) : ptr_(ptr), block_(new ControlBlockPointer<U>(ptr)){}

    template<class U>
    explicit SharedPtr(U* ptr, ControlBlockBase* block) : ptr_(ptr), block_(block) {}

    template<class U>
    SharedPtr(const SharedPtr<U>& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddStrong();
        }
    }

    template <class U>
    SharedPtr(SharedPtr<U>&& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddStrong();
        }
        other.Reset();
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template<typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        block_ = other.GetControlBlock();
        if (block_) {
            block_->AddStrong();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        ptr_ = other.Get();
        block_ = other.GetControlBlock();
        if (block_) {
            block_->AddStrong();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        Reset(other.Get(), other.GetControlBlock());
        if (block_) {
            block_->AddStrong();
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        Reset(other.ptr_, other.block_);
        if (block_) {
            block_->AddStrong();
        }
        other.Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ == nullptr) {
            return;
        }
        if (block_->RemoveStrong()) {
            delete block_;
        }
        block_ = nullptr;
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr == ptr_) {
            return;
        }
        Reset(ptr, new ControlBlockPointer(ptr));
    }
    template <class U>
    void Reset(U* ptr) {
        if (ptr == ptr_) {
            return;
        }
        Reset(ptr, new ControlBlockPointer<U>(ptr));
    }
    void Reset(T* ptr, ControlBlockBase* block) {
        if (block_ == block) {
            return;
        }
        Reset();
        ptr_ = ptr;
        block_ = block;
    }
    void Swap(SharedPtr& other) {
        SharedPtr tmp = *this;
        *this = other;
        other = tmp;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    ControlBlockBase* GetControlBlock() const {
        return block_;
    }

    typename std::add_lvalue_reference<T>::type
    operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_) {
            return block_->GetStrong();
        }
        return 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    ControlBlockBase* block_;
    T* ptr_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockEmplace<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block->Get(), block);
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
