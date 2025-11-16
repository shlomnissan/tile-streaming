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
    if (auto lod = ComputeLod(camera); first_frame_ || lod != curr_lod_) {
        prev_lod_ = first_frame_ ? lod : curr_lod_;
        curr_lod_ = lod;
        if (first_frame_) first_frame_ = false;
    }
}

auto PageManager::GetVisiblePages() const -> std::vector<Page*> {
    std::vector<Page*> visible_pages;

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
            auto pos_x = static_cast<float>(x) * page_size_;
            auto pos_y = static_cast<float>(y) * page_size_;
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
    auto virtual_width = camera.Width() / scale_x;
    auto world_units_per_pixel = virtual_width / window_dims_.width;
    auto lod = std::log2(world_units_per_pixel);
    return std::clamp(static_cast<int>(lod), 0, static_cast<int>(max_lod_));
}

auto PageManager::IsPageVisible(const Page&) const -> bool {
    return false;
}

auto PageManager::ComputeVisibleBounds(const OrthographicCamera&) const -> Box2 {
    return Box2 {};
}