#pragma once
#include "Duo2D/error.hpp"
#include "Duo2D/traits/buffer_traits.hpp"
#include "Duo2D/vulkan/core/command_pool.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/device_memory.hpp"
#include "Duo2D/vulkan/display/render_pass.hpp"
#include "Duo2D/vulkan/memory/renderable_buffer.hpp"
#include <cstring>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/make.hpp"
#include "Duo2D/vulkan/core/command_buffer.hpp"
#include "Duo2D/traits/renderable_traits.hpp"


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    result<renderable_buffer<FiF, Ts...>> renderable_buffer<FiF, Ts...>::create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept {
        renderable_buffer ret{};
        ret.logi_device_ptr = std::addressof(logi_device);
        ret.phys_device_ptr = std::addressof(phys_device);
        
        //Create command pool for copy commands
        __D2D_TRY_MAKE(ret.copy_cmd_pool, make<command_pool>(logi_device, phys_device), ccp);

        //If theres no static index or vertex data, skip creating their buffers
        constexpr static std::size_t static_buffer_size = instanced_buffer_size[buffer_type::index] + instanced_buffer_size[buffer_type::vertex];
        if constexpr (static_buffer_size == 0) goto create_uniform;

        {
        std::array<buffer, 1> static_staging_buffs;
        //Create staging buffer for static instance data buffer
        __D2D_TRY_MAKE(static_staging_buffs[0], make<buffer>(logi_device, static_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), isb);
        __D2D_TRY_MAKE(device_memory<1> static_staging_mem, 
            make<device_memory<1>>(logi_device, phys_device, static_staging_buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), ism);
        static_staging_mem.bind(logi_device, static_staging_buffs, 0, ret.copy_cmd_pool);

        //Fill staging buffer with static instance data
        void* mapped_static_data;
        vkMapMemory(logi_device, static_staging_mem, 0, static_buffer_size, 0, &mapped_static_data);
        ((copy_staging<Ts>(mapped_static_data)), ...);
        vkUnmapMemory(logi_device, static_staging_mem);

        //Create static instance buffer
        std::array<buffer, 1> static_data_buffs;
        __D2D_TRY_MAKE(static_data_buffs[0], make<buffer>(logi_device, static_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), ib);
        __D2D_TRY_MAKE(ret.static_data_mem, make<device_memory<1>>(logi_device, phys_device, static_data_buffs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), im);
        ret.static_data_mem.bind(logi_device, static_data_buffs, 0, ret.copy_cmd_pool);
        ret.static_data_buff = std::move(static_data_buffs[0]);
        

        //Copy from index staging buffer to index device buffer
        if(auto c = ret.copy(ret.static_data_buff, static_staging_buffs[0], static_buffer_size); !c.has_value())
            return c.error();
        }

    create_uniform:
        //If theres no uniform data, just return
        constexpr static std::size_t uniform_buffer_size = (data_size<Ts>[buffer_type::uniform] + ...);
        if constexpr (uniform_buffer_size == 0) return ret;

        //Create uniform buffer
        std::array<buffer, 1> uniform_buffs;
        __D2D_TRY_MAKE(uniform_buffs[0], make<buffer>(logi_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), ub);
        __D2D_TRY_MAKE(ret.host_mem, 
            make<device_memory<1>>(logi_device, phys_device, uniform_buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), um);
        ret.host_mem.bind(logi_device, uniform_buffs, 0, ret.copy_cmd_pool);
        ret.uniform_buff = std::move(uniform_buffs[0]);
        vkMapMemory(logi_device, ret.host_mem, 0, uniform_buffer_size, 0, &ret.uniform_buffer_map);

        __D2D_TRY_MAKE(ret.desc_pool, (make<descriptor_pool<FiF, count_v[impl::type_filter::has_uniform]>>(logi_device)), dp);

        //Create pipelines and descriptors
        ((ret.create_pipelines<Ts>(logi_device, window_render_pass)), ...);

        return ret;
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_buffer<FiF, Ts...>::apply(bool shrink) noexcept {
        if constexpr (!T::instanced && impl::has_indices_v<T>) 
            if(auto c = create_buffer<T, impl::type_filter::has_index>(shrink, data_size<T>[buffer_type::index], index_buffs, index_device_local_mem); !c.has_value())
                return c.error();

        if constexpr (!T::instanced && impl::has_vertices_v<T>) 
            if(auto c = create_buffer<T, impl::type_filter::none>(shrink, data_size<T>[buffer_type::vertex], data_buffs, data_device_local_mem); !c.has_value())
                return c.error();

        if constexpr (T::instanced) 
            if(auto c = create_buffer<T, impl::type_filter::none>(shrink, data_size<T>[buffer_type::instance], data_buffs, data_device_local_mem); !c.has_value())
                return c.error();

        
        if constexpr (impl::has_attributes_v<T>) {
            constexpr static std::size_t attribute_size = impl::extract_attribute_size<typename T::attribute_types>::value;
            __D2D_TRY_MAKE(void* map, (create_shared_buffer<T, impl::type_filter::has_attrib>(shrink, attribute_size, attribute_buffs)), c);
            std::span<std::byte>& attributes_span = std::get<index_v<T>[impl::type_filter::has_attrib]>(attributes_map_tuple); //tuple<span<T0_bytes>, span<T1_bytes>>[x] => span<Tx_bytes>
            attributes_span = std::span<std::byte>(static_cast<std::byte*>(map), attribute_size * std::get<std::vector<T>>(data).size());

            constexpr static std::array<std::size_t, num_attributes<T>> attribute_sizes = [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::array<std::size_t, num_attributes<T>>{sizeof(typename std::remove_reference_t<decltype(std::get<I>(std::declval<T>().attributes()))>::value_type)...};
            }(std::make_index_sequence<num_attributes<T>>{});
            static_assert(std::accumulate(attribute_sizes.cbegin(), attribute_sizes.cend(), 0) == attribute_size);
            constexpr static std::array<std::size_t, num_attributes<T>> attribute_offsets = []() {
                std::array<std::size_t, num_attributes<T>> ret;
                std::exclusive_scan(attribute_sizes.cbegin(), attribute_sizes.cend(), ret.begin(), 0);
                return ret;
            }();

            [&]<std::size_t... I>(std::index_sequence<I...>) {
                (emplace_attribute<I, T>(std::get<std::vector<T>>(data), attributes_span, attribute_offsets, attribute_size), ...);
            }(std::make_index_sequence<num_attributes<T>>{});
        }

        outdated[index_v<T>[impl::type_filter::none]] = false;
        return {};
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool renderable_buffer<FiF, Ts...>::needs_apply() const noexcept {
        return outdated[index_v<T>[impl::type_filter::none]];
    }
}


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T, impl::type_filter::idx BuffFilter, std::size_t N>
    result<void> renderable_buffer<FiF, Ts...>::create_buffer(bool shrink, std::size_t new_size, std::array<buffer, N>& buffs, device_memory<N>& mem) noexcept {
        const std::size_t buffer_data_size = new_size * std::get<std::vector<T>>(data).size();
        if(buffer_data_size == 0) {
            buffs[index_v<T>[BuffFilter]] = buffer{};
            return error::invalid_argument;
        }
        //Create staging buffer
        std::array<buffer, 1> staging_buffs;
        __D2D_TRY_MAKE(staging_buffs[0], make<buffer>(*logi_device_ptr, buffer_data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), sb);
        __D2D_TRY_MAKE(device_memory<1> staging_mem, 
            make<device_memory<1>>(*logi_device_ptr, *phys_device_ptr, staging_buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), sm);
        staging_mem.bind(*logi_device_ptr, staging_buffs, 0, copy_cmd_pool);

        //Fill staging buffer with data
        auto get_data = [this](std::size_t i) noexcept -> decltype(auto) {
            if constexpr (!T::instanced) { 
                if constexpr (BuffFilter == impl::type_filter::has_index) return std::get<std::vector<T>>(data)[i].indices();
                else return std::get<std::vector<T>>(data)[i].vertices();
            }
            else return std::get<std::vector<T>>(data)[i].instance();
        };

        void* mapped_data;
        vkMapMemory(*logi_device_ptr, staging_mem, 0, buffer_data_size, 0, &mapped_data);
        for(std::size_t i = 0; i < std::get<std::vector<T>>(data).size(); ++i) {
            const auto input = get_data(i);
            if constexpr(!T::instanced)
                std::memcpy(static_cast<char*>(mapped_data) + new_size * i, input.data(), new_size);
            else 
                std::memcpy(static_cast<char*>(mapped_data) + new_size * i, &input, new_size);
        }
        vkUnmapMemory(*logi_device_ptr, staging_mem);

        //Re-allocate buffer if it's the wrong size
        if(buffer_data_size > buffs[index_v<T>[BuffFilter]].size()|| shrink) {
            __D2D_TRY_MAKE(buffs[index_v<T>[BuffFilter]], make<buffer>(*logi_device_ptr, buffer_data_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | ((BuffFilter == impl::type_filter::has_index) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)), b);
            //Re-allocate the memory for the buffer as well if it's the wrong size
            device_memory<N> old_mem;
            if(buffer_data_size > mem.requirements()[index_v<T>[BuffFilter]].size || shrink) {
                old_mem = std::move(mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
                __D2D_TRY_MAKE(mem, make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), m);
            }
            mem.bind(*logi_device_ptr, buffs, index_v<T>[BuffFilter], copy_cmd_pool);
        }

        //Copy from staging buffer to device buffer 
        if(auto c = copy(buffs[index_v<T>[BuffFilter]], staging_buffs[0], buffer_data_size); !c.has_value())
            return c.error();
        return {};
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T, impl::type_filter::idx BuffFilter, std::size_t N>
    result<void*> renderable_buffer<FiF, Ts...>::create_shared_buffer(bool shrink, std::size_t new_size, std::array<buffer, N>& buffs) noexcept {
        const std::size_t buffer_data_size = new_size * std::get<std::vector<T>>(data).size();
        if(buffer_data_size == 0) {
            buffs[index_v<T>[BuffFilter]] = buffer{};
            return error::invalid_argument;
        }


        //Re-allocate buffer if it's the wrong size
        if(buffer_data_size > buffs[index_v<T>[BuffFilter]].size() || shrink) {
            __D2D_TRY_MAKE(buffs[index_v<T>[BuffFilter]], make<buffer>(*logi_device_ptr, buffer_data_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | ((BuffFilter == impl::type_filter::has_storage) ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)), b);
            //Re-allocate the memory for the buffer as well if it's the wrong size
            device_memory<N> old_shared_mem;
            if(buffer_data_size > shared_mem.requirements()[index_v<T>[BuffFilter]].size || shrink) {
                old_shared_mem = std::move(shared_mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
                auto sm = make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                if(sm.has_value()) shared_mem = *std::move(sm);
                else if(sm.error() == error::device_lacks_suitable_mem_type) {
                    __D2D_TRY_MAKE(shared_mem, make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), m);
                }
                else return sm.error();
            }
            shared_mem.bind(*logi_device_ptr, buffs, index_v<T>[BuffFilter], copy_cmd_pool);
        }

        void* mapped_data;
        vkMapMemory(*logi_device_ptr, shared_mem, 0, buffer_data_size, 0, &mapped_data);
        return mapped_data;
    }

    
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_buffer<FiF, Ts...>::create_pipelines(logical_device& logi_device, render_pass& window_render_pass) noexcept {
        if constexpr (impl::has_uniform_v<T>) {
            constexpr static std::size_t I = index_v<T>[impl::type_filter::has_uniform];
            //Create descriptor set layout
            __D2D_TRY_MAKE(desc_set_layouts[I], make<descriptor_set_layout>(logi_device), dsl);

            //Create pipeline layouts with descriptors
            __D2D_TRY_MAKE(std::get<pipeline_layout<T>>(pipeline_layouts), make<pipeline_layout<T>>(logi_device, desc_set_layouts[I]), pl);

            //Create descriptor set and pool
            __D2D_TRY_MAKE(desc_sets[I], (make<descriptor_set<FiF>>(logi_device, desc_pool, desc_set_layouts[I], uniform_buff, data_size<T>[buffer_type::uniform], offsets<T>()[buffer_type::uniform])), ds);
        } else {
            //Create pipeline layouts
            __D2D_TRY_MAKE(std::get<pipeline_layout<T>>(pipeline_layouts), make<pipeline_layout<T>>(logi_device), pl);
        }

        //Create pipelines
        __D2D_TRY_MAKE(std::get<pipeline<T>>(pipelines), make<pipeline<T>>(logi_device, window_render_pass, std::get<pipeline_layout<T>>(pipeline_layouts)), p);
        return {};
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    result<void> renderable_buffer<FiF, Ts...>::copy(buffer& dst, const buffer& src, std::size_t size) const noexcept {
        __D2D_TRY_MAKE(command_buffer copy_cmd_buffer, (make<command_buffer>(*logi_device_ptr, copy_cmd_pool)), ccb);
        if(auto b = copy_cmd_buffer.copy_begin(); !b.has_value()) return b.error();
        copy_cmd_buffer.copy(dst, src, size);
        if(auto e = copy_cmd_buffer.copy_end(*logi_device_ptr, copy_cmd_pool); !e.has_value()) return e.error();

        return {};
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    void renderable_buffer<FiF, Ts...>::copy_staging(void* data_map) noexcept {
        if constexpr (T::instanced && impl::has_indices_v<T>)
            std::memcpy(static_cast<char*>(data_map) + offsets<T>()[buffer_type::index], T::indices().data(), data_size<T>[buffer_type::index]);
        if constexpr (T::instanced && impl::has_vertices_v<T>)
            std::memcpy(static_cast<char*>(data_map) + offsets<T>()[buffer_type::vertex], T::vertices().data(), data_size<T>[buffer_type::vertex]);
    }
}


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename U> requires impl::RenderableType<std::remove_cvref_t<U>>
    constexpr void renderable_buffer<FiF, Ts...>::push_back(U&& value) noexcept {
        using T = std::remove_cvref_t<U>;
        std::vector<T>& data_vec = std::get<std::vector<T>>(data);
        data_vec.push_back(std::forward<U>(value));
        outdated[index_v<T>[impl::type_filter::none]] = true;
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T, typename... Args>
    constexpr T& renderable_buffer<FiF, Ts...>::emplace_back(Args&&... args) noexcept {
        std::vector<T>& data_vec = std::get<std::vector<T>>(data);
        T& ret = data_vec.emplace_back(std::forward<Args>(args)...);
        outdated[index_v<T>[impl::type_filter::none]] = true;
        return ret;
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::iterator renderable_buffer<FiF, Ts...>::erase(typename std::vector<T>::const_iterator pos) noexcept {
        return std::get<std::vector<T>>(data).erase(pos);
    }
    
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::iterator renderable_buffer<FiF, Ts...>::erase(typename std::vector<T>::const_iterator f, typename std::vector<T>::const_iterator l) noexcept {
        return std::get<std::vector<T>>(data).erase(f, l);
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::const_iterator renderable_buffer<FiF, Ts...>::cbegin() const noexcept {
        return std::get<std::vector<T>>(data).cbegin();
    }
    
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr typename std::vector<T>::const_iterator renderable_buffer<FiF, Ts...>::cend() const noexcept {
        return std::get<std::vector<T>>(data).cend();
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr bool renderable_buffer<FiF, Ts...>::empty() const noexcept {
        return std::get<std::vector<T>>(data).empty();
    }
    
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::size() const noexcept {
        return std::get<std::vector<T>>(data).size();
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<std::size_t I, typename T>
    constexpr void renderable_buffer<FiF, Ts...>::emplace_attribute(std::vector<T>& data_vec, std::span<std::byte> attributes_span, std::array<std::size_t, num_attributes<T>> attribute_offsets, std::size_t attribute_size) noexcept {
        using attribute_ref_type = decltype(std::get<I>(std::declval<T>().attributes())); //attribute<V>&
        using attribute_type = std::remove_reference_t<attribute_ref_type>; //attribute<V>
        using attribute_value_type = typename attribute_type::value_type; //V
        static_assert(std::is_lvalue_reference_v<attribute_ref_type> && !std::is_const_v<attribute_type>);
        
        for(std::size_t i = 0; i < data_vec.size(); ++i){
            attribute_ref_type attribute = std::get<I>(data_vec[i].attributes()); //tuple<attribute<Tx_Ai>&, attribute<Tx_Bi>&, ..., attribute<Tx_Ni&>[I] => attribute<Tx_Ii>&
            attribute_value_type old_value = attribute.get_ref(); //value = attribute<Tx_Ii>
            attribute = attribute_type(reinterpret_cast<attribute_value_type*>(&attributes_span[(attribute_size * i) + attribute_offsets[I]])); //attribute<Tx_Ii>& = attribute<Tx_Ii>(&span<Tx_I>[i])
            attribute.get_ref() = old_value;
        }
    }
}


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_buffer<FiF, Ts...>::index_buffer() const noexcept requires impl::has_indices_v<T> { 
        if constexpr (T::instanced) return static_data_buff; 
        else return index_buffs[index_v<T>[impl::type_filter::has_index]]; 
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_buffer<FiF, Ts...>::uniform_buffer() const noexcept requires impl::has_uniform_v<T> { 
        return uniform_buff; 
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr const buffer& renderable_buffer<FiF, Ts...>::vertex_buffer() const noexcept requires impl::has_vertices_v<T>  { 
        if constexpr (T::instanced) return static_data_buff; 
        else return data_buffs[index_v<T>[impl::type_filter::none]]; 
    };

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_buffer<FiF, Ts...>::instance_buffer() const noexcept requires (T::instanced) { 
        return data_buffs[index_v<T>[impl::type_filter::none]]; 
    };

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const buffer& renderable_buffer<FiF, Ts...>::attribute_buffer() const noexcept requires impl::has_attributes_v<T> { 
        return attribute_buffs[index_v<T>[impl::type_filter::has_attrib]]; 
    };


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::index_count() const noexcept {
        std::size_t idx_cnt = T::index_count;
        if constexpr (!T::instanced) idx_cnt *= std::get<std::vector<T>>(data).size(); 
        return idx_cnt;
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::vertex_count() const noexcept {
        std::size_t vert_cnt = T::vertex_count;
        if constexpr (!T::instanced) vert_cnt *= std::get<std::vector<T>>(data).size(); 
        return vert_cnt;
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::instance_count() const noexcept { 
        if constexpr (T::instanced) return std::get<std::vector<T>>(data).size(); 
        else return 1;
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    consteval buffer_bytes_t renderable_buffer<FiF, Ts...>::offsets() noexcept { 
        std::array<buffer_bytes_t, count_v[impl::type_filter::none]> buff_offsets;
        constexpr std::array<buffer_bytes_t, count_v[impl::type_filter::none]> static_sizes = {
            (Ts::instanced ? data_size<Ts> : buffer_bytes_t{0, data_size<Ts>[buffer_type::uniform], 0, 0})...
        };
        
        std::exclusive_scan(static_sizes.cbegin(), static_sizes.cend(), buff_offsets.begin(), buffer_bytes_t{},
            [](buffer_bytes_t acc, const buffer_bytes_t& prev_size){ return buffer_bytes_t{
                acc[buffer_type::vertex] + prev_size[buffer_type::vertex] + prev_size[buffer_type::index],
                acc[buffer_type::uniform] + prev_size[buffer_type::uniform],
                acc[buffer_type::vertex] + prev_size[buffer_type::vertex] + prev_size[buffer_type::index],
                0
            };});
        for(std::size_t i = 0; i < count_v[impl::type_filter::none]; ++i)
            buff_offsets[i][buffer_type::vertex] += static_sizes[i][buffer_type::index];
        return buff_offsets[index_v<T>[impl::type_filter::none]];
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<typename T::uniform_type> renderable_buffer<FiF, Ts...>::uniform_map() const noexcept requires impl::has_uniform_v<T> { 
        return {reinterpret_cast<typename T::uniform_type*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + offsets<T>()[buffer_type::uniform]), FiF}; 
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline<T>& renderable_buffer<FiF, Ts...>::associated_pipeline() const noexcept{ 
        return std::get<pipeline<T>>(pipelines);
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const pipeline_layout<T>& renderable_buffer<FiF, Ts...>::associated_pipeline_layout() const noexcept{ 
        return std::get<pipeline_layout<T>>(pipeline_layouts);
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr const descriptor_set<FiF>& renderable_buffer<FiF, Ts...>::desc_set() const noexcept{ 
        return desc_sets[index_v<T>[impl::type_filter::has_uniform]];
    }
}