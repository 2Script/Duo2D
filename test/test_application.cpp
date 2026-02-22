
#include <bit>
#include <cstdint>
#include <iostream>
#include <set>
#include <array>
#include <string>
#include <type_traits>
#include <utility>
#include <exception>
#include <filesystem>

#include <result/to_result.hpp>

#include <Duo2D/core/error.hpp>
#include <Duo2D/core/make.hpp>
#include <Duo2D/core/application.hpp>
#include <Duo2D/core/window.hpp>
#include <Duo2D/arith/matrix.hpp>
#include <Duo2D/arith/point.hpp>
#include "Duo2D/input/category.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/timeline/predefined_callbacks/update_swap_extent.hpp"

#include "./timeline.hpp"
#include "./resource_table.hpp"

std::exception_ptr asdasdasd{};


#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
extern "C" const char* __asan_default_options() { return "detect_leaks=0"; }
#endif


using window = d2d::window<d2d::test::basic_timeline, resource_configs>;
using application = d2d::application<window>;

constexpr sl::size_t N = resource_configs.size();
using command_traits_type = d2d::timeline::impl::command_traits<
	d2d::test::basic_timeline, 
	0, 
	sl::index_sequence_type<>, 
	sl::integer_sequence_type<d2d::command_family_t>
>;
constexpr sl::size_t command_group_count = command_traits_type::group_count + d2d::timeline::impl::dedicated_command_group::num_dedicated_command_groups;
using filter_sequence = d2d::impl::device_allocation_filter_sequence<N, resource_configs, command_group_count, d2d::buffering_policy::multi, d2d::memory_policy::push_constant>;

static_assert(command_traits_type::group_indices[0] == 0);
static_assert(command_traits_type::group_indices[1] == 1);
static_assert(command_traits_type::group_indices[2] == 1);
static_assert(command_traits_type::group_indices[3] == 1);
static_assert(command_traits_type::group_indices[4] == 1);
static_assert(command_traits_type::group_indices[5] == 1);
static_assert(command_traits_type::group_indices[6] == 2);
static_assert(command_traits_type::group_indices[7] == 2);

static_assert(command_traits_type::group_families[0] == d2d::command_family::none);
static_assert(command_traits_type::group_families[1] == d2d::command_family::graphics);
static_assert(command_traits_type::group_families[2] == d2d::command_family::present);


using render_process_type = d2d::render_process<N, resource_configs, command_group_count>;
using device_allocation_group_type = d2d::impl::device_allocation_group<N, resource_configs, command_group_count, d2d::buffering_policy::multi, sl::index_sequence_of_length_type<d2d::memory_policy::num_memory_policies>>;
using device_allocation_type = d2d::vk::device_allocation<
	D2D_FRAMES_IN_FLIGHT, 
	filter_sequence,
	d2d::buffering_policy::multi, 
	d2d::memory_policy::push_constant, 
	render_process_type
>;
static_assert(std::is_base_of_v<d2d::vk::device_allocation_segment<0, render_process_type>,render_process_type>);
static_assert(std::is_base_of_v<device_allocation_type, device_allocation_group_type>);
static_assert(std::is_base_of_v<device_allocation_group_type, render_process_type>);



static_assert(std::is_base_of_v<device_allocation_type, render_process_type>);
static_assert(std::is_base_of_v<device_allocation_group_type, render_process_type>);


constexpr sl::array<6, std::uint16_t> rect_indices{{0, 1, 2, 2, 1, 3}};
constexpr sl::array<3, d2d::pt2u32> rect_positions{{
	{{{0, 0}}},
	{{{400, 200}}},
	{{{800, 400}}},
}};

