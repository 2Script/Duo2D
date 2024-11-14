#include "Duo2D/graphics/pipeline/shader_buffer.hpp"
#include "Duo2D/error.hpp"
#include "Duo2D/graphics/pipeline/command_buffer.hpp"
#include "Duo2D/graphics/pipeline/make.hpp"
#include <cstring>
#include "zstring.hpp"

namespace d2d {
    result<shader_buffer> shader_buffer::create(logical_device& logi_device, physical_device& phys_device, command_pool& pool, void const* data, std::size_t data_size, buffer_type type) noexcept {
        shader_buffer ret{};

        __D2D_TRY_MAKE(buffer staging_buffer, make<buffer>(logi_device, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), sb);
        __D2D_TRY_MAKE(device_memory staging_buffer_memory, 
            make<device_memory>(logi_device, phys_device, staging_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), sbm);

        void* mapped_data;
        vkMapMemory(logi_device, staging_buffer_memory, 0, data_size, 0, &mapped_data);
        std::memcpy(mapped_data, data, data_size);
        //zstring::memcpy(mapped_data, data, data_size);
        vkUnmapMemory(logi_device, staging_buffer_memory);

        __D2D_TRY_MAKE(ret._buffer, make<buffer>(logi_device, data_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | static_cast<std::size_t>(type)), b);
        __D2D_TRY_MAKE(ret._buffer_memory, 
            make<device_memory>(logi_device, phys_device, ret._buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), bm);

        __D2D_TRY_MAKE(command_buffer copy_cmd_buffer, make<command_buffer>(logi_device, pool), ccb);
        copy_cmd_buffer.copy(ret._buffer, staging_buffer, data_size);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &copy_cmd_buffer;
        vkQueueSubmit(logi_device.queues[queue_family::graphics], 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(logi_device.queues[queue_family::graphics]);

        vkFreeCommandBuffers(logi_device, pool, 1, &copy_cmd_buffer);
        return ret;
    }
}