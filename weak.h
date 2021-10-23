#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : ptr_(nullptr), block_(nullptr){}

    WeakPtr(const WeakPtr& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddWeak();
        }
    }
    WeakPtr(WeakPtr&& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddWeak();
        }
        other.Reset();
    }

    template <class U>
    WeakPtr(const WeakPtr<U>& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddWeak();
        }
    }
    template <class U>
    WeakPtr(WeakPtr<U>&& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddWeak();
        }
        other.Reset();
    }

          // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : ptr_(other.Get()), block_(other.GetControlBlock()) {
        if (block_) {
            block_->AddWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        Reset(other.Get(), other.GetControlBlock());
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }
        Reset(other.Get(), other.GetControlBlock());
        other.Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ == nullptr) {
            return;
        }
        if (block_->RemoveWeak()) {
            delete block_;
        }
        block_ = nullptr;
        ptr_ = nullptr;
    }

    void Reset(T* ptr, ControlBlockBase* block) {
        if (block_ == block) {
            return;
        }
        Reset();
        ptr_ = ptr;
        block_ = block;
        if (block_) {
            block_->AddWeak();
        }
    }

    void Swap(WeakPtr& other) {
        WeakPtr tmp = *this;
        *this = other;
        other = tmp;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->GetStrong();
        }
        return 0;
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        return Expired() ? SharedPtr<T>() : SharedPtr(*this);
    }

    T* Get() const {
        return ptr_;
    }

    ControlBlockBase* GetControlBlock() const {
        return block_;
    }
private:
    ControlBlockBase* block_;
    T* ptr_;
};
