#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


namespace d2d::impl {
    template<typename T>
    class hybrid_ptr_base {
    public:
        using pointer = T*;

    public:
        constexpr hybrid_ptr_base(pointer ptr = nullptr, bool val = false) noexcept : _ptr_value(std::bit_cast<std::uintptr_t>(ptr)), _holds_ref(val) {}
        ~hybrid_ptr_base() noexcept = default;

        constexpr hybrid_ptr_base(const hybrid_ptr_base& other) noexcept :
            _ptr_value(other._ptr_value ? (other.holds_reference() ? other._ptr_value : std::bit_cast<std::uintptr_t>(new T(*std::bit_cast<pointer>(other._ptr_value)))) : std::uintptr_t{}),
            _holds_ref(other.holds_reference()) {}
        constexpr hybrid_ptr_base& operator=(const hybrid_ptr_base& other) noexcept {
            _ptr_value = other._ptr_value ? (other.holds_reference() ? other._ptr_value : std::bit_cast<std::uintptr_t>(new T(*std::bit_cast<pointer>(other._ptr_value)))) : std::uintptr_t{};
            _holds_ref = other.holds_reference();
        }

        constexpr hybrid_ptr_base(hybrid_ptr_base&& other) noexcept : 
            _ptr_value(std::exchange(other._ptr_value, std::uintptr_t{})),
            _holds_ref(std::exchange(other._holds_ref, false)) {}
        constexpr hybrid_ptr_base& operator=(hybrid_ptr_base&& other) noexcept {
            _ptr_value = std::exchange(other._ptr_value, std::uintptr_t{});
            _holds_ref = std::exchange(other._holds_ref, false);
            return *this;
        };


    public:
        constexpr pointer get()             const noexcept { return std::bit_cast<pointer>(this->_ptr_value); }
        constexpr bool    holds_reference() const noexcept { return this->_holds_ref; }


    protected:
        std::uintptr_t _ptr_value;
        bool _holds_ref;
    };

    template<typename T> requires (alignof(T) % 2 == 0)
    class hybrid_ptr_base<T> {
    public:
        using pointer = T*;

    public:
        constexpr hybrid_ptr_base(pointer ptr = nullptr, bool val = false) noexcept : _ptr_value(std::bit_cast<std::uintptr_t>(ptr) | (val & 0b1)){}
        ~hybrid_ptr_base() noexcept = default;

        constexpr hybrid_ptr_base(const hybrid_ptr_base& other) noexcept : 
            _ptr_value(other._ptr_value ? (other.holds_reference() ? other._ptr_value : std::bit_cast<std::uintptr_t>(new T(*std::bit_cast<pointer>(other._ptr_value)))) : std::uintptr_t{}) {}
        constexpr hybrid_ptr_base& operator=(const hybrid_ptr_base& other) {
            _ptr_value = other._ptr_value ? (other.holds_reference() ? other._ptr_value : std::bit_cast<std::uintptr_t>(new T(*std::bit_cast<pointer>(other._ptr_value)))) : std::uintptr_t{};
            return *this;
        };

        constexpr hybrid_ptr_base(hybrid_ptr_base&& other) noexcept : 
            _ptr_value(std::exchange(other._ptr_value, std::uintptr_t{})) {}
        constexpr hybrid_ptr_base& operator=(hybrid_ptr_base&& other) noexcept {
            _ptr_value = std::exchange(other._ptr_value, std::uintptr_t{});
            return *this;
        };

        
    public:
        constexpr pointer get()             const noexcept { return std::bit_cast<pointer>(this->_ptr_value & ~static_cast<std::uintptr_t>(0b1)); }
        constexpr bool    holds_reference() const noexcept { return this->_ptr_value & 0b1; }

    
    protected:
        std::uintptr_t _ptr_value;
    };
}


namespace d2d {
    //TODO: more intuitive name
    template<typename T, typename Deleter = std::default_delete<T>> requires (!std::is_rvalue_reference_v<Deleter>)
    class hybrid_ptr : public impl::hybrid_ptr_base<T> {
        constexpr static bool even_alignment = (alignof(T) % 2 != 0);
    public:
        using pointer = impl::hybrid_ptr_base<T>::pointer;
        using reference_wrapper = std::reference_wrapper<T>;
        using element_type = T;
        using deleter_type = Deleter;

