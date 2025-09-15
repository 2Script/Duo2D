#pragma once
#include "Duo2D/core/contiguous_map.hpp"

#include <algorithm>
#include <exception>
#include <iterator>
#include <utility>


namespace d2d {
    template<typename Key, typename T, typename Hash>
    template<typename K> 
    T      & contiguous_map<Key, T, Hash>::operator[](K&& key)       noexcept {
        auto idx = idx_map.find(std::forward<K>(key));
        return base_type::operator[](idx->second);
    }

    template<typename Key, typename T, typename Hash>
    template<typename K> 
    T const& contiguous_map<Key, T, Hash>::operator[](K&& key) const noexcept {
        auto idx = idx_map.find(std::forward<K>(key));
        return base_type::operator[](idx->second);
    }


    template<typename Key, typename T, typename Hash>
    template<typename K> 
    T      & contiguous_map<Key, T, Hash>::at(K&& key)       noexcept {
        const std::size_t idx = idx_map.at(std::forward<K>(key));
        return base_type::at(idx);
    }

    template<typename Key, typename T, typename Hash>
    template<typename K> 
    T const& contiguous_map<Key, T, Hash>::at(K&& key) const noexcept {
        const std::size_t idx = idx_map.at(std::forward<K>(key));
        return base_type::at(idx);
    }


    template<typename Key, typename T, typename Hash>
    template<typename K> 
    typename contiguous_map<Key, T, Hash>::base_type::iterator       contiguous_map<Key, T, Hash>::find(K&& key) noexcept {
        auto idx = idx_map.find(std::forward<K>(key));
        if(idx == idx_map.end()) return this->end();
        return this->begin() + idx->second;
    }

    template<typename Key, typename T, typename Hash>
    template<typename K> 
    typename contiguous_map<Key, T, Hash>::base_type::const_iterator contiguous_map<Key, T, Hash>::find(K&& key) const noexcept {
        auto idx = idx_map.find(std::forward<K>(key));
        if(idx == idx_map.end()) return this->cend();
        return this->cbegin() + idx->second;
    }
}

namespace d2d {
    template<typename Key, typename T, typename Hash>
    void contiguous_map<Key, T, Hash>::clear() noexcept {
        base_type::clear();
        idx_map.clear();
    }


    template<typename Key, typename T, typename Hash>
    template<typename P>
    std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool> contiguous_map<Key, T, Hash>::insert(P&& value) noexcept requires std::is_constructible_v<typename base_type::value_type, P&&> {
        typename base_type::value_type pair(std::forward<P>(value));
        auto idx_emplace = idx_map.try_emplace(pair.first);
        if(!idx_emplace.second) return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{this->begin() + idx_emplace.first->second, false};
        base_type::push_back(std::move(pair));
        idx_emplace.first->second = this->size() - 1;
        return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{base_type::end() - 1, true};
    }


    template<typename Key, typename T, typename Hash>
    template<typename K, typename M>
    std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool> contiguous_map<Key, T, Hash>::insert_or_assign(K&& key, M&& value) noexcept {
        auto idx_emplace = idx_map.try_emplace(std::forward<K>(key));
        if(!idx_emplace.second) { 
            base_type::operator[](idx_emplace.first->second).second = std::forward<M>(value);
            return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{this->begin() + idx_emplace.first->second, false};
        }
        base_type::emplace_back(idx_emplace.first->first, std::forward<M>(value));
        idx_emplace.first->second = this->size() - 1;
        return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{base_type::end() - 1, true};
    }
    

    template<typename Key, typename T, typename Hash>
    template<typename... Args>
    std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool> contiguous_map<Key, T, Hash>::emplace(Args&&... args) noexcept requires std::is_constructible_v<typename base_type::value_type, Args&&...> {
        typename base_type::value_type pair(std::forward<Args>(args)...);
        auto idx_emplace = idx_map.try_emplace(pair.first);
        if(!idx_emplace.second) return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{this->begin() + idx_emplace.first->second, false};
        base_type::push_back(std::move(pair));
        idx_emplace.first->second = this->size() - 1;
        return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{base_type::end() - 1, true};
    }
    

    template<typename Key, typename T, typename Hash>
    template<typename K, typename... Args>
    std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool> contiguous_map<Key, T, Hash>::try_emplace(K&& key, Args&&... args) noexcept {
        auto idx_emplace = idx_map.try_emplace(std::forward<K>(key));
        if(!idx_emplace.second) return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{this->begin() + idx_emplace.first->second, false};
        base_type::emplace_back(idx_emplace.first->first, T(std::forward<Args>(args)...));
        idx_emplace.first->second = this->size() - 1;
        return std::pair<typename contiguous_map<Key, T, Hash>::base_type::iterator, bool>{base_type::end() - 1, true};
    }
    
    
    
    template<typename Key, typename T, typename Hash>
    typename contiguous_map<Key, T, Hash>::base_type::iterator contiguous_map<Key, T, Hash>::erase(typename base_type::const_iterator pos) noexcept {
        if(pos == base_type::cend()) [[unlikely]] std::terminate();

        const std::size_t key_idx = std::distance(base_type::cbegin(), pos);
        Key const& key = base_type::operator[](key_idx).first;

        const std::size_t last_idx = this->size() - 1;
        if(last_idx != key_idx){
            Key const& last_key = base_type::operator[](last_idx).first;
            auto last_it = idx_map.find(last_key);
            if(last_it == idx_map.end()) [[unlikely]] std::terminate();
            last_it->second = key_idx;
            base_type::operator[](key_idx) = std::move(base_type::operator[](last_idx));
        }
        
        base_type::pop_back();
        idx_map.erase(key);
        return base_type::begin() + key_idx;
    }

    template<typename Key, typename T, typename Hash>
    typename contiguous_map<Key, T, Hash>::base_type::iterator contiguous_map<Key, T, Hash>::erase(typename base_type::const_iterator first, typename base_type::const_iterator last) noexcept {
        if(first == last) return last;

        typename base_type::iterator ret;
        for(auto it = first; it != last; ++it)
            ret = this->erase(it);
        return ret;

    }

    template<typename Key, typename T, typename Hash>
    template<typename K>
    std::size_t contiguous_map<Key, T, Hash>::erase(K&& key) noexcept {
        auto idx_it = idx_map.find(std::forward<K>(key));
        if(idx_it == idx_map.end()) return 0;
        const std::size_t key_idx = idx_it->second;
        
        const std::size_t last_idx = this->size() - 1;
        if(last_idx != key_idx){
            Key const& last_key = base_type::operator[](last_idx).first;
            auto last_it = idx_map.find(last_key);
            if(last_it == idx_map.end()) [[unlikely]] std::terminate();
            last_it->second = key_idx;
            base_type::operator[](key_idx) = std::move(base_type::operator[](last_idx));
        }

        base_type::pop_back();
        idx_map.erase(idx_it);
        return 1;
    }
    

    template<typename Key, typename T, typename Hash>
    void contiguous_map<Key, T, Hash>::swap(contiguous_map& other) noexcept {
        idx_map.swap(other.idx_map);
        base_type::swap(other);
    }
    
}