// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include "page_manager.h"

#include <imgui.h>

PageManager::PageManager(
    const Dimensions& image_dims,
    const Dimensions& window_dims,
    const float page_size,
    const int lods
) : image_dims_(image_dims),
    window_dims_(window_dims),
    page_size_(page_size),
    max_lod_(lods - 1)
{
    pages_.resize(lods);
    GeneratePages();
}

auto PageManager::Update(const OrthographicCamera& camera) -> void {
    const auto lod = ComputeLod(camera);

    if (first_frame_) {
        prev_lod_ = lod;
        curr_lod_ = lod;
        first_frame_ = false;
    }

    if (lod != curr_lod_) {
        prev_lod_ = curr_lod_;
        curr_lod_ = lod;
    }

    const auto visible_bounds = ComputeVisibleBounds(camera);
    for (auto i = 0; i <= max_lod_; ++i) {
        for (auto& page : pages_[i]) {
            page.visible = IsPageVisible(page, visible_bounds);
        }
    }
}

auto PageManager::GetVisiblePages() -> std::vector<Page*> {
    std::vector<Page*> visible_pages;

    for (auto& page : pages_[curr_lod_]) {
        visible_pages.push_back(&page);
    }

    return visible_pages;
}

auto PageManager::Debug() const -> void {
    ImGui::SetNextWindowFocus();
    ImGui::Begin("Page Manager");

    ImGui::Text(
        "Image dimensions: %dx%d",
        static_cast<int>(image_dims_.width),
        static_cast<int>(image_dims_.height)
    );

    ImGui::Text("Current LOD: %d", curr_lod_);
    ImGui::End();
}

auto PageManager::GeneratePages() -> void {
    for (auto i = 0u; i <= max_lod_; ++i) {
        auto lod_width = image_dims_.width / static_cast<float>(1 << i);
        auto lod_height = image_dims_.height / static_cast<float>(1 << i);
        auto scale = static_cast<float>(pow(2, i));
        auto grid_x = static_cast<int>(lod_width / page_size_);
        auto grid_y = static_cast<int>(lod_height / page_size_);
        auto n_pages = grid_x * grid_y;

        for (auto j = 1; j <= n_pages; ++j) {
            auto x = (j - 1) % grid_x;
            auto y = (j - 1) / grid_y;
            auto pos_x = static_cast<float>(x) * page_size_ * scale;
            auto pos_y = static_cast<float>(y) * page_size_ * scale;
            pages_[i].emplace_back(Page {
                {x, y}, // grid index
                {pos_x, pos_y}, // position,
                {page_size_ * scale, page_size_ * scale}, // size,
                scale, // scale
                i // lod
            });
        }
    }
}

auto PageManager::ComputeLod(const OrthographicCamera& camera) const -> int {
    auto scale_x = glm::length(glm::vec3{camera.transform[0]});
    auto virtual_width = camera.Width() * scale_x;
    auto world_units_per_pixel = virtual_width / window_dims_.width;
    auto lod = std::log2(world_units_per_pixel);
    return std::clamp(static_cast<int>(lod), 0, static_cast<int>(max_lod_));
}

auto PageManager::IsPageVisible(const Page& page, const Box2& visible_bounds) const -> bool {
    auto bounds = Box2 {.min = page.position, .max = page.position + page.size};
    return visible_bounds.Intersects(bounds);
}

auto PageManager::ComputeVisibleBounds(const OrthographicCamera& camera) const -> Box2 {
    auto inv_vp = glm::inverse(camera.projection * camera.View());
    auto top_left = inv_vp * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
    auto bottom_right = inv_vp * glm::vec4(1.0f, -1.0f, 0.0, 1.0f);
    return Box2::FromPoints(
        {top_left.x, top_left.y},
        {bottom_right.x, bottom_right.y}
    );
}