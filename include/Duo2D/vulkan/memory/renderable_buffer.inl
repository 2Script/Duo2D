#pragma once
#include "Duo2D/traits/buffer_traits.hpp"
#include "Duo2D/traits/renderable_traits.hpp"
#include "Duo2D/vulkan/memory/renderable_buffer.hpp"
#include <memory>
#include <numeric>
#include <result/verify.h>
#include <type_traits>
#include <vulkan/vulkan_core.h>


namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    result<renderable_buffer<FiF, Ts...>> renderable_buffer<FiF, Ts...>::create(logical_device& logi_device, physical_device& phys_device, render_pass& window_render_pass) noexcept {
        renderable_buffer ret{};
        ret.logi_device_ptr = std::addressof(logi_device);
        ret.phys_device_ptr = std::addressof(phys_device);
        
        //Create command pool for copy commands
        __D2D_TRY_MAKE(ret.copy_cmd_pool, make<command_pool>(logi_device, phys_device), ccp);

        //If theres no static index or vertex data, skip creating their buffers
        constexpr static std::size_t static_buffer_size = instanced_buffer_size[buffer_data_type::index] + instanced_buffer_size[buffer_data_type::vertex];
        if constexpr (static_buffer_size == 0) goto create_uniform;

        {
        constexpr static std::size_t static_data_count = ((Ts::instanced ? impl::has_fixed_indices_v<Ts> + impl::has_fixed_vertices_v<Ts> : 0) + ...);
        constexpr static std::array<std::span<const std::byte>, static_data_count> inputs = [](){
            std::array<std::span<const std::byte>, static_data_count> ret;
            std::size_t idx = 0;
            constexpr auto emplace_instanced_input = []<typename T>(std::array<std::span<const std::byte>, static_data_count>& arr, std::size_t& i){
                if constexpr (T::instanced && impl::has_fixed_indices_v<T>)  arr[i++] = std::span<const std::byte>(fixed_index_bytes<T>);
                if constexpr (T::instanced && impl::has_fixed_vertices_v<T>) arr[i++] = std::span<const std::byte>(fixed_vertex_bytes<T>);
            };
            (emplace_instanced_input.template operator()<Ts>(ret, idx), ...);
            return ret;
        }();


        RESULT_VERIFY_UNSCOPED(ret.stage(static_buffer_size, inputs), s);
        auto [staging_buffer, staging_mem] = *std::move(s);
        RESULT_VERIFY((ret.template alloc_buffer<0, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.static_data_buff), 1}, static_buffer_size, ret.static_device_local_mem
        )));
        RESULT_VERIFY((ret.template staging_to_device_local<0>(std::span{std::addressof(ret.static_data_buff), 1}, staging_buffer)));
        }

    create_uniform:
        //If theres no uniform data, just return
        constexpr static std::size_t uniform_buffer_size = (fixed_size<Ts>[buffer_data_type::uniform] + ...);
        if constexpr (uniform_buffer_size == 0) return ret;

        //Create uniform buffer
        RESULT_VERIFY((ret.template alloc_buffer<0, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0>(
            std::span<buffer, 1>{std::addressof(ret.uniform_buff), 1}, uniform_buffer_size, ret.host_mem
        )));

        RESULT_TRY_COPY(ret.uniform_buffer_map, ret.host_mem.map(logi_device, uniform_buffer_size));

        __D2D_TRY_MAKE(ret.desc_pool, (make<descriptor_pool<FiF, count_v[impl::type_filter::has_uniform]>>(logi_device)), dp);

        //Create pipelines and descriptors
        ((ret.create_pipelines<Ts>(logi_device, window_render_pass)), ...);

        return ret;
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    result<void> renderable_buffer<FiF, Ts...>::apply() noexcept {
        constexpr static std::size_t I = index_v<T>[impl::type_filter::none];
        constexpr static std::size_t I_ni = index_v<T>[impl::type_filter::not_instanced];
        constexpr static std::size_t I_a = index_v<T>[impl::type_filter::has_attrib];
        if(std::get<std::vector<T>>(data).size() == 0) {
            data_buffs[I] = buffer{};
            return error::invalid_argument;
        }

        //Fill input data vector
        std::size_t data_buffer_size = 0;
        std::vector<std::vector<std::byte>> inputs;
        vertex_offsets = {};
        inputs.reserve(std::get<std::vector<T>>(data).size() * 2);
        if constexpr (!T::instanced) {
            non_instanced_counts[I_ni].reserve(std::get<std::vector<T>>(data).size());
            index_firsts[I_ni].reserve(std::get<std::vector<T>>(data).size());
            vertex_firsts[I_ni].reserve(std::get<std::vector<T>>(data).size());
        }
        if constexpr (!T::instanced && impl::has_indices_v<T>) {
            for(std::size_t i = 0; i < std::get<std::vector<T>>(data).size(); ++i) {
                const auto input = std::get<std::vector<T>>(data)[i].indices();
                std::byte const* input_begin = reinterpret_cast<std::byte const*>(input.data());
                inputs.emplace_back(input_begin, input_begin + (input.size() * (sizeof input[0])));
                data_buffer_size += input.size() * (sizeof input[0]);
                index_firsts[I_ni].push_back(std::accumulate(non_instanced_counts[I_ni].begin(), non_instanced_counts[I_ni].end(), 0));
                non_instanced_counts[I_ni].push_back(input.size());
            }
            vertex_offsets[I_ni] = data_buffer_size;
        }
        if constexpr (!T::instanced && impl::has_vertices_v<T>){
            std::vector<std::uint32_t> vertex_counts{};
            std::vector<std::uint32_t>& count_vec = [&]() noexcept -> std::vector<std::uint32_t>& { 
                if constexpr(impl::has_indices_v<T>) return vertex_counts; 
                else return non_instanced_counts[I_ni]; 
            }();

            for(std::size_t i = 0; i < std::get<std::vector<T>>(data).size(); ++i) {
                const auto input = std::get<std::vector<T>>(data)[i].vertices();
                std::byte const* input_begin = reinterpret_cast<std::byte const*>(input.data());
                inputs.emplace_back(input_begin, input_begin + (input.size() * (sizeof input[0])));
                data_buffer_size += input.size() * (sizeof input[0]);
                vertex_firsts[I_ni].push_back(std::accumulate(count_vec.cbegin(), count_vec.cend(), 0));
                count_vec.push_back(input.size());
            }
        }

        if constexpr (T::instanced) {
            for(std::size_t i = 0; i < std::get<std::vector<T>>(data).size(); ++i) {
                const auto input = std::get<std::vector<T>>(data)[i].instance();
                std::byte const* input_begin = reinterpret_cast<std::byte const*>(&input);
                inputs.emplace_back(input_begin, input_begin + sizeof input);
                data_buffer_size += sizeof input;
            }
        }

        constexpr static VkBufferUsageFlags usage_flags = ((T::instanced || impl::has_vertices_v<T>) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0) | (impl::has_indices_v<T> ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0);
        

        std::vector<std::span<const std::byte>> input_spans(inputs.size());
        for(std::size_t i = 0; i < inputs.size(); ++i)
            input_spans[i] = {inputs[i].data(), inputs[i].size()};


        RESULT_VERIFY_UNSCOPED(stage(data_buffer_size, input_spans), s);
        auto [staging_buffer, staging_mem] = *std::move(s);
        if(data_buffer_size > data_buffs[I].size()) 
            RESULT_VERIFY((alloc_buffer<I, usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0>(std::span{data_buffs}, data_buffer_size, device_local_mem)));
        RESULT_VERIFY((staging_to_device_local<I>(std::span{data_buffs}, staging_buffer)));

        
        if constexpr (!impl::has_attributes_v<T>) {
            outdated[I] = false;
            return {};
        }

        constexpr static std::size_t total_attribute_size = fixed_size<T>[buffer_data_type::attribute];
        const std::size_t attribute_buffer_size = total_attribute_size * std::get<std::vector<T>>(data).size();
        if(attribute_buffer_size == 0) {
            attribute_buffs[I_a] = buffer{};
            return {};
        }

        if(attribute_buffer_size > attribute_buffs[I_a].size()) {
            RESULT_VERIFY((alloc_buffer<I, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT>(
                std::span{attribute_buffs}, attribute_buffer_size, shared_mem
            )));
        }

        //Must recreate ALL attribute span maps
        RESULT_TRY_COPY_UNSCOPED(void* shared_mem_map, shared_mem.map(*logi_device_ptr, VK_WHOLE_SIZE), smm);
        std::size_t buffer_offset = 0;

        auto emplace_all_attributes = [this, &buffer_offset, shared_mem_map]<typename U>(){
            if constexpr (!impl::has_attributes_v<U>) return;
            constexpr static std::size_t I_u = index_v<U>[impl::type_filter::has_attrib];
            const std::size_t curr_attrib_buff_size = attribute_buffs[I_u].size(); 
            std::span<std::byte>& attributes_span = std::get<I_u>(attributes_map_tuple); //tuple<span<T0_bytes>, span<T1_bytes>>[x] => span<Tx_bytes>
            attributes_span = std::span<std::byte>(static_cast<std::byte*>(shared_mem_map) + buffer_offset, curr_attrib_buff_size);
            buffer_offset += (curr_attrib_buff_size + shared_mem.requirements()[I_u].alignment - 1) & ~(shared_mem.requirements()[I_u].alignment - 1);;

            constexpr static std::array<std::size_t, num_attributes<U>> attribute_sizes = [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::array<std::size_t, num_attributes<U>>{sizeof(typename std::remove_reference_t<decltype(std::get<I>(std::declval<U>().attributes()))>::value_type)...};
            }(std::make_index_sequence<num_attributes<U>>{});
            static_assert(std::accumulate(attribute_sizes.cbegin(), attribute_sizes.cend(), 0) == total_attribute_size);
            constexpr static std::array<std::size_t, num_attributes<U>> attribute_offsets = []() {
                std::array<std::size_t, num_attributes<U>> ret;
                std::exclusive_scan(attribute_sizes.cbegin(), attribute_sizes.cend(), ret.begin(), 0);
                return ret;
            }();

            [&]<std::size_t... I>(std::index_sequence<I...>) {
                (emplace_attribute<I, U>(attributes_span, attribute_offsets, total_attribute_size), ...);
            }(std::make_index_sequence<num_attributes<U>>{});
        };
        (emplace_all_attributes.template operator()<Ts>(), ...);

        outdated[I] = false;
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
    template<typename InputContainerT>
    result<std::pair<buffer, device_memory<1>>> renderable_buffer<FiF, Ts...>::stage(std::size_t total_buffer_size, InputContainerT&& inputs) noexcept {
        std::array<buffer, 1> staging_buffs;
        RESULT_TRY_MOVE(staging_buffs[0], make<buffer>(*logi_device_ptr, total_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
        RESULT_TRY_MOVE_UNSCOPED(device_memory<1> staging_mem, 
            make<device_memory<1>>(*logi_device_ptr, *phys_device_ptr, staging_buffs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), sm);
        staging_mem.bind(*logi_device_ptr, staging_buffs[0], 0);

        //Fill staging buffer with data
        RESULT_TRY_COPY_UNSCOPED(void* mapped_data, staging_mem.map(*logi_device_ptr, total_buffer_size), smm);
        std::byte* data_ptr = static_cast<std::byte*>(mapped_data); 
        for(std::size_t i = 0; i < std::forward<InputContainerT>(inputs).size(); ++i) {
            std::memcpy(data_ptr, std::forward<InputContainerT>(inputs)[i].data(), std::forward<InputContainerT>(inputs)[i].size_bytes());
            data_ptr += std::forward<InputContainerT>(inputs)[i].size_bytes();
        }
        staging_mem.unmap(*logi_device_ptr);
        return std::pair{std::move(staging_buffs[0]), std::move(staging_mem)};
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<std::size_t I, VkFlags BufferUsage, VkMemoryPropertyFlags MemProps, VkMemoryPropertyFlags FallbackMemProps, std::size_t N>
    result<void> renderable_buffer<FiF, Ts...>::alloc_buffer(std::span<buffer, N> buffs, std::size_t total_buffer_size, device_memory<N>& mem) noexcept {
        //Create the device local buffer
        RESULT_TRY_MOVE(buffs[I], make<buffer>(*logi_device_ptr, total_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | BufferUsage));

        //Re-allocate the memory
        device_memory<N> old_mem = std::move(mem); //Make sure old memory used for the rest of the buffers stays alive until after bind
        auto m = make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, MemProps);
        if(m.has_value()) mem = *std::move(m);
        else if(FallbackMemProps != 0 && m.error() == error::device_lacks_suitable_mem_type) {
            RESULT_TRY_MOVE_UNSCOPED(mem, make<device_memory<N>>(*logi_device_ptr, *phys_device_ptr, buffs, FallbackMemProps), r)
        }
        else return m.error();
        
        //Create copy command buffer
        RESULT_TRY_MOVE_UNSCOPED(command_buffer copy_cmd_buffer, (make<command_buffer>(*logi_device_ptr, copy_cmd_pool)), ccb);
        RESULT_VERIFY(copy_cmd_buffer.copy_begin());

        //Re-create all other buffers
        std::array<buffer, N> new_buffs = {};
        std::size_t mem_offset = 0;
        for(std::size_t i = 0; i < N; ++i) {
            if(buffs[i].empty()) continue;
            
            //Create a new buffer with the same properties as the old one
            RESULT_TRY_MOVE(new_buffs[i], buffs[i].clone(*logi_device_ptr));
            
            //Bind the newly allocated memory to this new buffer
            mem.bind(*logi_device_ptr, new_buffs[i], mem_offset);
            mem_offset += (new_buffs[i].size() + mem.requirements()[i].alignment - 1) & ~(mem.requirements()[i].alignment - 1);

            //Copy the data from all other old buffers into the new ones
            if(i == I) continue;
            switch(new_buffs[i].type()){
            case buffer_type::generic:
                copy_cmd_buffer.copy_generic(new_buffs[i], buffs[i], new_buffs[i].size());
                break;
            case buffer_type::image:
                copy_cmd_buffer.copy_image(new_buffs[i], buffs[i], new_buffs[i].image_size());
                break;
            }
        }

        RESULT_VERIFY(copy_cmd_buffer.copy_end(*logi_device_ptr, copy_cmd_pool));

        //Replace the old buffer with the new one
        for(std::size_t i = 0; i < N; ++i)
            if(!buffs[i].empty())
                buffs[i] = std::move(new_buffs[i]);

        return {};
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<std::size_t I, std::size_t N>
    result<void> renderable_buffer<FiF, Ts...>::staging_to_device_local(std::span<buffer, N> device_local_buffs, buffer const& staging_buff) noexcept {
        //Create copy command buffer
        RESULT_TRY_MOVE_UNSCOPED(command_buffer copy_cmd_buffer, (make<command_buffer>(*logi_device_ptr, copy_cmd_pool)), ccb);
        RESULT_VERIFY(copy_cmd_buffer.copy_begin());
        
        //Copy from staging buffer to current device local buffer
        switch(device_local_buffs[I].type()){
        case buffer_type::generic:
            copy_cmd_buffer.copy_generic(device_local_buffs[I], staging_buff, staging_buff.size());
            break;
        case buffer_type::image:
            copy_cmd_buffer.copy_image(device_local_buffs[I], staging_buff, staging_buff.image_size());
            break;
        }


        RESULT_VERIFY(copy_cmd_buffer.copy_end(*logi_device_ptr, copy_cmd_pool));
        return {};
    }
}


namespace d2d {
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
            __D2D_TRY_MAKE(desc_sets[I], (make<descriptor_set<FiF>>(logi_device, desc_pool, desc_set_layouts[I], uniform_buff, fixed_size<T>[buffer_data_type::uniform], static_offsets<T>()[buffer_data_type::uniform])), ds);
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
    //template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    //template<typename T, std::size_t N>
    //constexpr void renderable_buffer<FiF, Ts...>::emplace_instanced_input(std::array<std::span<const std::byte>, N>& input_arr, std::size_t& idx) noexcept {
    //    if constexpr (T::instanced && impl::has_fixed_indices_v<T>) {
    //        constexpr static std::array index_bytes = std::bit_cast<std::array<std::byte, fixed_size<T>[buffer_data_type::index]>>(T::indices());
    //        input_arr[idx++] = std::span<const std::byte>{index_bytes};
    //    }
    //    if constexpr (T::instanced && impl::has_fixed_vertices_v<T>) {
    //        constexpr static std::array vertex_bytes = std::bit_cast<std::array<std::byte, fixed_size<T>[buffer_data_type::vertex]>>(T::vertices());
    //        input_arr[idx++] = std::span<const std::byte>{vertex_bytes};
    //    }
    //}
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<std::size_t I, typename T>
    constexpr void renderable_buffer<FiF, Ts...>::emplace_attribute(std::span<std::byte> attributes_span, std::array<std::size_t, num_attributes<T>> attribute_offsets, std::size_t attribute_size) noexcept {
        using attribute_ref_type = decltype(std::get<I>(std::declval<T>().attributes())); //attribute<V>&
        using attribute_type = std::remove_reference_t<attribute_ref_type>; //attribute<V>
        using attribute_value_type = typename attribute_type::value_type; //V
        static_assert(std::is_lvalue_reference_v<attribute_ref_type> && !std::is_const_v<attribute_type>);
        
        for(std::size_t i = 0; i < std::get<std::vector<T>>(data).size(); ++i){
            attribute_ref_type attribute = std::get<I>(std::get<std::vector<T>>(data)[i].attributes()); //tuple<attribute<Tx_Ai>&, attribute<Tx_Bi>&, ..., attribute<Tx_Ni&>[I] => attribute<Tx_Ii>&
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
        else return data_buffs[index_v<T>[impl::type_filter::none]]; 
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
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::index_count(std::size_t i) const noexcept requires (!T::instanced && impl::has_indices_v<T>) {
        return non_instanced_counts[index_v<T>[impl::type_filter::not_instanced]][i];
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::vertex_count(std::size_t i) const noexcept requires (!T::instanced && impl::has_vertices_v<T>) {
        return non_instanced_counts[index_v<T>[impl::type_filter::not_instanced]][i];
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::instance_count() const noexcept { 
        return std::get<std::vector<T>>(data).size();
    }


    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::first_index(std::size_t i) const noexcept requires (!T::instanced && impl::has_indices_v<T>) {
        return index_firsts[index_v<T>[impl::type_filter::has_index]][i];
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::uint32_t renderable_buffer<FiF, Ts...>::first_vertex(std::size_t i) const noexcept requires (!T::instanced && impl::has_vertices_v<T>) {
        return vertex_firsts[index_v<T>[impl::type_filter::not_instanced]][i];
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T>
    constexpr std::size_t renderable_buffer<FiF, Ts...>::vertex_buffer_offset() const noexcept requires (!T::instanced && impl::has_vertices_v<T>) {
        return vertex_offsets[index_v<T>[impl::type_filter::not_instanced]];
    }

    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    consteval buffer_bytes_t renderable_buffer<FiF, Ts...>::static_offsets() noexcept { 
        std::array<buffer_bytes_t, count_v[impl::type_filter::none]> buff_offsets;
        constexpr std::array<buffer_bytes_t, count_v[impl::type_filter::none]> static_sizes = {
            (Ts::instanced ? fixed_size<Ts> : buffer_bytes_t{0, fixed_size<Ts>[buffer_data_type::uniform], 0, 0, fixed_size<Ts>[buffer_data_type::attribute]})...
        };
        
        std::exclusive_scan(static_sizes.cbegin(), static_sizes.cend(), buff_offsets.begin(), buffer_bytes_t{},
            [](buffer_bytes_t acc, const buffer_bytes_t& prev_size){ return buffer_bytes_t{
                acc[buffer_data_type::vertex] + prev_size[buffer_data_type::vertex] + prev_size[buffer_data_type::index],
                acc[buffer_data_type::uniform] + prev_size[buffer_data_type::uniform],
                acc[buffer_data_type::vertex] + prev_size[buffer_data_type::vertex] + prev_size[buffer_data_type::index],
                0,
                acc[buffer_data_type::attribute] + prev_size[buffer_data_type::attribute],
            };});
        for(std::size_t i = 0; i < count_v[impl::type_filter::none]; ++i)
            buff_offsets[i][buffer_data_type::vertex] += static_sizes[i][buffer_data_type::index];
        return buff_offsets[index_v<T>[impl::type_filter::none]];
    }
}

namespace d2d {
    template<std::size_t FiF, impl::RenderableType... Ts> requires (sizeof...(Ts) > 0)
    template<typename T> 
    constexpr std::span<typename T::uniform_type> renderable_buffer<FiF, Ts...>::uniform_map() const noexcept requires impl::has_uniform_v<T> { 
        return {reinterpret_cast<typename T::uniform_type*>(reinterpret_cast<std::uintptr_t>(uniform_buffer_map) + static_offsets<T>()[buffer_data_type::uniform]), FiF}; 
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