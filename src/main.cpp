// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include <print>
#include <vector>

#include "core/orthographic_camera.h"
#include "core/shaders.h"
#include "core/window.h"
#include "geometries/plane_geometry.h"
#include "resources/zoom_pan_camera.h"

#include "shaders/headers/scene_frag.h"
#include "shaders/headers/scene_vert.h"
#include "shaders/headers/line_vert.h"
#include "shaders/headers/line_frag.h"

#include "chunk.h"
#include "chunk_manager.h"

#include <imgui.h>

struct Bounds {
    glm::vec2 min {0.0f};
    glm::vec2 max {0.0f};
};

auto main() -> int {
    constexpr auto win_width = 1024;
    constexpr auto win_height = 1024;
    constexpr auto aspect = static_cast<float>(win_width) / win_height;
    constexpr auto camera_width = 8192.0f;
    constexpr auto camera_height = camera_width / aspect;
    constexpr auto lods = 4;

    auto chunk_manager = ChunkManager {{
        .image_dims = {8192, 8192},
        .window_dims = {win_width, win_height},
        .chunk_size = 1024.0f,
        .lods = lods
    }};

    auto window = Window {win_width, win_height, "Virtual Textures"};
    auto camera = OrthographicCamera {0.0f, camera_width, camera_height, 0.0f, -1.0f, 1.0f};
    auto controls = ZoomPanCamera {&camera};
    auto geometry = PlaneGeometry {{
        .width = 1024.0f,
        .height = 1024.0f,
        .width_segments = 1,
        .height_segments = 1
    }};

    auto shader_tile = Shaders {{
        {ShaderType::kVertexShader, _SHADER_scene_vert},
        {ShaderType::kFragmentShader, _SHADER_scene_frag}
    }};

    auto shader_line = Shaders {{
        {ShaderType::kVertexShader, _SHADER_line_vert},
        {ShaderType::kFragmentShader, _SHADER_line_frag}
    }};

    window.Start([&]([[maybe_unused]] const double _){
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        controls.Update();
        chunk_manager.Update(camera);
        chunk_manager.Debug();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);

        shader_tile.Use();
        shader_tile.SetUniform("u_Projection", camera.Projection());

        auto chunks = chunk_manager.GetVisibleChunks();
        for (auto& chunk : chunks) {
            if (chunk->State() != ChunkState::Loaded) {
                continue;
            }
            chunk->Texture().Bind();
            shader_tile.SetUniform("u_ModelView", camera.View() * chunk->ModelMatrix());
            geometry.Draw(shader_tile);
        }

        if (chunk_manager.show_wireframes) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            shader_line.Use();
            shader_line.SetUniform("u_Projection", camera.Projection());

            for (const auto& chunk : chunks) {
                if (chunk->Lod() != chunk_manager.curr_lod ||
                    chunk->State() != ChunkState::Loaded) {
                    continue;
                }
                shader_line.SetUniform("u_ModelView", camera.View() * chunk->ModelMatrix());
                geometry.Draw(shader_line);
            }
        }
    });

    return 0;
}