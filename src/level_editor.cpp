#include "asset_manager.h"
#include "background.h"
#include "raylib.h"

constexpr int pixel_size{3};
constexpr int tile_size{16};
constexpr int background_size{64};
constexpr int tile_width{32};
constexpr int tile_height{20};

constexpr int background_tile_width{tile_width / 4};
constexpr int background_tile_height{tile_height / 4};

constexpr int background_height{background_size * background_tile_height * pixel_size};

constexpr Vector2 gui_tile_size{240.f, 112.f};
constexpr Vector2 tileset_tile_size{256.f, 176.f};

constexpr Vector2 tile_gui_pos{0.f, background_height};
constexpr Vector2 tile_tileset_pos{gui_tile_size.x * pixel_size, background_height};

struct Editor {
 public:
  void update() {
    int key = GetKeyPressed();
    if (key >= KEY_ZERO && key <= KEY_FIVE) {
      background.preload(key - KEY_ZERO, background_tile_width, background_tile_height, pixel_size);
    }
  }

  void draw() const {
    background.draw({0.f, 0.f});

    DrawTexturePro(*asset_manager.textures[TextureNames::GuiTiles],
                   {0.f, 0.f, static_cast<float>(asset_manager.textures[TextureNames::GuiTiles]->width),
                    static_cast<float>(asset_manager.textures[TextureNames::GuiTiles]->height)},
                   {tile_gui_pos.x, tile_gui_pos.y,
                    static_cast<float>(asset_manager.textures[TextureNames::GuiTiles]->width * pixel_size),
                    static_cast<float>(asset_manager.textures[TextureNames::GuiTiles]->height * pixel_size)},
                   Vector2Zero(), 0.f, WHITE);
    DrawTexturePro(*asset_manager.textures[TextureNames::TilesetTiles],
                   {0.f, 0.f, static_cast<float>(asset_manager.textures[TextureNames::TilesetTiles]->width),
                    static_cast<float>(asset_manager.textures[TextureNames::TilesetTiles]->height)},
                   {tile_tileset_pos.x, tile_tileset_pos.y,
                    static_cast<float>(asset_manager.textures[TextureNames::TilesetTiles]->width * pixel_size),
                    static_cast<float>(asset_manager.textures[TextureNames::TilesetTiles]->height * pixel_size)},
                   Vector2Zero(), 0.f, WHITE);
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
};

int main() {
  InitWindow(tile_width * pixel_size * tile_size,
             tile_height * pixel_size * tile_size + tileset_tile_size.y * pixel_size, "Pupu Level Editor");
  asset_manager.preload();

  Editor editor{};

  while (!WindowShouldClose()) {
    editor.update();

    BeginDrawing();

    ClearBackground(RAYWHITE);

    editor.draw();

    EndDrawing();
  }

  asset_manager.unload_assets();
  editor.unload();

  CloseWindow();
}
