#pragma once

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.

template <class T, std::size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    CompressedPairElement() = default;
};

template <class T, std::size_t I>
struct CompressedPairElement<T, I, true> : T {
    CompressedPairElement() = default;

    template <typename U>
    CompressedPairElement(U&& value) : T(std::forward<decltype(value)>(value)) {
    }

    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};

template <class T, std::size_t I>
struct CompressedPairElement<T, I, false> {
    CompressedPairElement() : value_(){};

    template <typename U>
    CompressedPairElement(U&& value) : value_(std::forward<decltype(value)>(value)) {
    }

    T& Get() {
        return value_;
    }

    const T& Get() const {
        return value_;
    }

    T value_;
};

template <typename F, typename S>
class CompressedPair : CompressedPairElement<F, 0>, CompressedPairElement<S, 1> {
public:
    CompressedPair() = default;

    template <typename FT, typename ST>
    CompressedPair(FT&& first, ST&& second)
        : CompressedPairElement<F, 0>(std::forward<decltype(first)>(first)),
          CompressedPairElement<S, 1>(std::forward<decltype(second)>(second)) {
    }

    F& GetFirst() {
        return CompressedPairElement<F, 0>::Get();
    }

    const F& GetFirst() const {
        return CompressedPairElement<F, 0>::Get();
    }

    S& GetSecond() {
        return CompressedPairElement<S, 1>::Get();
    }

    const S& GetSecond() const {
        return CompressedPairElement<S, 1>::Get();
    }
};
