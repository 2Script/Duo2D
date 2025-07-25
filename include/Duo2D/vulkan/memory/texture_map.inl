#pragma once
#include "Duo2D/vulkan/memory/texture_map.hpp"

#include <msdfgen.h>
#include <bit>


namespace d2d::vk::impl::draw_op {
    constexpr void move_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept {
        //TODO: use hb_draw_state instead of glyph_context->pos
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        if(shape_ptr->contours.empty() || !shape_ptr->contours.back().edges.empty())
            shape_ptr->contours.emplace_back();
        glyph_data_ptr->pos = pt2f{target_x, target_y} * glyph_data_ptr->scale;
    };
    
    constexpr void line_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float target_x, float target_y, void* glyph_ctx) noexcept {
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        const pt2d old_val = std::exchange(glyph_data_ptr->pos, pt2f{target_x, target_y} * glyph_data_ptr->scale);
        if (old_val != glyph_data_ptr->pos)
            shape_ptr->contours.back().edges.emplace_back(std::bit_cast<msdfgen::Point2>(old_val), std::bit_cast<msdfgen::Point2>(glyph_data_ptr->pos));
    };
    
    constexpr void quad_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float control_x, float control_y, float target_x, float target_y, void* glyph_ctx) noexcept {
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        const pt2d old_val = std::exchange(glyph_data_ptr->pos, pt2f{target_x, target_y} * glyph_data_ptr->scale);
        if (old_val != glyph_data_ptr->pos) {
            const pt2d control_val = pt2f{control_x, control_y} * glyph_data_ptr->scale;
            shape_ptr->contours.back().edges.emplace_back(std::bit_cast<msdfgen::Point2>(old_val), std::bit_cast<msdfgen::Point2>(control_val), std::bit_cast<msdfgen::Point2>(glyph_data_ptr->pos));
        }
    };
    
    constexpr void cubic_to(hb_draw_funcs_t*, void* shape_ctx, hb_draw_state_t*, float first_control_x, float first_control_y, float second_control_x, float second_control_y, float target_x, float target_y, void* glyph_ctx) noexcept {
        glyph_context* glyph_data_ptr = static_cast<glyph_context*>(glyph_ctx);
        msdfgen::Shape* shape_ptr = static_cast<msdfgen::Shape*>(shape_ctx);
        const pt2d old_val = std::exchange(glyph_data_ptr->pos, pt2f{target_x, target_y} * glyph_data_ptr->scale);
        const pt2d first_control_val = pt2f{first_control_x, first_control_y} * glyph_data_ptr->scale;
        const pt2d second_control_val = pt2f{second_control_x, second_control_y} * glyph_data_ptr->scale;
        if (old_val != glyph_data_ptr->pos || d2d::cross(first_control_val, second_control_val) != 0.) {
            shape_ptr->contours.back().edges.emplace_back(
                std::bit_cast<msdfgen::Point2>(old_val), 
                std::bit_cast<msdfgen::Point2>(first_control_val), 
                std::bit_cast<msdfgen::Point2>(second_control_val), 
                std::bit_cast<msdfgen::Point2>(glyph_data_ptr->pos)
            );
        }
    }; 
}