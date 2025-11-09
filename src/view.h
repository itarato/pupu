#pragma once

#include "common.h"
#include "raylib.h"

struct View {
 public:
  View() {
    camera.zoom = 1.f;
  }

  Camera2D const& get_camera() const {
    return camera;
  }

  void update(int const map_tile_width, int const map_tile_height) {
    static const float zoom_levels[8]{0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 6.f, 8.0f};
    static int zoom_level_index{2};

    bool has_changed{false};
    if (IsKeyPressed(KEY_LEFT_BRACKET) && zoom_level_index > 0) {
      has_changed = true;
      zoom_level_index -= 1;
    }
    if (IsKeyPressed(KEY_RIGHT_BRACKET) && zoom_level_index < 7) {
      has_changed = true;
      zoom_level_index += 1;
    }

    if (has_changed) {
      camera.zoom = zoom_levels[zoom_level_index];
      WORLD_ZOOM = camera.zoom;
      SetWindowSize(map_tile_width * TILE_SIZE * DEFAULT_PIXEL_SIZE * camera.zoom,
                    map_tile_height * TILE_SIZE * DEFAULT_PIXEL_SIZE * camera.zoom);
    }
  }

 private:
  Camera2D camera{};
};
