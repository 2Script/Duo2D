#pragma once
#include <concepts>
#include <cstddef>
#include <ostream>
#include <type_traits>
#include <variant>

namespace d2d {
    template<typename T> requires std::is_standard_layout_v<T>
    struct attribute  {
        constexpr attribute() noexcept = default;
        template<typename U> requires (std::is_constructible_v<T, U> && !std::is_same_v<T*, std::remove_cvref_t<U>>)
        constexpr attribute(U&& value) noexcept : local_value(std::forward<U>(value)), holds_ref(false) {}
        constexpr attribute(T* const& value) noexcept : ref(value), holds_ref(true) {}

    public:
        template<typename U>
        constexpr attribute& operator=(U&& value) noexcept {
            get_ref() = std::forward<U>(value);
            return *this;
        }

    public:
        constexpr T const* operator->() const noexcept { return get_ptr(); }
        constexpr T const* operator&()  const noexcept { return get_ptr(); }
        constexpr T*       operator->()       noexcept { return get_ptr(); }
        constexpr T*       operator&()        noexcept { return get_ptr(); }
        constexpr T const* get_ptr() const noexcept { return holds_ref ? ref : &local_value; }
        constexpr T*       get_ptr()       noexcept { return holds_ref ? ref : &local_value; }
        
        constexpr operator const T&() const noexcept { return get_ref(); }
        constexpr operator T&() noexcept { return get_ref(); }
        constexpr const T& get_ref() const noexcept { return holds_ref ? *ref : local_value; }
        constexpr T& get_ref() noexcept { return holds_ref ? *ref : local_value; }

    public:
        T& operator +=(const T& rhs) noexcept(noexcept(std::declval<T>()  += rhs)) requires (requires (T t){{t  += rhs} -> std::same_as<T&>;}) { return get_ref()  += rhs; }
        T& operator -=(const T& rhs) noexcept(noexcept(std::declval<T>()  -= rhs)) requires (requires (T t){{t  -= rhs} -> std::same_as<T&>;}) { return get_ref()  -= rhs; }
        T& operator *=(const T& rhs) noexcept(noexcept(std::declval<T>()  *= rhs)) requires (requires (T t){{t  *= rhs} -> std::same_as<T&>;}) { return get_ref()  *= rhs; }
        T& operator /=(const T& rhs) noexcept(noexcept(std::declval<T>()  /= rhs)) requires (requires (T t){{t  /= rhs} -> std::same_as<T&>;}) { return get_ref()  /= rhs; }
        T& operator &=(const T& rhs) noexcept(noexcept(std::declval<T>()  &= rhs)) requires (requires (T t){{t  &= rhs} -> std::same_as<T&>;}) { return get_ref()  &= rhs; }
        T& operator |=(const T& rhs) noexcept(noexcept(std::declval<T>()  |= rhs)) requires (requires (T t){{t  |= rhs} -> std::same_as<T&>;}) { return get_ref()  |= rhs; }
        T& operator ^=(const T& rhs) noexcept(noexcept(std::declval<T>()  ^= rhs)) requires (requires (T t){{t  ^= rhs} -> std::same_as<T&>;}) { return get_ref()  ^= rhs; }
        T& operator>>=(const T& rhs) noexcept(noexcept(std::declval<T>() >>= rhs)) requires (requires (T t){{t >>= rhs} -> std::same_as<T&>;}) { return get_ref() >>= rhs; }
        T& operator<<=(const T& rhs) noexcept(noexcept(std::declval<T>() <<= rhs)) requires (requires (T t){{t <<= rhs} -> std::same_as<T&>;}) { return get_ref() <<= rhs; }

        decltype(auto) operator[](std::size_t idx) noexcept(noexcept(std::declval<T>()[idx])) requires (requires (T t){t[idx];}) { return get_ref()[idx]; }
        decltype(auto) operator[](std::size_t idx) const noexcept(noexcept(std::declval<const T>()[idx])) requires (requires (const T t){t[idx];}) { return get_ref()[idx]; }

    public:
        using value_type = T;

    private:
        union { //TODO add constructor or force T to be trivially constructible
            T local_value;
            T* ref;
        };
        bool holds_ref = false;
    };
}