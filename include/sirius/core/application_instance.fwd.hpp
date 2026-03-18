#pragma once
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"

namespace acma {
    template<typename TimelineT, auto BufferConfigs, auto AssetHeapConfigs>
	requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    class application_instance;
}
