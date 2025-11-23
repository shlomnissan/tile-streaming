// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <format>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "core/texture2d.h"
#include "loaders/image_loader.h"

enum class TileState {
    Unloaded,
    Loading,
    Loaded,
    Error
};

struct TileId {
    unsigned lod;
    int x;
    int y;
};

template<>
struct std::formatter<TileId> : std::formatter<std::string> {
    auto format(const TileId& id, auto& ctx) const {
        return std::formatter<std::string>::format(
            std::format("{}_{}_{}", id.lod, id.x, id.y),
            ctx
        );
    }
};

struct Tile {
    TileId id;

    glm::vec2 position;
    glm::vec2 size;

    float scale;

    bool visible {false};

    TileState state {TileState::Unloaded};

    Texture2D texture;

    Tile(
        const TileId& id,
        const glm::vec2 position,
        const glm::vec2 size,
        const float scale
    ) : id(id),
        position(position),
        size(size),
        scale(scale) {}

    [[nodiscard]] auto Transform() const -> glm::mat4;
};