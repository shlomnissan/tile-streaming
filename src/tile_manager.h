// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#pragma once

#include <memory>
#include <vector>

#include <glm/vec2.hpp>

#include "core/orthographic_camera.h"
#include "loaders/image_loader.h"
#include "tile.h"
#include "types.h"

class TileManager {
public:
    TileManager(
        const Dimensions& image_dims,
        const Dimensions& window_dims,
        const float tile_size,
        const int lods
    );

    auto Update(const OrthographicCamera& camera) -> void;

    auto GetVisibleTiles() -> std::vector<Tile*>;

    auto Debug(const OrthographicCamera& camera) const -> void;

private:
    std::vector<int> tiles_x_per_lod_;
    std::vector<int> tiles_y_per_lod_;
    std::vector<std::vector<Tile>> tiles_;

    std::shared_ptr<ImageLoader> loader_;

    Dimensions texture_dims_;
    Dimensions window_dims_;

    Box2 visible_bounds_;

    float tile_size_ {0};

    unsigned max_lod_ {0};
    unsigned curr_lod_ {0};
    unsigned prev_lod_ {0};

    bool first_frame_ {true};

    auto GenerateTiles() -> void;

    auto ComputeLod(const OrthographicCamera& camera) const -> int;

    auto IsTileVisible(const Tile& tile, const Box2& visible_bounds) const -> bool;

    auto ComputeVisibleBounds(const OrthographicCamera& camera) const -> Box2;

    auto GetTileIndex(const TileId& id) const -> int;

    auto RequestTile(const TileId& id) -> void;
};