// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <glm/vec2.hpp>

enum class PageState {
    Unloaded,
    Loading,
    Loaded,
    Error
};

struct Dimensions {
    float width {0};
    float height {0};

    auto AspectRatio() const { return width / height; }
};

struct Box2 {
    glm::vec2 min {0.0f};
    glm::vec2 max {0.0f};
};