int main(){
    auto a = d2d::make<application>("Duo2D Test");
    if(!a.has_value()) return a.error();
    application app = *std::move(a);


    auto d = app.devices();
    if(!d.has_value()) {
        std::cout << d2d::error::last_glfw_desc() << std::endl;
        return d.error();
    }
    std::set<d2d::vk::physical_device> device_list = *d;
    if(device_list.empty()) return d2d::error::no_vulkan_devices;

    //std::cout << std::filesystem::current_path() << std::endl;
    //const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../../test/assets"));

    app.selected_device() = *device_list.begin();
    auto i = app.initialize_device();
    if(!i.has_value()) return i.error(); 

    auto _w = app.add_window();
    if(!_w.has_value()) return _w.error();
    window* win = *_w;

	window& w = *win;


	
	w.timeline_callbacks().push_back(typename window::timeline_callbacks_type{
		.on_frame_begin_fn = [](typename window::render_process_type& proc) noexcept -> d2d::result<void> {
			push_constants constants {
				.swap_extent = proc.swap_chain().extent(),
				.position_buff_addr = sl::universal::get<resource_id::positions>(proc).gpu_address()
			};
			std::memcpy(
				sl::universal::get<resource_id::push_constants>(proc).data(),
				&constants,
				sizeof(push_constants)
			);
			return {};
		},
		.on_swap_chain_updated_fn = &d2d::timeline::predefined_callbacks::update_swap_extent<window, resource_id::push_constants>,
	});



	RESULT_VERIFY(sl::universal::get<resource_id::draw_commands>(w).reserve(sizeof(VkDrawIndexedIndirectCommand)));
	RESULT_VERIFY((sl::universal::get<resource_id::draw_commands>(w).template try_emplace_back<VkDrawIndexedIndirectCommand>(
		6, 3, 0, 0, 0
	)));

	sl::array<1, VkDeviceAddress> addresses{{
		sl::universal::get<resource_id::positions>(w).gpu_address(),
	}};
	std::memcpy(sl::universal::get<resource_id::push_constants>(w).data(), addresses.data(), addresses.size_bytes());


	RESULT_VERIFY(sl::universal::get<resource_id::staging>(w).resize(rect_indices.size_bytes()));
	std::memcpy(sl::universal::get<resource_id::staging>(w).data(), rect_indices.data(), rect_indices.size_bytes());
	
	RESULT_VERIFY(sl::universal::get<resource_id::rectangle_indices>(w).resize(rect_indices.size_bytes()));

	RESULT_VERIFY((d2d::copy(
		sl::universal::get<resource_id::rectangle_indices>(w),
		sl::universal::get<resource_id::staging>(w),
		rect_indices.size_bytes()
	)));
	//sl::universal::get<resource_id::staging>(w).clear();


	RESULT_VERIFY(sl::universal::get<resource_id::staging>(w).resize(rect_positions.size_bytes()));
	std::memcpy(sl::universal::get<resource_id::staging>(w).data(), rect_positions.data(), rect_positions.size_bytes());

	RESULT_VERIFY(sl::universal::get<resource_id::positions>(w).try_resize(rect_positions.size_bytes()));

	RESULT_VERIFY((d2d::copy(
		sl::universal::get<resource_id::positions>(w),
		sl::universal::get<resource_id::staging>(w),
		rect_positions.size_bytes()
	)));
	sl::universal::get<resource_id::staging>(w).clear();

    //2nd window
    /* {
    auto w2 = app.add_window("Duo2D Test (Second Window)");
    if(!w.has_value()) return w2.error();
    } */
	 
	{
    //for(auto const& event_fn : d2d::input::defaults::interactable::event_fns<window>())
    //    win->input_event_functions().insert(event_fn);
    //for(auto const& press_binding : d2d::input::defaults::interactable::press_bindings<window>())
    //    win->input_active_bindings().insert(press_binding);
    //for(auto const& release_binding : d2d::input::defaults::interactable::release_bindings<window>())
    //    win->input_inactive_bindings().insert(release_binding);


    d2d::input::event_set& left_active = win->input_active_bindings()[d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::key_code::kb_a}];
    left_active.applicable_categories.set(d2d::input::category::system);
    left_active.event_ids[d2d::input::category::system] = 0;
    d2d::input::event_set& left_inactive = win->input_inactive_bindings()[d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::key_code::kb_a}];
    left_inactive.applicable_categories.set(d2d::input::category::system);
    left_inactive.event_ids[d2d::input::category::system] = 1;
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{0, d2d::input::category::system}, [](void*, d2d::input::combination, bool pressed, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "left active (by " << (pressed ? std::string_view("press") : std::string_view("release")) << ")" << std::endl;
    });
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{1, d2d::input::category::system}, [](void*, d2d::input::combination, bool pressed, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "left inactive (by " << (pressed ? std::string_view("press") : std::string_view("release")) << ")" << std::endl;
    });
    //win->input_active_bindings().insert_or_assign(d2d::input::combination{{}, d2d::input::key_code::kb_a}, left_active);
    //win->input_inactive_bindings().insert_or_assign(d2d::input::combination{{d2d::input::key_code::kb_a}, d2d::input::generic_code::any}, left_active);
    //win->input_active_bindings().insert_or_assign(d2d::input::combination{{d2d::input::key_code::kb_a}, d2d::input::generic_code::any}, left_inactive);
    //win->input_inactive_bindings().insert_or_assign(d2d::input::combination{{}, d2d::input::key_code::kb_a}, left_inactive);
    //win->input_active_bindings().insert_or_assign(d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::key_code::kb_a}, left_active);
    //win->input_inactive_bindings().insert_or_assign(d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::key_code::kb_a}, left_inactive);


    d2d::input::event_set& ctrl_a_g = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_ctrl, d2d::input::key_code::kb_a}, d2d::input::key_code::kb_g}];
    ctrl_a_g.applicable_categories.set(d2d::input::category::system);
    ctrl_a_g.event_ids[d2d::input::category::system] = 2;
    d2d::input::event_set& ctrl_g_a = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_ctrl, d2d::input::key_code::kb_g}, d2d::input::key_code::kb_a}];
    ctrl_g_a.applicable_categories.set(d2d::input::category::system);
    ctrl_g_a.event_ids[d2d::input::category::system] = 2;
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{2, d2d::input::category::system}, [](void*, d2d::input::combination, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "advanced key event called" << std::endl;
    }); 

    win->current_input_categories().set(d2d::input::category::ui);
    //win->current_input_categories().set(2);

    d2d::input::event_set& inactive_mouse_move = win->input_inactive_bindings()[d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::mouse_code::move}];
    inactive_mouse_move.applicable_categories.set(d2d::input::category::system);
    inactive_mouse_move.event_ids[d2d::input::category::system] = 3;
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{3, d2d::input::category::system}, [](void*, d2d::input::combination, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "mouse move (inactive)" << std::endl;
    }); 

    d2d::input::event_set& shift_mouse_move = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_shift}, d2d::input::mouse_code::move}];
    shift_mouse_move.applicable_categories.set(d2d::input::category::system);
    shift_mouse_move.event_ids[d2d::input::category::system] = 4;
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{4, d2d::input::category::system}, [](void*, d2d::input::combination, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "shift mouse move" << std::endl;
    }); 

    d2d::input::event_set& shift_scroll_up = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_shift}, d2d::input::mouse_code::scroll}];
    shift_scroll_up.applicable_categories.set(d2d::input::category::system);
    shift_scroll_up.event_ids[d2d::input::category::system] = 5;
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{5, d2d::input::category::system}, [](void*, d2d::input::combination, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t scroll_magnitude, void*){
        if(scroll_magnitude->y() >= 0) return;
        std::cout << "shift scroll" << std::endl;
    }); 


    d2d::input::event_set& left_mouse_btn = win->input_active_bindings()[d2d::input::combination{{}, d2d::input::mouse_code::button_1}];
    left_mouse_btn.applicable_categories.set(d2d::input::category::system);
    left_mouse_btn.event_ids[d2d::input::category::system] = 6;
    //win->input_modifier_flags()[d2d::input::mouse_code::button_1] |= d2d::input::modifier_flags::no_modifiers_allowed;
    win->input_event_functions().try_emplace(d2d::input::categorized_event_t{6, d2d::input::category::system}, [](void*, d2d::input::combination, bool, d2d::input::categorized_event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "left mouse button" << std::endl;
    }); 
	}



    std::thread edit_s([](window&) noexcept -> d2d::result<void> {//, window& w){
		return {};
    }, std::ref(*win));


    std::future<d2d::result<void>> render = app.start_async_render();

    while(app.open()) {
        app.poll_events();
        //if(auto r = app.render(); !r.has_value()) [[unlikely]]
        //    return r.error();
    }
    
    if(auto r = render.get(); !r.has_value()) {
        std::cout << std::format("rendering error {}: {}", static_cast<std::int64_t>(r.error()), d2d::error::code_descs.find(r.error())->second)<< std::endl;

        auto formats = win->physical_device_ptr()->query<d2d::vk::device_query::display_formats>(win->surface());
        std::cout << std::format("{} formats ({} is selected):", formats.size(), static_cast<std::uint32_t>(win->swap_chain().format().pixel_format.id));
        for(auto f : formats)
            std::cout << std::format("{},", static_cast<std::uint32_t>(f.pixel_format.id));
        std::cout << std::endl;

        auto present_modes = win->physical_device_ptr()->query<d2d::vk::device_query::present_modes>(win->surface());
        std::cout << std::format("{} present_modes ({} is selected):", present_modes.size(), static_cast<std::uint32_t>(win->swap_chain().mode()));
        for(bool b : present_modes)
            std::cout << std::format("{},", b);
        std::cout << std::endl;
    }
    RESULT_VERIFY(app.join());
    edit_s.join();

    std::cout << d2d::error::last_glfw_desc() << std::endl;
    return 0;
}