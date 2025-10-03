#pragma once

#include "asset_manager.h"
#include "raylib.h"
#include "raymath.h"

constexpr int BACKGROUND_SIZE{64};

struct Background {
 public:
  void draw(const Vector2 pos) const {
    if (background_index == -1) return;

    DrawTextureV(render_texture.texture, pos, WHITE);
  }

  void preload(int index, int tile_width, int tile_height, int pixel_size) {
    if (background_index == index) return;

    if (index < 0 || index >= BACKGROUND_COUNT) {
      TraceLog(LOG_ERROR, "Invalid background index");
      return;
    }

    unload();

    background_index = index;
    render_texture =
        LoadRenderTexture(tile_width * pixel_size * BACKGROUND_SIZE, tile_height * pixel_size * BACKGROUND_SIZE);

    BeginTextureMode(render_texture);

    for (int y = 0; y < tile_height; y++) {
      for (int x = 0; x < tile_width; x++) {
        DrawTexturePro(
            *asset_manager.textures[TextureNames::Background__0 + background_index],
            {0.f, 0.f, BACKGROUND_SIZE, BACKGROUND_SIZE},
            {static_cast<float>(BACKGROUND_SIZE * pixel_size * x), static_cast<float>(BACKGROUND_SIZE * pixel_size * y),
             static_cast<float>(BACKGROUND_SIZE * pixel_size), static_cast<float>(BACKGROUND_SIZE * pixel_size)},
            Vector2Zero(), 0.f, WHITE);
      }
    }

    EndTextureMode();
  }

  void unload() {
    if (background_index != -1) UnloadRenderTexture(render_texture);
  }

 private:
  int background_index{-1};
  RenderTexture2D render_texture;
};
