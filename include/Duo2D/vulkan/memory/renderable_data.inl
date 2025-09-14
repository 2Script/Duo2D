#pragma once
#include "Duo2D/arith/size.hpp"
#include "Duo2D/vulkan/display/texture.hpp"
#include "Duo2D/vulkan/device/logical_device.hpp"
#include "Duo2D/vulkan/memory/descriptor_pool.hpp"
#include "Duo2D/vulkan/memory/renderable_allocator.hpp"
#include "Duo2D/vulkan/memory/renderable_data.hpp"
#include "Duo2D/vulkan/memory/pipeline.hpp"
#include <cstdint>
#include <limits>
#include <numeric>
#include <result.hpp>
#include <result/verify.h>
#include <string_view>
#include <vulkan/vulkan_core.h>


namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T>
    constexpr renderable_input_data<T>::iterator renderable_input_data<T>::erase(const_iterator pos) noexcept {
        outdated = true;
        return renderable_input_map<T>::erase(pos);
    }

    template<::d2d::impl::directly_renderable T>
    constexpr renderable_input_data<T>::iterator renderable_input_data<T>::erase(const_iterator first, const_iterator last) noexcept {
        outdated = true;
        return renderable_input_map<T>::erase(first, last);
    }

    
    template<::d2d::impl::directly_renderable T>
    std::size_t renderable_input_data<T>::erase(key_type const& key) noexcept {
        const std::size_t erased_count = renderable_input_map<T>::erase(key);
        if(erased_count) outdated = true;
        return erased_count;
    }
}


namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T>
    result<void> renderable_input_data<T>::set_hidden(key_type const& key, bool value) noexcept {
        if(this->find(key) == this->end()) return errc::element_not_found;
        hidden_state.insert_or_assign(key, value);
        return {};
    }

    template<::d2d::impl::directly_renderable T>
    result<void> renderable_input_data<T>::toggle_hidden(key_type const& key) noexcept {
        if(this->find(key) == this->end()) return errc::element_not_found;
        auto emplace_result = hidden_state.try_emplace(key);
        emplace_result.first->second = !emplace_result.first->second;
        return {};
    }


    template<::d2d::impl::directly_renderable T>
    bool renderable_input_data<T>::shown(key_type const& key) noexcept {
        return !hidden_state.try_emplace(key).first->second;
    }

    template<::d2d::impl::directly_renderable T>
    bool renderable_input_data<T>::hidden(key_type const& key) noexcept {
        return hidden_state.try_emplace(key).first->second;
    }
}



namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_attributes
    template<std::size_t I>
    void renderable_attribute_data<T>::emplace_single_attribute(std::array<std::size_t, renderable_attribute_data<T>::num_attributes> attribute_offsets) noexcept {
        using attribute_ref_type = decltype(std::get<I>(std::declval<T>().attributes())); //attribute<V>&
        using attribute_type = std::remove_reference_t<attribute_ref_type>; //attribute<V>
        using attribute_value_type = typename attribute_type::value_type; //V
        static_assert(std::is_lvalue_reference_v<attribute_ref_type> && !std::is_const_v<attribute_type>);
        
        //Old values must be retrieved before ANY modifications are done so that attributes with existing references arent overwritten
        std::vector<attribute_value_type> old_values;
        old_values.reserve(this->size());
        for(auto input_renderable_pair : *this)
            old_values.push_back(std::get<I>(input_renderable_pair.second.attributes()).get_ref());

        std::size_t i = 0;
        for(auto iter = this->begin(); iter != this->end(); ++iter, ++i){
            attribute_ref_type attribute = std::get<I>(iter->second.attributes());
            attribute = attribute_type(reinterpret_cast<attribute_value_type*>(&attributes_span[(attribute_data_size * i) + attribute_offsets[I]]));
            attribute.get_ref() = old_values[i];
        }
    }

    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_attributes
    void renderable_attribute_data<T>::emplace_attributes(std::size_t buff_offset, void* mem_map) noexcept {
        attributes_span = std::span<std::byte>(static_cast<std::byte*>(mem_map) + buff_offset, attribute_buffer_size());

        constexpr static std::array<std::size_t, num_attributes> attribute_sizes = impl::attribute_traits<typename renderable_traits<T>::attribute_types>::sizes;
        constexpr static std::array<std::size_t, num_attributes> attribute_offsets = []() {
            std::array<std::size_t, num_attributes> ret;
            std::exclusive_scan(attribute_sizes.cbegin(), attribute_sizes.cend(), ret.begin(), 0);
            return ret;
        }();
        static_assert(std::accumulate(attribute_sizes.cbegin(), attribute_sizes.cend(), 0) == attribute_data_size);

        [this]<std::size_t... I>(std::index_sequence<I...>) {
            (this->emplace_single_attribute<I>(attribute_offsets), ...);
        }(std::make_index_sequence<num_attributes>{});
    }
}

namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_attributes
    template<std::size_t I>
    void renderable_attribute_data<T>::unbind_single_attribute() noexcept {
        using attribute_ref_type = decltype(std::get<I>(std::declval<T>().attributes())); //attribute<V>&
        using attribute_type = std::remove_reference_t<attribute_ref_type>; //attribute<V>
        static_assert(std::is_lvalue_reference_v<attribute_ref_type> && !std::is_const_v<attribute_type>);

        for(auto iter = this->begin(); iter != this->end(); ++iter){
            attribute_ref_type attribute = std::get<I>(iter->second.attributes());
            attribute = attribute_type(attribute.get_ref());
        }
    }

    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_attributes
    void renderable_attribute_data<T>::unbind_attributes() noexcept {
        [this]<std::size_t... I>(std::index_sequence<I...>) {
            (this->unbind_single_attribute<I>(), ...);
        }(std::make_index_sequence<num_attributes>{});
    }
}



namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T>
    template<typename LoadTextureFn>
    result<std::vector<std::span<const std::byte>>> renderable_instance_data<T>::make_inputs(LoadTextureFn&&) noexcept {
        input_data_size = 0;
        return std::vector<std::span<const std::byte>>{}; 
    }


    template<::d2d::impl::directly_renderable T> requires renderable_constraints<T>::has_instance_data
    template<typename LoadTextureFn>
    result<std::vector<std::span<const std::byte>>> renderable_instance_data<T>::make_inputs(LoadTextureFn&&) noexcept {
        //if(this->size() == 0) return error::invalid_argument;

        instance_inputs.reserve(this->size());
        input_data_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->size());
        for(const auto& input_pair : *this) {
            instance_inputs.push_back(input_pair.second.instance());
            
            std::byte const* input_begin = reinterpret_cast<std::byte const*>(&instance_inputs.back());
            input_bytes.emplace_back(input_begin, input_begin + sizeof(instance_input_type));
            input_data_size += sizeof(instance_input_type);
        }
        return std::move(input_bytes);
    }


    template<::d2d::impl::directly_renderable T> requires (!renderable_constraints<T>::instanced)
    template<typename LoadTextureFn>
    result<std::vector<std::span<const std::byte>>> renderable_instance_data<T>::make_inputs(LoadTextureFn&&) noexcept {
        //if(this->size() == 0) return error::invalid_argument;

        vertex_inputs.reserve(this->size());
        input_data_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->size());
        for(const auto& input_pair : *this) {
            vertex_inputs.push_back(input_pair.second.vertices());
            if constexpr(::d2d::impl::directly_renderable_file<T>) input_pair.second.unload();
            
            const auto& input = vertex_inputs.back();
            std::byte const* input_begin = reinterpret_cast<std::byte const*>(input.data());
            input_bytes.emplace_back(input_begin, input_begin + (input.size() * (sizeof input[0])));
            input_data_size += (input.size() * (sizeof input[0]));
            
            vertex_firsts.push_back(std::accumulate(vertex_counts.cbegin(), vertex_counts.cend(), 0));
            vertex_counts.push_back(input.size());
        }
        return std::move(input_bytes);
    }


    template<::d2d::impl::directly_renderable T> requires (!renderable_constraints<T>::instanced && renderable_constraints<T>::has_indices)
    template<typename LoadTextureFn>
    result<std::vector<std::span<const std::byte>>> renderable_index_data<T>::make_inputs(LoadTextureFn&& _) noexcept {
        //if(this->size() == 0) return error::invalid_argument;

        index_inputs.reserve(this->size());
        std::size_t index_input_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->size() * 2);
        for(const auto& input_pair : *this) {
            index_inputs.push_back(input_pair.second.indices());
            
            const auto& input = index_inputs.back();
            std::byte const* input_begin = reinterpret_cast<std::byte const*>(input.data());
            input_bytes.emplace_back(input_begin, input_begin + (input.size() * (sizeof input[0])));
            index_input_size += (input.size() * (sizeof input[0]));
            index_firsts.push_back(std::accumulate(index_counts.begin(), index_counts.end(), 0));
            index_counts.push_back(input.size());
        }
        this->vertex_offset = index_input_size;

        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> vertex_input, renderable_instance_data<T>::make_inputs(std::forward<LoadTextureFn>(_)), vi);
        this->input_data_size += index_input_size;
        input_bytes.insert(input_bytes.end(), vertex_input.cbegin(), vertex_input.cend());
        return std::move(input_bytes);
    }


    template<::d2d::impl::directly_renderable T> requires (renderable_constraints<T>::has_textures)
    template<typename LoadTextureFn>
    result<std::vector<std::span<const std::byte>>> renderable_texture_data<T>::make_inputs(LoadTextureFn&& load_texture_fn) noexcept {
        //if(this->size() == 0) return error::invalid_argument;

        RESULT_VERIFY_UNSCOPED(make_texture_indices(std::forward<LoadTextureFn>(load_texture_fn)), mtg);
        auto [texture_idx_input_size, input_bytes] = *std::move(mtg);

        RESULT_TRY_MOVE_UNSCOPED(std::vector<std::span<const std::byte>> vertex_and_index_input, renderable_index_data<T>::make_inputs(std::forward<LoadTextureFn>(load_texture_fn)), ii);
        texture_idx_offset = this->input_data_size;
        this->input_data_size += texture_idx_input_size;
        input_bytes.insert(input_bytes.begin(), vertex_and_index_input.cbegin(), vertex_and_index_input.cend());
        return std::move(input_bytes);
    }
}

namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T> requires (renderable_constraints<T>::has_textures)
    template<typename LoadTextureFn>
    result<std::pair<std::size_t, std::vector<std::span<const std::byte>>>> renderable_texture_data<T>::make_texture_indices(LoadTextureFn&& load_texture_fn) noexcept {
        texture_idx_inputs.reserve(this->size());
        std::size_t texture_idx_input_size = 0;

        std::vector<std::span<const std::byte>> input_bytes;
        input_bytes.reserve(this->size());
        for(const auto& input_pair : *this) {
            for(std::size_t i = 0; i < renderable_traits<T>::max_texture_count; ++i) {
                const auto& key = input_pair.second.texture_keys()[i];
                if(key.empty()) continue;
                RESULT_VERIFY(std::forward<LoadTextureFn>(load_texture_fn)(key));
            }
            std::array<texture_idx_t, renderable_traits<T>::max_texture_count> texture_idxs;
            for(std::size_t i = 0; i < renderable_traits<T>::max_texture_count; ++i) {
                const auto& key = input_pair.second.texture_keys()[i];
                if(key.empty()) {
                    texture_idxs[i] = std::numeric_limits<texture_idx_t>::max();
                    continue;
                }
                RESULT_TRY_COPY(texture_idxs[i], std::forward<LoadTextureFn>(load_texture_fn)(key));
            }
            texture_idx_inputs.push_back(texture_idxs);
            
            const std::array<texture_idx_t, renderable_traits<T>::max_texture_count>& input = texture_idx_inputs.back();
            input_bytes.emplace_back(reinterpret_cast<std::byte const*>(input.data()), (sizeof(texture_idx_t) * (renderable_traits<T>::max_texture_count)));
            texture_idx_input_size += (sizeof(texture_idx_t) * (renderable_traits<T>::max_texture_count));
        }

        return std::pair{texture_idx_input_size, std::move(input_bytes)};
    }


    template<::d2d::impl::directly_renderable T> requires (renderable_constraints<T>::has_textures)
    template<typename SkipT, typename LoadTextureFn>
    result<void> renderable_texture_data<T>::update_texture_indices(LoadTextureFn&& load_texture_fn, renderable_allocator& allocator, buffer& data_buffer) noexcept {
        if constexpr(std::is_same_v<T, SkipT>) return {};
        if(data_buffer.empty()) return {};


        RESULT_VERIFY_UNSCOPED(make_texture_indices(std::forward<LoadTextureFn>(load_texture_fn)), mtg);
        auto [texture_idx_input_size, input_bytes] = *std::move(mtg);

        std::size_t original_data_size = data_buffer.size();
        std::size_t new_data_offset =  original_data_size - texture_idx_input_size;
        RESULT_VERIFY_UNSCOPED(allocator.stage(original_data_size, input_bytes, new_data_offset), stg);
        auto [staging_buffer, staging_mem] = *std::move(stg);
        RESULT_VERIFY(allocator.staging_to_device_local(data_buffer, staging_buffer, new_data_offset, texture_idx_input_size));
        return {};
    }
}


namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T, std::size_t FiF> requires (renderable_constraints<T>::has_uniform || renderable_constraints<T>::has_textures)
    result<void> renderable_descriptor_data<T, FiF>::create_uniform_descriptors(buffer& uniform_buff, std::size_t uniform_buff_offset) noexcept {
        if constexpr(!renderable_constraints<T>::has_uniform) return {};
        
        valid_descriptors[uniform_binding] = true;

        pool_sizes[uniform_binding] = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = FiF
        };

        set_layout_bindings[uniform_binding] = {
            .binding = uniform_binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        };

        set_layout_flags[uniform_binding] = 0;

        const std::size_t uniform_size = uniform_data_size / FiF;
        for(std::size_t i = 0; i < FiF; ++i) {
            uniform_buffer_infos[i] = {
                .buffer = static_cast<VkBuffer>(uniform_buff),
                .offset = uniform_buff_offset + (i * uniform_size),
                .range = uniform_size,
            };
        }

        return {};
    }


    template<::d2d::impl::directly_renderable T, std::size_t FiF> requires (renderable_constraints<T>::has_uniform || renderable_constraints<T>::has_textures)
    result<void> renderable_descriptor_data<T, FiF>::create_texture_descriptors(texture_map& textures) noexcept {
        if constexpr(!renderable_constraints<T>::has_textures) return {};
        const std::uint32_t descriptor_count = std::max<std::uint32_t>(1, textures.size());

        valid_descriptors[texture_binding] = true;

        pool_sizes[texture_binding] = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<std::uint32_t>(FiF * descriptor_count),
        };

        set_layout_bindings[texture_binding] = {
            .binding = texture_binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = descriptor_count,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        };

        set_layout_flags[texture_binding] = 0;//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        image_infos.clear();
        image_infos.reserve(textures.size());
        for(auto iter = textures.cbegin(); iter != textures.cend(); ++iter) {
            image_infos.push_back({
                .sampler = iter->second.sampler(),
                .imageView = iter->second.view(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            });
        }


        return {};
    }
}


namespace d2d::vk::impl {
    template<::d2d::impl::directly_renderable T, std::size_t FiF>
    result<void> renderable_descriptor_data<T, FiF>::create_pipeline_layout(std::shared_ptr<logical_device> logi_device) noexcept {
        RESULT_TRY_MOVE(pl_layout, make<pipeline_layout<T>>(logi_device));
        return {};
    }
    template<::d2d::impl::directly_renderable T, std::size_t FiF> requires (renderable_constraints<T>::has_uniform || renderable_constraints<T>::has_textures)
    result<void> renderable_descriptor_data<T, FiF>::create_pipeline_layout(std::shared_ptr<logical_device> logi_device) noexcept {
        RESULT_TRY_MOVE(pool, make<descriptor_pool>(logi_device, std::span{pool_sizes}, FiF, valid_descriptors));
        RESULT_TRY_MOVE(set_layout, make<descriptor_set_layout>(logi_device, std::span{set_layout_bindings}, std::span{set_layout_flags}, valid_descriptors));
        
        std::array<VkDescriptorSetLayout, FiF> layouts;
        layouts.fill(static_cast<VkDescriptorSetLayout>(set_layout));
        //std::array<std::uint32_t, FiF> variable_counts;
        //variable_counts.fill(image_infos.size());

        //VkDescriptorSetVariableDescriptorCountAllocateInfo variable_alloc_info {
        //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        //    .pNext = nullptr,
        //    .descriptorSetCount = variable_counts.size(),
        //    .pDescriptorCounts = variable_counts.data(),
        //};

        VkDescriptorSetAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            //.pNext = &variable_alloc_info,
            .descriptorPool = pool,
            .descriptorSetCount = layouts.size(),
            .pSetLayouts = layouts.data(),
        };



        __D2D_VULKAN_VERIFY(vkAllocateDescriptorSets(*logi_device, &alloc_info, sets.data()));

        for (size_t i = 0; i < FiF; i++) {
            std::vector<VkWriteDescriptorSet> writes;
            writes.reserve(renderable_constraints<T>::has_uniform + (renderable_constraints<T>::has_textures * image_infos.size()));

            if constexpr(renderable_constraints<T>::has_uniform) {
                writes.push_back({
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = sets[i],
                    .dstBinding = uniform_binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &uniform_buffer_infos[i],
                });
            }

            if constexpr(renderable_constraints<T>::has_textures) {
                for(std::size_t j = 0; j < image_infos.size(); ++j) {
                    writes.push_back({
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = sets[i],
                        .dstBinding = texture_binding,
                        .dstArrayElement = static_cast<std::uint32_t>(j),
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = &image_infos[j],
                    });
                }
            }

            vkUpdateDescriptorSets(*logi_device, writes.size(), writes.data(), 0, nullptr);
        }



        RESULT_TRY_MOVE(pl_layout, make<pipeline_layout<T>>(logi_device, set_layout));
        return {};
    }
}


namespace d2d::vk {
    template<::d2d::impl::directly_renderable T, std::size_t FiF>
    result<void> renderable_data<T, FiF>::create_pipeline(std::shared_ptr<logical_device> logi_device, render_pass& window_render_pass) noexcept {
        if(!this->pl_layout) return errc::descriptors_not_initialized;
        RESULT_TRY_MOVE(pl, make<pipeline<T>>(logi_device, window_render_pass, this->pl_layout));
        return  {};
    }
    
}