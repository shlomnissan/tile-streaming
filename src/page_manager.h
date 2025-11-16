// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include "page.h"

#include "core/orthographic_camera.h"

#include <vector>

#include <glm/vec2.hpp>

class PageManager {
public:
    float page_size_ {0};
    int curr_lod {0};
    int prev_lod {0};
    bool show_wireframes {false};

    struct Dimensions {
        unsigned int width {0};
        unsigned int height {0};
    };

    struct Bounds {
        glm::vec2 min {0.0f};
        glm::vec2 max {0.0f};
    };

    struct Parameters {
        Dimensions image_dims;
        Dimensions window_dims;
        float page_size;
        int lods;
    };

    explicit PageManager(const Parameters& params);

    auto Debug() -> void;

    auto Update(const OrthographicCamera& camera) -> void;

    auto GetVisiblePages() -> std::vector<Page*>;

private:
    std::vector<std::vector<Page>> pages_;

    Bounds visible_bounds_ {};
    Dimensions image_dims_ {};
    Dimensions window_dims_ {};

    int lods_ {0};
    int max_lod_ {0};

    auto GeneratePages() -> void;

    auto ComputeLod(const OrthographicCamera& camera) const -> int;

    auto IsPageVisible(const Page& page) const -> bool;

    auto ComputeVisibleBounds(const OrthographicCamera& camera) const -> Bounds;
};