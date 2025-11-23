// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include <print>
#include <vector>

#include <imgui.h>

#include "core/orthographic_camera.h"
#include "core/shaders.h"
#include "core/window.h"
#include "geometries/plane_geometry.h"
#include "resources/zoom_pan_camera.h"

#include "tile_manager.h"
#include "types.h"

#include "shaders/headers/tile_frag.h"
#include "shaders/headers/tile_vert.h"

auto main() -> int {
    const auto window_dims = Dimensions {1024.0f, 1024.0f};
    const auto texture_dims = Dimensions {8192.0f, 8192.0f};
    const auto tile_size = 1024.0f;
    const auto lods = 4;

    auto tile_manager = TileManager {
        texture_dims,
        window_dims,
        tile_size,
        lods
    };

    auto window = Window {
        static_cast<int>(window_dims.width),
        static_cast<int>(window_dims.height),
        "Tile Streaming"
    };

    // Match the camera's world-space width to the full image width so that
    // one world unit corresponds to one texel at LOD 0. This keeps zoom and
    // LOD calculations intuitive: at zoom = 1 the entire image fits exactly
    // in view, and world-units-per-pixel directly reflects texel density.
    const auto camera_width = texture_dims.width;

    const auto camera_height = camera_width / window_dims.AspectRatio();
    auto camera = OrthographicCamera {0.0f, camera_width, camera_height, 0.0f, -1.0f, 1.0f};
    auto controls = ZoomPanCamera {&camera};

    const auto geometry = PlaneGeometry {{
        .width = 1024.0f,
        .height = 1024.0f,
        .width_segments = 1,
        .height_segments = 1
    }};

    auto tile_shader = Shaders {{
        {ShaderType::kVertexShader, _SHADER_tile_vert},
        {ShaderType::kFragmentShader, _SHADER_tile_frag}
    }};

    window.Start([&]([[maybe_unused]] const double _){
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        controls.Update();
        tile_manager.Update(camera);
        tile_manager.Debug(camera);

        tile_shader.Use();
        tile_shader.SetUniform("u_Projection", camera.projection);

        auto tiles = tile_manager.GetVisibleTiles();
        for (auto& tile : tiles) {
            if (tile->state == TileState::Loaded) {
                tile->texture.Bind();
                tile_shader.SetUniform("u_ModelView", camera.View() * tile->Transform());
                geometry.Draw(tile_shader);
            }
        }
    });

    return 0;
}