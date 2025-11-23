// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include "tile_manager.h"

#include <format>
#include <print>

#include <imgui.h>

TileManager::TileManager(
    const Dimensions& texture_dims,
    const Dimensions& window_dims,
    const float tile_size,
    const int lods
) :
    loader_(ImageLoader::Create()),
    texture_dims_(texture_dims),
    window_dims_(window_dims),
    tile_size_(tile_size),
    max_lod_(lods - 1)
{
    tiles_x_per_lod_.resize(lods);
    tiles_y_per_lod_.resize(lods);
    tiles_.resize(lods);
    GenerateTiles();
}

auto TileManager::Update(const OrthographicCamera& camera) -> void {
    const auto this_lod = ComputeLod(camera);

    if (first_frame_) {
        prev_lod_ = this_lod;
        curr_lod_ = this_lod;
        first_frame_ = false;
    }

    if (this_lod != curr_lod_) {
        prev_lod_ = curr_lod_;
        curr_lod_ = this_lod;
    }

    const auto visible_bounds = ComputeVisibleBounds(camera);
    for (auto lod = 0; lod <= max_lod_; ++lod) {
        for (auto& tile : tiles_[lod]) {
            tile.visible = IsTileVisible(tile, visible_bounds);
            if (tile.visible && lod == curr_lod_ && tile.state == TileState::Unloaded) {
                RequestTile(tile.id);
            }
        }
    }
}

auto TileManager::GetVisibleTiles() -> std::vector<Tile*> {
    std::vector<Tile*> visible_tiles;

    // always include low-res tiles
    for (auto& tile : tiles_[max_lod_]) {
        if (tile.visible && tile.state == TileState::Loaded) {
            visible_tiles.push_back(&tile);
        }
    }

    if (curr_lod_ == max_lod_) return visible_tiles;

    for (auto& tile : tiles_[curr_lod_]) {
        if (tile.state == TileState::Loaded) {
            visible_tiles.push_back(&tile);
        }
    }

    return visible_tiles;
}

auto TileManager::Debug(const OrthographicCamera& camera) const -> void {
    auto camera_scale = glm::length(glm::vec3 {camera.transform[0]});

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Tile Manager");
    ImGui::Text("Texture size: %d", static_cast<int>(texture_dims_.height));
    ImGui::Text("Current LOD: %d", curr_lod_);
    ImGui::Text("Camera size: %.2f", camera.Width() * camera_scale);

    ImGui::End();
}

auto TileManager::GenerateTiles() -> void {
    for (auto lod = 0u; lod <= max_lod_; ++lod) {
        auto lod_w = texture_dims_.width / static_cast<float>(1 << lod);
        auto lod_h = texture_dims_.height / static_cast<float>(1 << lod);
        auto lod_scale = static_cast<float>(pow(2, lod));
        auto tiles_x = static_cast<int>(std::ceil(lod_w / tile_size_));
        auto tiles_y = static_cast<int>(std::ceil(lod_h / tile_size_));

        tiles_x_per_lod_[lod] = tiles_x;
        tiles_y_per_lod_[lod] = tiles_y;

        tiles_[lod].reserve(tiles_x * tiles_y);

        for (auto y = 0; y < tiles_y; ++y) {
            for (auto x = 0; x < tiles_x; ++x) {
                auto id = TileId {lod, x, y};

                auto size = glm::vec2 {
                    tile_size_ * lod_scale,
                    tile_size_ * lod_scale,
                };

                auto position = glm::vec2 {
                    static_cast<float>(x) * size.x,
                    static_cast<float>(y) * size.y
                };

                tiles_[lod].emplace_back(id, position, size, lod_scale);
            }
        }
    }
}

auto TileManager::ComputeLod(const OrthographicCamera& camera) const -> int {
    auto scale_x = glm::length(glm::vec3{camera.transform[0]});
    auto virtual_width = camera.Width() * scale_x;
    auto world_units_per_pixel = virtual_width / window_dims_.width;
    auto lod = std::log2(world_units_per_pixel);
    return std::clamp(static_cast<int>(lod), 0, static_cast<int>(max_lod_));
}

auto TileManager::IsTileVisible(const Tile& tile, const Box2& visible_bounds) const -> bool {
    auto bounds = Box2 {.min = tile.position, .max = tile.position + tile.size};
    return visible_bounds.Intersects(bounds);
}

auto TileManager::ComputeVisibleBounds(const OrthographicCamera& camera) const -> Box2 {
    auto inv_vp = glm::inverse(camera.projection * camera.View());
    auto top_left = inv_vp * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
    auto bottom_right = inv_vp * glm::vec4(1.0f, -1.0f, 0.0, 1.0f);
    return Box2::FromPoints(
        {top_left.x, top_left.y},
        {bottom_right.x, bottom_right.y}
    );
}

auto TileManager::GetTileIndex(const TileId& id) const -> int {
    return id.y * tiles_x_per_lod_[id.lod] + id.x;
}

auto TileManager::RequestTile(const TileId& id) -> void {
    const auto idx = GetTileIndex(id);
    const auto path = std::format("assets/tiles/{}.png", id);

    tiles_[id.lod][idx].state = TileState::Loading;
     loader_->LoadAsync(path, [this, id, idx](auto result) {
        if (result) {
            tiles_[id.lod][idx].texture.SetImage(result.value());
            tiles_[id.lod][idx].state = TileState::Loaded;
            std::println("Loaded tile {}", id);
        } else {
            tiles_[id.lod][idx].state = TileState::Unloaded;
            std::println("Failed to load tile {}", id.lod);
        }
    });
}