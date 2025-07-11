#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include "Duo2D/core/hybrid_ptr.hpp"
#include "Duo2D/traits/different_from.hpp"


namespace d2d::vk {
    template<typename T> requires std::is_standard_layout_v<T>
    struct attribute {
        inline attribute() noexcept : val(new T) {}
        template<typename U> requires (std::is_constructible_v<T, U> && !std::is_same_v<T*, std::remove_cvref_t<U>>)
        inline attribute(U&& value) noexcept : val(new T(std::forward<U>(value))) {}
        constexpr attribute(T* value) noexcept : val(std::ref(*value)) {}
        
    public:
        template<::d2d::impl::when_decayed_different_from<attribute> U>
        constexpr attribute& operator=(U&& value) noexcept {
            get_ref() = std::forward<U>(value);
            return *this;
        }

    public:
        constexpr T const* operator->() const noexcept { return get_ptr(); }
        constexpr T const* operator&()  const noexcept { return get_ptr(); }
        constexpr T*       operator->()       noexcept { return get_ptr(); }
        constexpr T*       operator&()        noexcept { return get_ptr(); }
        constexpr T const* get_ptr() const noexcept { return val.get(); }
        constexpr T*       get_ptr()       noexcept { return val.get(); }
        
        constexpr operator T const&() const noexcept { return get_ref(); }
        constexpr operator T      &()       noexcept { return get_ref(); }
        constexpr T const& get_ref() const noexcept { return *val.get(); }
        constexpr T      & get_ref()       noexcept { return *val.get(); }

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

        decltype(auto) operator[](std::size_t idx)       noexcept(noexcept(std::declval<T>()[idx]))       requires (requires (      T t){t[idx];}) { return get_ref()[idx]; }
        decltype(auto) operator[](std::size_t idx) const noexcept(noexcept(std::declval<const T>()[idx])) requires (requires (const T t){t[idx];}) { return get_ref()[idx]; }

    public:
        using value_type = T;

    private:
        hybrid_ptr<T> val;
    };
}