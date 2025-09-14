#pragma once
#include <cstddef>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>


namespace d2d {
    //TODO: sparse_vector with std::vector<T>
    template<typename Key, typename T, typename Hash = std::hash<Key>>
    class contiguous_map : public std::vector<std::pair<Key, T>> {
        using base_type = std::vector<std::pair<Key, T>>;
    public:
        using key_type = Key;
        using mapped_type = T;

    public:
        inline contiguous_map() noexcept = default;
        //TODO: more constructors

    public:
        template<typename K> T      & operator[](K&& key)       noexcept;
        template<typename K> T const& operator[](K&& key) const noexcept;

        template<typename K> T      & at(K&& key)       noexcept;
        template<typename K> T const& at(K&& key) const noexcept;

        template<typename K> typename base_type::iterator       find(K&& key) noexcept;
        template<typename K> typename base_type::const_iterator find(K&& key) const noexcept;

    public:
        void clear() noexcept;

        template<typename P>
        std::pair<typename base_type::iterator, bool> insert(P&& value) noexcept requires std::is_constructible_v<typename base_type::value_type, P&&>;
        
        template<typename K, typename M>
        std::pair<typename base_type::iterator, bool> insert_or_assign(K&& key, M&& value) noexcept;

        template<typename... Args>
        std::pair<typename base_type::iterator, bool> emplace(Args&&... args) noexcept requires std::is_constructible_v<typename base_type::value_type, Args&&...>;
        
        template<typename K, typename... Args>
        std::pair<typename base_type::iterator, bool> try_emplace(K&& key, Args&&... args) noexcept;
        
        typename base_type::iterator erase(typename base_type::const_iterator pos) noexcept;
        typename base_type::iterator erase(typename base_type::const_iterator first, typename base_type::const_iterator last) noexcept;
        template<typename K>
        std::size_t erase(K&& key) noexcept;

        void swap(contiguous_map& other) noexcept;


    private:
        using base_type::at;
        using base_type::operator[];
    private:
        using base_type::clear;
        using base_type::insert;
        using base_type::emplace;
        using base_type::erase;
        using base_type::push_back;
        using base_type::emplace_back;
        using base_type::pop_back;
        using base_type::resize;
        using base_type::swap;

    private:
        std::unordered_map<Key, std::size_t> idx_map;
        //std::vector<typename std::unordered_map<Key, std::size_t>::iterator> idx_iters;
        //OR (better yet)
        //std::vector<Key> key_map;
    };
}

#include "Duo2D/core/contiguous_map.inl"