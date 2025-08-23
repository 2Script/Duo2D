
#include <iostream>
#include <result/to_result.hpp>
#include <set>
#include <array>
#include <string>
#include <type_traits>
#include <utility>
#include <filesystem>

#include <Duo2D/core/error.hpp>
#include <Duo2D/core/make.hpp>
#include <Duo2D/core/application.hpp>
#include <Duo2D/core/window.hpp>
#include <Duo2D/arith/matrix.hpp>
#include <Duo2D/arith/point.hpp>
#include "Duo2D/input/category.hpp"
#include "Duo2D/input/code.hpp"
#include "Duo2D/input/modifier_flags.hpp"
#include <Duo2D/graphics/prim/debug_rect.hpp>
#include <Duo2D/graphics/prim/styled_rect.hpp>
#include <Duo2D/graphics/ui/text.hpp>
#include <Duo2D/graphics/ui/progress_bar.hpp>


#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
extern "C" const char* __asan_default_options() { return "detect_leaks=0"; }
#endif


int main(){
    auto a = d2d::make<d2d::application>("Duo2D Test");
    if(!a.has_value()) return a.error();
    d2d::application app = *std::move(a);


    auto d = app.devices();
    if(!d.has_value()) {
        std::cout << d2d::error::last_glfw_desc() << std::endl;
        return d.error();
    }
    std::set<d2d::vk::physical_device> device_list = *d;
    if(device_list.empty()) return d2d::error::no_vulkan_devices;

    const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../../test/assets"));

    app.selected_device() = *device_list.begin();
    auto i = app.initialize_device();
    if(!i.has_value()) return i.error(); 

    auto w = app.add_window();
    if(!w.has_value()) return w.error();
    d2d::window* win = *w;
    d2d::pt2<int> v = {10, 10};
    d2d::transform(v, d2d::scale{4, 4}, d2d::translate{10,10});
    d2d::styled_rect s = d2d::styled_rect{{50,100,300,1044-100}, 0x009999FF};
    d2d::clone_rect c = d2d::clone_rect{s};
    d2d::debug_rect dr = d2d::debug_rect{{}, {400, 750,200,100}, 0x999900FF};
    d2d::debug_rect junk = d2d::debug_rect{{}, {500-400,450-225,800,450}, 0x222222FF};
    //s.bounds = d2d::rect<float>{800-400,450-225,800,450};
    //s.color = 0x009999FF;
    s.transform->scale = {.6f, .9f};
    s.transform->rotation = d2d::vk_mat2{{std::array<float, 2>{{1.f, 2.f}}, std::array<float, 2>{{3.f, 4.f}}}};
    s.transform->translation = {5.f, 7.f};
    s.border_width = 5;
    
    //s.texture_bounds->size = {1200, 675};
    std::string test_img_alpha_path = assets_path / "test_img_alpha.png";
    s._texture_paths = {test_img_alpha_path, ""};
    //s.texture_bounds->pos = {350, 100};
    //s.texture_bounds->size = {300, 1044-100};
    std::pair<const std::string, d2d::styled_rect> p = {"cyan", s};
    std::pair<const std::string, d2d::clone_rect> pc = {"cyan", c};
    std::pair<const std::string, d2d::debug_rect> pd = {"yellow", dr};
    std::pair<const std::string, d2d::debug_rect> j = {"junk", junk};

    auto insert_cyan = win->insert(std::move(p));
    if(!insert_cyan.second) return -69;
    d2d::styled_rect& cyan_ref = insert_cyan.first->second;
    cyan_ref.border_width = 7;
    RESULT_VERIFY(win->apply_changes<d2d::styled_rect>());

    auto emplace_magenta = win->try_emplace<d2d::clone_rect>("magenta", d2d::styled_rect{d2d::rect<float>{400,50,1000,675}, 0x990099FF});
    if(!emplace_magenta.second) return -69;
    d2d::clone_rect& magenta_ref = emplace_magenta.first->second;
    magenta_ref.transform->scale = {.6f, .9f};
    magenta_ref.transform->rotation = d2d::vk_mat2{{std::array<float, 2>{{1.f, 2.f}}, std::array<float, 2>{{3.f, 4.f}}}};
    magenta_ref.transform->translation = {5.f, 7.f};
    //magenta_ref._texture_paths = {"/home/artin/Repos/Arastais/Test/test_img_alpha.png"};
    //magenta_ref.texture_bounds->pos = {100, 100};
    //magenta_ref.texture_bounds->size = {300, 1044};
    std::string test_img_alpha_2_path = assets_path / "test_img_alpha_2.png";
    std::string test_img_alpha_3_path = assets_path / "test_img_alpha_3.png";
    magenta_ref._texture_paths = {test_img_alpha_3_path, test_img_alpha_2_path};
    magenta_ref.texture_bounds->pos = {100, 0};
    magenta_ref.texture_bounds->size = {1100, 675};
    magenta_ref.border_width = 10;
    RESULT_VERIFY(win->apply_changes<d2d::clone_rect>());

    win->insert(pd);
    //auto insert_white = win->emplace<d2d::debug_rect>("white", d2d::debug_rect{{}, d2d::rect<float>{50,50,1000,200}, 0x999999FF});
    //if(!insert_white.second) return -69;
    ////d2d::debug_rect& white_ref = insert_white.first->second;
    RESULT_VERIFY(win->apply_changes<d2d::debug_rect>());

    win->erase<d2d::debug_rect>("yellow");
    RESULT_VERIFY(win->apply_changes<d2d::debug_rect>());
    auto insert_new = win->insert(pd);
    if(!insert_new.second) return -69;
    //d2d::debug_rect& dr_ref = insert_new.first->second;
    if(win->insert(pd).second) return -69;
    RESULT_VERIFY(win->apply_changes<d2d::debug_rect>());

    win->emplace<d2d::debug_rect>("junk", junk);
    win->erase<d2d::debug_rect>("junk");
    RESULT_VERIFY(win->apply_changes<d2d::debug_rect>());

    //expensive text rendering
    {
    d2d::font test_font_mono("Test Mono");
    d2d::font test_font("Test");
    d2d::font test_font2("Test2");
    if(!win->try_emplace<d2d::font>(test_font_mono, assets_path / "test_mono.ttf").second) return -69;
    //if(!win->try_emplace<d2d::font>(test_font, assets_path / "test_font.ttf").second) return -69;
    //if(!win->emplace<d2d::font>(std::make_pair(test_font2, assets_path / "test_font2.ttf")).second) return -69;
    RESULT_VERIFY(win->apply_changes<d2d::font>()); 

    //d2d::text sample_text("initial", {10,10}, test_font, 32, 0x0, 0);
    //auto emplace_text = win->emplace<d2d::text>("sample_text", std::move(sample_text));
    //if(!emplace_text.second) return -69;
    //d2d::text& text_ref = emplace_text.first->second;
    //text_ref.emplace("before", {}, test_font2, 64, 0xFFFFFFFF, 11 - 7); //realloc but no apply
    //if(text_ref.try_emplace("long_sample_text", {}, test_font2, 64, 0xFFFFFFFF).second) return -69; //fails - needs size increase
    //RESULT_VERIFY(win->apply_changes<d2d::text>()); //text is "before" here
    //text_ref.emplace("long_after", {}, test_font2, 64, 0xFFFFFFFF); //no realloc or apply
    //text_ref.emplace("longer_after", {}, test_font2, 64, 0xFFFFFFFF); //realloc + apply
    //if(text_ref.try_emplace("very_long_after", {}, test_font2, 64, 0xFFFFFFFF).second) return -69; //fails - needs size increase
    //if(!text_ref.try_emplace("after", {}, test_font2, 64, 0xFFFFFFFF).second) return -69; //succeeds - no realloc

    auto emplace_text_2 = win->try_emplace<d2d::text>("sample_text_2", 16);
    if(!emplace_text_2.second) return -69;
    d2d::text& text_2_ref = emplace_text_2.first->second;
    //text_2_ref.emplace("Affluent in mega", d2d::pt2f{700, 750}, test_font, 128, 0x0C'0C'0C'FF, 5);
    text_2_ref.emplace("Affluent in mega", d2d::pt2f{700, 750}, test_font_mono, 128, 0x0C'0C'0C'FF, 5);
    RESULT_VERIFY(win->apply_changes<d2d::text>());

    auto emplace_progress_bar = win->try_emplace<d2d::progress_bar>("sample_progress_bar", d2d::pt2f{700, 900}, d2d::sz2f{400, 100}, test_font_mono, 0xFF'FF'FF'FF, 0xFF'00'00'FF, 0xFF'FF'FF'FF, 32);
    if(!emplace_progress_bar.second) return -69;
    RESULT_VERIFY(win->apply_changes<d2d::progress_bar>());

    //s._texture_paths = {test_img_alpha_path, ""};
    //s.texture_bounds->pos = {350, 100};
    //s.texture_bounds->size = {300, 1044-100};
    } 


    //2nd window
    /* {
    auto w2 = app.add_window("Duo2D Test (Second Window)");
    if(!w.has_value()) return w2.error();
    } */
    d2d::input::event_set left_active{};
    left_active.applicable_categories.set(d2d::input::category::system);
    left_active.event_ids[d2d::input::category::system] = 0;
    d2d::input::event_set left_inactive{};
    left_inactive.applicable_categories.set(d2d::input::category::system);
    left_inactive.event_ids[d2d::input::category::system] = 1;
    win->event_functions().try_emplace(d2d::input::event_t{0, d2d::input::category::system}, [](void*, d2d::input::combination, bool pressed, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "left active (by " << (pressed ? std::string_view("press") : std::string_view("release")) << ")" << std::endl;
    });
    win->event_functions().try_emplace(d2d::input::event_t{1, d2d::input::category::system}, [](void*, d2d::input::combination, bool pressed, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "left inactive (by " << (pressed ? std::string_view("press") : std::string_view("release")) << ")" << std::endl;
    });
    //win->input_active_bindings().insert_or_assign(d2d::input::combination{{}, d2d::input::key_code::kb_a}, left_active);
    //win->input_inactive_bindings().insert_or_assign(d2d::input::combination{{d2d::input::key_code::kb_a}, d2d::input::generic_code::any}, left_active);
    //win->input_active_bindings().insert_or_assign(d2d::input::combination{{d2d::input::key_code::kb_a}, d2d::input::generic_code::any}, left_inactive);
    //win->input_inactive_bindings().insert_or_assign(d2d::input::combination{{}, d2d::input::key_code::kb_a}, left_inactive);
    win->input_active_bindings().insert_or_assign(d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::key_code::kb_a}, left_active);
    win->input_inactive_bindings().insert_or_assign(d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::key_code::kb_a}, left_inactive);


    d2d::input::event_set& ctrl_a_g = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_ctrl, d2d::input::key_code::kb_a}, d2d::input::key_code::kb_g}];
    ctrl_a_g.applicable_categories.set(d2d::input::category::system);
    ctrl_a_g.event_ids[d2d::input::category::system] = 2;
    d2d::input::event_set& ctrl_g_a = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_ctrl, d2d::input::key_code::kb_g}, d2d::input::key_code::kb_a}];
    ctrl_g_a.applicable_categories.set(d2d::input::category::system);
    ctrl_g_a.event_ids[d2d::input::category::system] = 2;
    win->event_functions().try_emplace(d2d::input::event_t{2, d2d::input::category::system}, [](void*, d2d::input::combination, bool, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "advanced key event called" << std::endl;
    }); 

    win->current_input_categories().set(1);
    //win->current_input_categories().set(2);

    d2d::input::event_set& inactive_mouse_move = win->input_inactive_bindings()[d2d::input::combination{{d2d::input::generic_code::any}, d2d::input::mouse_code::move}];
    inactive_mouse_move.applicable_categories.set(2);
    inactive_mouse_move.event_ids[2] = 0;
    win->event_functions().try_emplace(d2d::input::event_t{0, 2}, [](void*, d2d::input::combination, bool, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "mouse move (inactive)" << std::endl;
    }); 

    d2d::input::event_set& shift_mouse_move = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_shift}, d2d::input::mouse_code::move}];
    shift_mouse_move.applicable_categories.set(1);
    shift_mouse_move.event_ids[1] = 0;
    win->event_functions().try_emplace(d2d::input::event_t{0, 1}, [](void*, d2d::input::combination, bool, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "shift mouse move" << std::endl;
    }); 

    d2d::input::event_set& shift_scroll_up = win->input_active_bindings()[d2d::input::combination{{d2d::input::key_code::kb_left_shift}, d2d::input::mouse_code::scroll_up}];
    shift_scroll_up.applicable_categories.set(1);
    shift_scroll_up.event_ids[1] = 1;
    win->event_functions().try_emplace(d2d::input::event_t{1, 1}, [](void*, d2d::input::combination, bool, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "shift scroll" << std::endl;
    }); 


    d2d::input::event_set& left_mouse_btn = win->input_active_bindings()[d2d::input::combination{{}, d2d::input::mouse_code::button_1}];
    left_mouse_btn.applicable_categories.set(1);
    left_mouse_btn.event_ids[1] = 2;
    win->input_modifier_flags()[d2d::input::mouse_code::button_1] |= d2d::input::modifier_flags::no_modifiers_allowed;
    win->event_functions().try_emplace(d2d::input::event_t{2, 1}, [](void*, d2d::input::combination, bool, d2d::input::event_t, d2d::input::mouse_aux_t, void*){
        std::cout << "left mouse button" << std::endl;
    }); 



    std::thread edit_s([](d2d::styled_rect& sr){//, d2d::window& w){
        sr.texture_bounds->pos = {1000, 1000};
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        //std::size_t begin_count = ++w.update_count.value;
        std::cout << "updating texture" << std::endl;
        sr.texture_bounds->pos = {350, 100};
        sr.texture_bounds->size = {300, 1044-100};
        //if(w.update_count.value <= begin_count) {
        //    VkSemaphoreSignalInfo signal_info {
        //        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
        //        .pNext = NULL,
        //        .semaphore = w.test_semaphore,
        //        .value = begin_count,
        //    };
        //    vkSignalSemaphore(w.logical_device(), &signal_info);
        //}
    }, std::ref(cyan_ref));//, std::ref(*win));


    std::thread render_thread([](d2d::application& a) -> d2d::result<void> {
        while(a.open())
            if(auto r = a.render(); !r.has_value()) [[unlikely]]
                return r.error();
        return {};
    }, std::ref(app));


    while(app.open()) {
        app.poll_events();
        //if(auto r = app.render(); !r.has_value()) [[unlikely]]
        //    return r.error();
    }
    
    edit_s.join();
    render_thread.join();
    RESULT_VERIFY(app.join());
    return 0;
}