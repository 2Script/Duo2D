#pragma once
#include <array>
#include <climits>
#include <concepts>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <bitset>

#include <cstddef>
#include <cstdint>

#include <GLFW/glfw3.h>

#include "Duo2D/input/code.hpp"


namespace d2d::input {
    constexpr static std::size_t combination_size = (std::numeric_limits<code_t>::max() + 1) / CHAR_BIT;

    using combination_array = std::array<std::uint8_t, combination_size / sizeof(std::uint8_t)>;
    using combination_bitset = std::bitset<combination_size * CHAR_BIT>;
}


namespace d2d::input {
    class combination_reference {
        friend struct combination;

    public:
        constexpr combination_reference(combination_reference const&) noexcept = default;
        ~combination_reference() noexcept = default;
    public:
        constexpr combination_reference& operator=(bool x) noexcept {
            if(x) *elem_ptr |= mask;
            else *elem_ptr &= ~mask;
            return *this;
        }
        constexpr combination_reference& operator=(combination_reference const& other) noexcept { return operator=(static_cast<bool>(other)); }
    public:
        constexpr operator bool() const noexcept { return static_cast<bool>(*elem_ptr & mask); }
        constexpr bool operator~() const noexcept { return !static_cast<bool>(*this); }
    public:
        constexpr combination_reference& flip() noexcept {
	        *elem_ptr ^= mask;
            return *this;
        }
    
    private:
        constexpr combination_reference() noexcept = default;
        constexpr combination_reference(typename combination_array::value_type* p, typename combination_array::value_type m) noexcept :
            elem_ptr(p), mask(m) {}
    private:
        typename combination_array::value_type* elem_ptr;
        typename combination_array::value_type mask;
    };
}


namespace d2d::input {
    //TODO: making combination SIMD aligned (using alignas) causes garbage values when used in binding_map
    struct /*alignas(combination_size)*/ combination : combination_array {
        constexpr combination() noexcept = default;
        constexpr combination(std::initializer_list<code_t> modifiers, code_t main) noexcept : combination_array{} {
            main_input() = main;
            //set(main, true);
            for(code_t mod : modifiers) set(mod, true);
        }


    public:
        constexpr code_t      & main_input()       noexcept { return combination_array::operator[](size() - 1); }
        constexpr code_t const& main_input() const noexcept { return combination_array::operator[](size() - 1); }

    public:
        constexpr bool operator==(const combination& rhs) const noexcept { return __builtin_memcmp(data(), rhs.data(), combination_size) == 0; }
        constexpr bool operator<=>(const combination& rhs) const noexcept { return __builtin_memcmp(data(), rhs.data(), combination_size); }
    
    public:
        constexpr bool                  operator[](code_t code) const noexcept { return combination_array::operator[](elem_idx(code)) & bit_mask(code); }
        constexpr combination_reference operator[](code_t code)       noexcept { return combination_reference{&combination_array::operator[](elem_idx(code)), bit_mask(code)}; }

    public:
        constexpr std::size_t size() const noexcept { return combination_size / sizeof(value_type); }
    
    public:
        //constexpr combination& set() const noexcept {}
        constexpr combination& set(code_t code, bool value = true) noexcept { 
            if(value) combination_array::operator[](elem_idx(code)) |= bit_mask(code);
            else combination_array::operator[](elem_idx(code)) &= ~bit_mask(code);
            return *this;
        }
    

    private:
        constexpr static std::size_t elem_idx(code_t code) noexcept { return code / (sizeof(typename combination_array::value_type) * CHAR_BIT); }
        constexpr static std::size_t bit_idx (code_t code) noexcept { return code % (sizeof(typename combination_array::value_type) * CHAR_BIT); }
        constexpr static typename combination_array::value_type bit_mask(code_t code) noexcept { return static_cast<typename combination_array::value_type>(1) << bit_idx(code); }
    };
}



namespace std {
    template<>
    struct hash<d2d::input::combination> {
        constexpr std::size_t operator()(d2d::input::combination input_combo) const noexcept {
            return std::hash<d2d::input::combination_bitset>{}(std::bit_cast<d2d::input::combination_bitset>(input_combo));
        }
    };
}