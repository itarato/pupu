#include <memory>

#include "asset_manager.h"
#include "background.h"
#include "common.h"
#include "raylib.h"

constexpr int pixel_size{3};
constexpr int tile_size{16};
constexpr int background_size{64};
constexpr int tile_width{32};
constexpr int tile_height{20};

constexpr int background_tile_width{tile_width / 4};
constexpr int background_tile_height{tile_height / 4};

constexpr Rectangle background_frame{
    0.f,
    0.f,
    background_size* background_tile_width,
    background_size* background_tile_height,
};

constexpr Rectangle gui_tile_frame{
    0.f,
    background_frame.height* pixel_size,
    240.f,
    112.f,
};
constexpr Rectangle tileset_tile_frame{
    gui_tile_frame.width * pixel_size,
    background_frame.height* pixel_size,
    256.f,
    176.f,
};

enum TileSource {
  Gui,
  Tileset,
};

struct TileSelection {
  TileSource source;
  Vector2 tile_pos;

  void draw(Vector2 const pos, int const pixel_size) const {
    std::shared_ptr<Texture2D> texture;
    if (source == TileSource::Gui) {
      texture = asset_manager.textures[TextureNames::GuiTiles];
    } else if (source == TileSource::Tileset) {
      texture = asset_manager.textures[TextureNames::TilesetTiles];
    } else {
      TraceLog(LOG_ERROR, "Invalid tile source");
      return;
    }

    DrawTexturePro(
        *texture, {0.f, 0.f, tile_size, tile_size},
        {pos.x, pos.y, static_cast<float>(tile_size * pixel_size), static_cast<float>(tile_size * pixel_size)},
        Vector2Zero(), 0.f, WHITE);
  }
};

struct Editor {
 public:
  Editor() {
    background.preload(0, background_tile_width, background_tile_height, pixel_size);
  }

  void update() {
    int key = GetKeyPressed();
    if (key >= KEY_ZERO && key <= KEY_FIVE) {
      background.preload(key - KEY_ZERO, background_tile_width, background_tile_height, pixel_size);
    }

    if (IsMouseButtonPressed(0)) {
    }
  }

  void draw() const {
    // Background.
    background.draw({0.f, 0.f});

    // Tilesets.
    DrawTexturePro(
        *asset_manager.textures[TextureNames::GuiTiles], {0.f, 0.f, gui_tile_frame.width, gui_tile_frame.height},
        {gui_tile_frame.x, gui_tile_frame.y, gui_tile_frame.width * pixel_size, gui_tile_frame.height * pixel_size},
        Vector2Zero(), 0.f, WHITE);
    DrawTexturePro(*asset_manager.textures[TextureNames::TilesetTiles],
                   {0.f, 0.f, tileset_tile_frame.width, tileset_tile_frame.height},
                   {tileset_tile_frame.x, tileset_tile_frame.y, tileset_tile_frame.width * pixel_size,
                    tileset_tile_frame.height * pixel_size},
                   Vector2Zero(), 0.f, WHITE);

    // Tile highlight.
    Vector2 mouse_pos = GetMousePosition();
    DrawRectangle(mod_reduced(mouse_pos.x, tile_size * pixel_size), mod_reduced(mouse_pos.y, tile_size * pixel_size),
                  tile_size * pixel_size, tile_size * pixel_size, ColorAlpha(WHITE, 0.3f));

    tile_selection.draw(Vector2Zero(), pixel_size);
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
  TileSelection tile_selection{TileSource::Gui, Vector2Zero()};
};

int main() {
  InitWindow(background_frame.width * pixel_size, (background_frame.height + tileset_tile_frame.height) * pixel_size,
             "Pupu Level Editor");
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
