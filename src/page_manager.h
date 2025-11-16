// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <vector>

#include <glm/vec2.hpp>

#include "core/orthographic_camera.h"
#include "page.h"
#include "types.h"

class PageManager {
public:
    PageManager(
        const Dimensions& image_dims,
        const Dimensions& window_dims,
        const float page_size,
        const int lods
    );

    auto Update(const OrthographicCamera& camera) -> void;

    auto GetVisiblePages() const -> std::vector<Page*>;

    auto Debug() const -> void;

private:
    std::vector<std::vector<Page>> pages_;

    Dimensions image_dims_;
    Dimensions window_dims_;
    Box2 visible_bounds_;

    float page_size_ {0};

    unsigned max_lod_ {0};
    unsigned curr_lod_ {0};
    unsigned prev_lod_ {0};

    bool first_frame_ {true};
    bool show_wireframes_ {false};

    auto GeneratePages() -> void;

    auto ComputeLod(const OrthographicCamera& camera) const -> int;

    auto IsPageVisible(const Page& page) const -> bool;

    auto ComputeVisibleBounds(const OrthographicCamera& camera) const -> Box2;
};