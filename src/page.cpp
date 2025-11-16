// Copyright © 2025 - Present, Shlomi Nissan.
// All rights reserved.

#include "page.h"

#include <random>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

Page::Page(const Params& params, const fs::path& path) : params_(params), source_(path) {
    image_loader_ = ImageLoader::Create();
}

auto Page::Load() -> void {
    using enum PageState;

    if (state_ == Loading || state_ == Loaded) {
        return;
    }

    state_ = Loading;

    image_loader_->LoadAsync(source_, [this](const auto& image) {
        if (image.has_value()) {
            texture_.SetImage(image.value());
            state_ = Loaded;
            std::print("Loaded page {}\n", source_.string());
        } else {
            state_ = Error;
        }
    });
}

auto Page::ModelMatrix() const -> glm::mat4 {
    return glm::translate(glm::mat4(1.0f), glm::vec3 {
        params_.position.x + params_.size.x / 2.0f, // offset right
        params_.position.y + params_.size.y / 2.0f, // offset up
        0.0f
    }) * glm::scale(glm::mat4(1.0f), glm::vec3 {params_.scale, params_.scale, 1.0f});
}