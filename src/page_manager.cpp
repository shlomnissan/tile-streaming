// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include "page_manager.h"

#include <format>

#include <imgui.h>

PageManager::PageManager(const Parameters& params) :
    page_size_(params.page_size),
    image_dims_(params.image_dims),
    window_dims_(params.window_dims),
    lods_(params.lods),
    max_lod_(params.lods - 1)
{
    pages_.resize(lods_);
    GeneratePages();
}

auto PageManager::ComputeLod(const OrthographicCamera& camera) const -> int {
    const auto virtual_width = camera.Width() / glm::length(glm::vec3{camera.transform[0]});
    const auto virtual_units_per_screen_pixel =  virtual_width / window_dims_.width;
    const auto lod_shift = static_cast<float>(lods_) - 1;
    const float lod_f = lod_shift + std::log2(1 / virtual_units_per_screen_pixel);
    return std::clamp(static_cast<int>(std::floor(lod_f)), 0, max_lod_);
}

auto PageManager::Update(const OrthographicCamera& camera) -> void {
    if (auto this_lod = ComputeLod(camera); this_lod != curr_lod) {
        prev_lod = curr_lod;
        curr_lod = this_lod;
    }

    visible_bounds_ = ComputeVisibleBounds(camera);

    for (auto lod = 0; lod < lods_; ++lod) {
        for (auto& page : pages_[lod]) {
            page.visible = IsPageVisible(page);
            if (curr_lod == lod && page.visible && page.State() == PageState::Unloaded) {
                page.Load();
            }
        }
    }
};

auto PageManager::GeneratePages() -> void {
    for (auto i = 0u; i < lods_; ++i) {
        const auto lod_width = static_cast<float>(image_dims_.width) / (1 << i);
        const auto lod_height = static_cast<float>(image_dims_.height) / (1 << i);
        const auto scale = static_cast<float>(pow(2, i));
        const auto grid_x = static_cast<int>(lod_width / page_size_);
        const auto grid_y = static_cast<int>(lod_height / page_size_);
        const auto n_pages = grid_x * grid_y;

        for (auto j = 1; j <= n_pages; ++j) {
            auto x = (j - 1) % grid_x;
            auto y = (j - 1) / grid_y;
            auto path = std::format("assets/tiles/{}_{}_{}.png", i, x, y);
            pages_[i].emplace_back(Page::Params {
                .grid_index = {x, y},
                .position = {x * page_size_ * scale, y * page_size_ * scale},
                .size = {page_size_ * scale, page_size_ * scale},
                .scale = scale,
                .lod = i
            }, path);
        }
    }

    for (auto& page : pages_[max_lod_]) {
        page.Load();
    }
}

auto PageManager::GetVisiblePages() -> std::vector<Page*> {
    std::vector<Page*> visible_pages;

    // always include low-res tiles
    for (auto& page : pages_[max_lod_]) {
        if (page.visible && page.State() == PageState::Loaded) {
            visible_pages.push_back(&page);
        }
    }

    if (curr_lod == max_lod_) return visible_pages;

    bool all_loaded = true;
    for (auto& page : pages_[curr_lod]) {
        if (page.visible) {
            visible_pages.push_back(&page);
            if (page.State() != PageState::Loaded) {
                all_loaded = false;
            }
        }
    }

    if (!all_loaded && curr_lod != max_lod_) {
        for (auto& page : pages_[prev_lod]) {
            if (page.visible && page.State() == PageState::Loaded) {
                visible_pages.push_back(&page);
            }
        }
    }

    if (all_loaded && prev_lod != curr_lod && prev_lod != max_lod_) {
        // TODO: release previous LOD memory
    }

    return visible_pages;
}

auto PageManager::ComputeVisibleBounds(const OrthographicCamera& camera) const -> Bounds {
    const auto top_left_ndc = glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f);
    const auto bottom_right_ndc = glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f);

    const auto inv_vp = glm::inverse(camera.Projection() * camera.View());

    const auto top_left_world = inv_vp * top_left_ndc;
    const auto bottom_right_world = inv_vp * bottom_right_ndc;

    return Bounds {
        .min = {top_left_world.x, top_left_world.y},
        .max = {bottom_right_world.x, bottom_right_world.y}
    };
}

auto PageManager::IsPageVisible(const Page& page) const -> bool {
    const Bounds page_bounds = {
        .min = page.Position(),
        .max = page.Position() + page.Size()
    };

    const auto page_min = glm::min(page_bounds.min, page_bounds.max);
    const auto page_max = glm::max(page_bounds.min, page_bounds.max);
    const auto bounds_min = glm::min(visible_bounds_.min, visible_bounds_.max);
    const auto bounds_max = glm::max(visible_bounds_.min, visible_bounds_.max);

    return (page_min.x <= bounds_max.x && page_max.x >= bounds_min.x) &&
           (page_min.y <= bounds_max.y && page_max.y >= bounds_min.y);
}

auto PageManager::Debug() -> void {
    ImGui::SetNextWindowFocus();
    ImGui::Begin("Page Manager");
    ImGui::Text("Image dimensions: %dx%d", image_dims_.width, image_dims_.height);
    ImGui::Text("Current LOD: %d", curr_lod);
    ImGui::Separator();
    ImGui::Checkbox("Show Wireframes", &show_wireframes);
    ImGui::End();
}