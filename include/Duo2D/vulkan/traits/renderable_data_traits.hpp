#pragma once
#include <cstddef>
#include <type_traits>
#include <tuple>

#include <llfio.hpp>

#include "Duo2D/traits/directly_renderable.hpp"
#include "Duo2D/traits/renderable_container_like.hpp"
#include "Duo2D/traits/same_as.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/graphics/core/font.hpp"

namespace d2d {
    template<std::size_t FramesInFlight, typename> class renderable_tuple;
}


namespace d2d::vk {
    template<typename... Ts>
    struct renderable_data_traits {
        template<typename T>
        using data_single_tuple_type           = std::conditional_t<::d2d::impl::directly_renderable<T>,       std::tuple<T>,                             std::tuple<>>;
        template<typename T>
        using container_data_single_tuple_type = std::conditional_t<::d2d::impl::renderable_container_like<T>, std::tuple<impl::renderable_input_map<T>>, std::tuple<>>;


        using data_tuple_type           = decltype(std::tuple_cat(std::declval<data_single_tuple_type<Ts>>()...));
        using container_data_tuple_type = decltype(std::tuple_cat(std::declval<container_data_single_tuple_type<Ts>>()...));
        
        template<typename T, std::size_t FiF> struct map_traits {};
        template<::d2d::impl::when_decayed_same_as<::d2d::font> F, std::size_t FiF> struct map_traits<F, FiF> {
            using key_type       = typename ::d2d::impl::font_path_map::key_type      ;
            using mapped_type    = typename ::d2d::impl::font_path_map::mapped_type   ;
            using value_type     = typename ::d2d::impl::font_path_map::value_type    ;
            using iterator       = typename ::d2d::impl::font_path_map::iterator      ;
            using const_iterator = typename ::d2d::impl::font_path_map::const_iterator;
        };
        template<::d2d::impl::directly_renderable R, std::size_t FiF> struct map_traits<R, FiF> {
            using key_type       = typename renderable_tuple<FiF, data_tuple_type>::template key_type<R>      ;
            using mapped_type    = typename renderable_tuple<FiF, data_tuple_type>::template mapped_type<R>   ;
            using value_type     = typename renderable_tuple<FiF, data_tuple_type>::template value_type<R>    ;
            using iterator       = typename renderable_tuple<FiF, data_tuple_type>::template iterator<R>      ;
            using const_iterator = typename renderable_tuple<FiF, data_tuple_type>::template const_iterator<R>;
        };
        template<::d2d::impl::renderable_container_like C, std::size_t FiF> struct map_traits<C, FiF> {
            using key_type       = typename impl::renderable_input_map<C>::key_type      ;
            using mapped_type    = typename impl::renderable_input_map<C>::mapped_type   ;
            using value_type     = typename impl::renderable_input_map<C>::value_type    ;
            using iterator       = typename impl::renderable_input_map<C>::iterator      ;
            using const_iterator = typename impl::renderable_input_map<C>::const_iterator;  
        };
    

        template<typename T, std::size_t FiF> using key_type       = typename map_traits<T, FiF>::key_type      ;
        template<typename T, std::size_t FiF> using mapped_type    = typename map_traits<T, FiF>::mapped_type   ;
        template<typename T, std::size_t FiF> using value_type     = typename map_traits<T, FiF>::value_type    ;
        template<typename T, std::size_t FiF> using iterator       = typename map_traits<T, FiF>::iterator      ;
        template<typename T, std::size_t FiF> using const_iterator = typename map_traits<T, FiF>::const_iterator;  
    };
}