    public:
        constexpr hybrid_ptr(std::nullptr_t = nullptr) noexcept requires std::is_nothrow_default_constructible_v<Deleter> : 
            impl::hybrid_ptr_base<T>(), del{} {}
    public:
        constexpr explicit hybrid_ptr(pointer p) noexcept requires std::is_nothrow_default_constructible_v<Deleter> : 
            impl::hybrid_ptr_base<T>(p, false), del{} {}

        constexpr hybrid_ptr(reference_wrapper r) noexcept :
            impl::hybrid_ptr_base<T>(std::addressof(r.get()), true), del{} {}


        constexpr hybrid_ptr(pointer p, deleter_type const& d                   ) noexcept requires (!std::is_lvalue_reference_v<deleter_type> && std::is_nothrow_copy_constructible_v<deleter_type>):
            impl::hybrid_ptr_base<T>(p, false), del(std::forward<decltype(d)>(d)) {}
        constexpr hybrid_ptr(pointer p, deleter_type&& d                        ) noexcept requires (!std::is_lvalue_reference_v<deleter_type> && std::is_nothrow_move_constructible_v<deleter_type>):
            impl::hybrid_ptr_base<T>(p, false), del(std::forward<decltype(d)>(d)) {}
        constexpr hybrid_ptr(pointer p, std::remove_reference_t<deleter_type>& d) noexcept requires std::is_lvalue_reference_v<deleter_type>:
            impl::hybrid_ptr_base<T>(p, false), del(std::forward<decltype(d)>(d)) {}

        constexpr hybrid_ptr(reference_wrapper r, deleter_type const& d                   ) noexcept requires (!std::is_lvalue_reference_v<deleter_type> && std::is_nothrow_copy_constructible_v<deleter_type>):
            impl::hybrid_ptr_base<T>(std::addressof(r.get()), true), del(std::forward<decltype(d)>(d)) {}
        constexpr hybrid_ptr(reference_wrapper r, deleter_type&& d                        ) noexcept requires (!std::is_lvalue_reference_v<deleter_type> && std::is_nothrow_move_constructible_v<deleter_type>):
            impl::hybrid_ptr_base<T>(std::addressof(r.get()), true), del(std::forward<decltype(d)>(d)) {}
        constexpr hybrid_ptr(reference_wrapper r, std::remove_reference_t<deleter_type>& d) noexcept requires std::is_lvalue_reference_v<deleter_type>:
            impl::hybrid_ptr_base<T>(std::addressof(r.get()), true), del(std::forward<decltype(d)>(d)) {}
        

    public:
        constexpr hybrid_ptr(hybrid_ptr&& other) noexcept : 
            impl::hybrid_ptr_base<T>(std::move(other)),
            del(std::move(other.del)) {}
        constexpr hybrid_ptr& operator=(hybrid_ptr&& other) noexcept {
            if(!this->holds_reference() && this->get()) get_deleter()(this->get());
            impl::hybrid_ptr_base<T>::operator=(std::move(other));
            del = std::move(other.del);
            return *this;
        }

        constexpr hybrid_ptr(hybrid_ptr const& other) noexcept : 
            impl::hybrid_ptr_base<T>(other), 
            del(other.del) {}
        constexpr hybrid_ptr& operator=(hybrid_ptr const& other) noexcept {
            impl::hybrid_ptr_base<T>::operator=(other);
            del = other.del;
            return *this;
        }
    public:
        constexpr ~hybrid_ptr() noexcept { if(!this->holds_reference() && this->get()) get_deleter()(this->get()); }
        

    public:
        constexpr deleter_type const& get_deleter() const noexcept { return del; }
        constexpr deleter_type      & get_deleter()       noexcept { return del; }

    public:
        constexpr std::add_lvalue_reference_t<T> operator* () const noexcept(noexcept(*std::declval<pointer>())) { return *this->get(); }
        constexpr pointer                        operator->() const noexcept                                     { return this->get();  }


    private:
        [[no_unique_address]] Deleter del;
    };
}

namespace d2d {
    template<typename T, typename... Args>
    inline hybrid_ptr<T> make_hybrid(Args&&... args) {
        return hybrid_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename T>
    inline hybrid_ptr<T> make_hybrid_for_overwrite() {
        return hybrid_ptr<T>(new T);
    }
}