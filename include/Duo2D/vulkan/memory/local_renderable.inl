#pragma once
#include "Duo2D/vulkan/memory/local_renderable.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include <result/verify.h>


namespace d2d::impl {
    template<RenderableType T> requires has_attributes_v<T>
    template<std::size_t I>
    void local_attributes<T>::emplace_single_attribute(std::array<std::size_t, local_attributes<T>::num_attributes> attribute_offsets) noexcept {
        using attribute_ref_type = decltype(std::get<I>(std::declval<T>().attributes())); //attribute<V>&
        using attribute_type = std::remove_reference_t<attribute_ref_type>; //attribute<V>
        using attribute_value_type = typename attribute_type::value_type; //V
        static_assert(std::is_lvalue_reference_v<attribute_ref_type> && !std::is_const_v<attribute_type>);
        
        for(std::size_t i = 0; i < data.size(); ++i){
            attribute_ref_type attribute = std::get<I>(data[i].attributes()); //tuple<attribute<Tx_Ai>&, attribute<Tx_Bi>&, ..., attribute<Tx_Ni&>[I] => attribute<Tx_Ii>&
            attribute_value_type old_value = attribute.get_ref(); //value = attribute<Tx_Ii>
            attribute = attribute_type(reinterpret_cast<attribute_value_type*>(&attributes_span[(attribute_data_size * i) + attribute_offsets[I]])); //attribute<Tx_Ii>& = attribute<Tx_Ii>(&span<Tx_I>[i])
            attribute.get_ref() = old_value;
        }
    }

    template<RenderableType T> requires has_attributes_v<T>
    std::size_t local_attributes<T>::emplace_attributes(std::size_t& buff_offset, void* mem_map, VkDeviceSize mem_align) noexcept {
        attributes_span = std::span<std::byte>(static_cast<std::byte*>(mem_map) + buff_offset, attribute_buffer_size());
        std::size_t old_offset = buff_offset;
        buff_offset += (attribute_buffer_size() + mem_align - 1) & ~(mem_align - 1);

        constexpr static std::array<std::size_t, num_attributes> attribute_sizes = impl::extract_attribute_sizes<typename T::attribute_types>::value;
        constexpr static std::array<std::size_t, num_attributes> attribute_offsets = []() {
            std::array<std::size_t, num_attributes> ret;
            std::exclusive_scan(attribute_sizes.cbegin(), attribute_sizes.cend(), ret.begin(), 0);
            return ret;
        }();
        static_assert(std::accumulate(attribute_sizes.cbegin(), attribute_sizes.cend(), 0) == attribute_data_size);

        [this]<std::size_t... I>(std::index_sequence<I...>) {
            (emplace_single_attribute<I>(attribute_offsets), ...);
        }(std::make_index_sequence<num_attributes>{});

        return old_offset;
    }
}


namespace d2d::impl {
    template<impl::RenderableType T>
    result<std::vector<std::span<const std::byte>>> local_instance<T>::make_inputs() noexcept {
        //if(this->data.size() == 0) return error::invalid_argument;

        instance_inputs.clear();
        instance_inputs.reserve(this->data.size());
        input_data_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->data.size());
        for(std::size_t i = 0; i < this->data.size(); ++i) {
            instance_inputs.push_back(this->data[i].instance());
            
            std::byte const* input_begin = reinterpret_cast<std::byte const*>(&instance_inputs.back());
            input_bytes.emplace_back(input_begin, input_begin + sizeof(instance_input_type));
            input_data_size += sizeof(instance_input_type);
        }
        return std::move(input_bytes);
    }

    template<impl::RenderableType T> requires (!T::instanced)
    result<std::vector<std::span<const std::byte>>> local_instance<T>::make_inputs() noexcept {
        //if(this->data.size() == 0) return error::invalid_argument;

        vertex_inputs.clear();
        vertex_inputs.reserve(this->data.size());
        input_data_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->data.size());
        for(std::size_t i = 0; i < this->data.size(); ++i) {
            vertex_inputs.push_back(this->data[i].vertices());
            
            const auto& input = vertex_inputs.back();
            std::byte const* input_begin = reinterpret_cast<std::byte const*>(input.data());
            input_bytes.emplace_back(input_begin, input_begin + (input.size() * (sizeof input[0])));
            input_data_size += (input.size() * (sizeof input[0]));
            
            vertex_firsts.push_back(std::accumulate(vertex_counts.cbegin(), vertex_counts.cend(), 0));
            vertex_counts.push_back(input.size());
        }
        return std::move(input_bytes);
    }


    template<impl::RenderableType T> requires (!T::instanced && has_indices_v<T>)
    result<std::vector<std::span<const std::byte>>> local_indices<T>::make_inputs() noexcept {
        //if(this->data.size() == 0) return error::invalid_argument;

        index_inputs.clear();
        index_inputs.reserve(this->data.size());
        std::size_t index_input_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->data.size() * 2);
        for(std::size_t i = 0; i < this->data.size(); ++i) {
            index_inputs.push_back(this->data[i].indices());
            
            const auto& input = index_inputs.back();
            std::byte const* input_begin = reinterpret_cast<std::byte const*>(input.data());
            input_bytes.emplace_back(input_begin, input_begin + (input.size() * (sizeof input[0])));
            index_input_size += (input.size() * (sizeof input[0]));
            
            index_firsts.push_back(std::accumulate(index_counts.begin(), index_counts.end(), 0));
            index_counts.push_back(input.size());
        }
        this->vertex_offset = index_input_size;

        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> vertex_input, local_instance<T>::make_inputs(), vi);
        this->input_data_size += index_input_size;
        input_bytes.insert(input_bytes.end(), vertex_input.cbegin(), vertex_input.cend());
        return std::move(input_bytes);
    }   
}


namespace d2d::impl {
    template<RenderableType T, std::size_t FiF>
    result<void> local_uniform<T, FiF>::create_descriptors(logical_device& logi_device, buffer&, descriptor_pool<FiF>&, std::size_t) noexcept {
        RESULT_TRY_MOVE(pl_layout, make<pipeline_layout<T>>(logi_device));
        return {};
    }

    template<RenderableType T, std::size_t FiF> requires has_uniform_v<T>
    result<void> local_uniform<T, FiF>::create_descriptors(logical_device& logi_device, buffer& uniform_buff, descriptor_pool<FiF>& desc_pool, std::size_t uniform_buff_offset) noexcept {
        RESULT_TRY_MOVE(set_layout, make<descriptor_set_layout>(logi_device));
        RESULT_TRY_MOVE(pl_layout, make<pipeline_layout<T>>(logi_device, set_layout));
        RESULT_TRY_MOVE(set, (make<descriptor_set<FiF>>(logi_device, desc_pool, set_layout, uniform_buff, uniform_data_size, uniform_buff_offset)));
        return {};
    }
}


namespace d2d {
    template<impl::RenderableType T, std::size_t FiF>
    result<void> local_renderable<T, FiF>::create_pipeline(logical_device& logi_device, render_pass& window_render_pass) noexcept {
        if(!this->pl_layout) return errc::descriptors_not_initialized;
        RESULT_TRY_MOVE(pl, make<pipeline<T>>(logi_device, window_render_pass, this->pl_layout));
        return  {};
    }
    
}