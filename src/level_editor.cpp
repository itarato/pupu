#include <memory>
#include <unordered_map>

#include "asset_manager.h"
#include "background.h"
#include "common.h"
#include "raylib.h"

constexpr int pixel_size{3};
constexpr int tile_size{16};
constexpr int tile_size_px{tile_size * pixel_size};
constexpr int background_tile_size{64};
constexpr int tile_width{32};
constexpr int tile_height{20};

constexpr int background_tile_width{tile_width / 4};
constexpr int background_tile_height{tile_height / 4};

constexpr Rectangle background_frame{0.f, 0.f, tile_size* tile_width, tile_size* tile_height};
constexpr Rectangle background_frame_px = upscale(background_frame, pixel_size);

constexpr Rectangle gui_tile_frame{0.f, background_frame.height, 240.f, 112.f};
constexpr Rectangle tileset_tile_frame{gui_tile_frame.width, background_frame.height, 256.f, 176.f};

constexpr Rectangle gui_tile_frame_px = upscale(gui_tile_frame, pixel_size);
constexpr Rectangle tileset_tile_frame_px = upscale(tileset_tile_frame, pixel_size);

enum TileSource {
  Gui,
  Tileset,
};

struct TileSelection {
  TileSource source;
  IntVec2 tile_pos;

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

    DrawTexturePro(*texture, {static_cast<float>(tile_pos.x), static_cast<float>(tile_pos.y), tile_size, tile_size},
                   {pos.x, pos.y, static_cast<float>(tile_size_px), static_cast<float>(tile_size_px)}, Vector2Zero(),
                   0.f, WHITE);
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

    if (IsMouseButtonDown(0)) {
      Vector2 mouse_pos = GetMousePosition();

      if (CheckCollisionPointRec(mouse_pos, background_frame_px)) {
        IntVec2 int_coord{mod_reduced(mouse_pos.x, tile_size_px), mod_reduced(mouse_pos.y, tile_size_px)};
        tiles[int_coord] = tile_selection;
      } else if (CheckCollisionPointRec(mouse_pos, gui_tile_frame_px)) {
        tile_selection = TileSelection{TileSource::Gui, relative_frame_pos(gui_tile_frame_px, tile_size, pixel_size)};
      } else if (CheckCollisionPointRec(mouse_pos, tileset_tile_frame_px)) {
        tile_selection =
            TileSelection{TileSource::Tileset, relative_frame_pos(tileset_tile_frame_px, tile_size, pixel_size)};
      }
    }
    if (IsMouseButtonDown(1)) {
      Vector2 mouse_pos = GetMousePosition();

      if (CheckCollisionPointRec(mouse_pos, background_frame_px)) {
        IntVec2 int_coord{mod_reduced(mouse_pos.x, tile_size_px), mod_reduced(mouse_pos.y, tile_size_px)};
        tiles.erase(int_coord);
      }
    }
  }

  void draw() const {
    // Background.
    background.draw({0.f, 0.f});

    // Tiles.
    for (auto const& [k, v] : tiles) v.draw(k.to_vector2(), pixel_size);

    // Tilesets.
    DrawTexturePro(*asset_manager.textures[TextureNames::GuiTiles],
                   {0.f, 0.f, gui_tile_frame.width, gui_tile_frame.height}, gui_tile_frame_px, Vector2Zero(), 0.f,
                   WHITE);
    DrawTexturePro(*asset_manager.textures[TextureNames::TilesetTiles],
                   {0.f, 0.f, tileset_tile_frame.width, tileset_tile_frame.height}, tileset_tile_frame_px,
                   Vector2Zero(), 0.f, WHITE);

    Vector2 mouse_pos = GetMousePosition();

    if (CheckCollisionPointRec(mouse_pos, background_frame_px)) {
      tile_selection.draw({static_cast<float>(mod_reduced(mouse_pos.x, tile_size_px)),
                           static_cast<float>(mod_reduced(mouse_pos.y, tile_size_px))},
                          pixel_size);

    } else {
      // Tile highlight.
      DrawRectangle(mod_reduced(mouse_pos.x, tile_size_px), mod_reduced(mouse_pos.y, tile_size_px), tile_size_px,
                    tile_size_px, ColorAlpha(WHITE, 0.5f));
      DrawRectangleLinesEx({static_cast<float>(mod_reduced(mouse_pos.x, tile_size_px)),
                            static_cast<float>(mod_reduced(mouse_pos.y, tile_size_px)), tile_size_px, tile_size_px},
                           pixel_size, BLACK);
    }
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
  TileSelection tile_selection{TileSource::Gui, {0, 0}};
  std::unordered_map<IntVec2, TileSelection> tiles{};
};

int main() {
  InitWindow(background_frame_px.width, background_frame_px.height + tileset_tile_frame_px.height, "Pupu Level Editor");
  SetTargetFPS(30);

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
