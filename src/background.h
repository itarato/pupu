#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "raymath.h"

constexpr int BACKGROUND_SIZE{64};

struct Background {
 public:
  void draw(const Vector2 pos, int const pixel_size) const {
    if (background_index == -1) return;

    DrawTexturePro(render_texture.texture,
                   {0.f, 0.f, static_cast<float>(tile_width * TILE_SIZE * pixel_size),
                    static_cast<float>(tile_height * TILE_SIZE * pixel_size)},
                   {pos.x, pos.y, static_cast<float>(tile_width * TILE_SIZE * pixel_size),
                    static_cast<float>(tile_height * TILE_SIZE * pixel_size)},
                   vector_zero, 0.f, WHITE);
  }

  void preload(int index, int new_tile_width, int new_tile_height, int pixel_size) {
    tile_width = new_tile_width;
    tile_height = new_tile_height;
    if (index < 0 || index >= BACKGROUND_COUNT) {
      TraceLog(LOG_ERROR, "Invalid background index");
      return;
    }

    unload();

    background_index = index;
    render_texture = LoadRenderTexture(tile_width * TILE_SIZE * pixel_size, tile_height * TILE_SIZE * pixel_size);

    BeginTextureMode(render_texture);

    for (int y = 0; y < (tile_height + 3) / 4; y++) {
      for (int x = 0; x < (tile_width + 3) / 4; x++) {
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

  int get_current_index() const {
    return background_index;
  }

 private:
  int background_index{-1};
  int tile_width{};
  int tile_height{};
  RenderTexture2D render_texture;
};
