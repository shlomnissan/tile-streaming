// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <memory>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "core/texture2d.h"
#include "loaders/image_loader.h"
#include "types.h"

struct Page {
    glm::ivec2 index;
    glm::vec2 position;
    glm::vec2 size;

    float scale;

    unsigned lod;

    bool visible {false};

    PageState state {PageState::Unloaded};

    Texture2D texture;

    Page(
        const glm::ivec2 index,
        const glm::vec2 position,
        const glm::vec2 size,
        const float scale,
        const unsigned lod
    ): index(index), position(position), size(size), scale(scale), lod(lod) {}

    [[nodiscard]] auto Transform() const -> glm::mat4;
};