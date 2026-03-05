#pragma once
#include "Duo2D/core/buffer_config_table.hpp"
#include "Duo2D/core/asset_heap_config_table.hpp"

namespace d2d {
    template<typename TimelineT, auto BufferConfigs, auto AssetHeapConfigs>
	requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    class application_instance;
}
