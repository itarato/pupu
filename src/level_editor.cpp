#include <cstdio>
#include <memory>
#include <unordered_map>

#include "asset_manager.h"
#include "background.h"
#include "common.h"
#include "raylib.h"

constexpr int background_tile_size{64};
constexpr int tile_width{32};
constexpr int tile_height{20};

constexpr int background_tile_width{tile_width / 4};
constexpr int background_tile_height{tile_height / 4};

constexpr Rectangle background_frame{0.f, 0.f, TILE_SIZE* tile_width, TILE_SIZE* tile_height};
constexpr Rectangle background_frame_px = upscale(background_frame, PIXEL_SIZE);

constexpr Rectangle gui_tile_frame{0.f, background_frame.height, 240.f, 112.f};
constexpr Rectangle tileset_tile_frame{gui_tile_frame.width, background_frame.height, 256.f, 176.f};

constexpr Rectangle gui_tile_frame_px = upscale(gui_tile_frame, PIXEL_SIZE);
constexpr Rectangle tileset_tile_frame_px = upscale(tileset_tile_frame, PIXEL_SIZE);

struct Editor {
 public:
  Editor() {
    background.preload(0, background_tile_width, background_tile_height, PIXEL_SIZE);
  }

  void update() {
    int key = GetKeyPressed();
    if (key >= KEY_ZERO && key <= KEY_FIVE) {
      background.preload(key - KEY_ZERO, background_tile_width, background_tile_height, PIXEL_SIZE);
    }

    if (IsMouseButtonDown(0)) {
      Vector2 mouse_pos = GetMousePosition();

      if (CheckCollisionPointRec(mouse_pos, background_frame_px)) {
        // Draw tile.
        IntVec2 int_coord{mod_reduced(mouse_pos.x, TILE_SIZE_PX), mod_reduced(mouse_pos.y, TILE_SIZE_PX)};
        tiles[int_coord] = tile_selection;
      } else if (CheckCollisionPointRec(mouse_pos, gui_tile_frame_px)) {
        // Pick gui tile.
        tile_selection = TileSelection{TileSource::Gui, relative_frame_pos(gui_tile_frame_px, TILE_SIZE, PIXEL_SIZE)};
      } else if (CheckCollisionPointRec(mouse_pos, tileset_tile_frame_px)) {
        // Pick tileset tile.
        tile_selection =
            TileSelection{TileSource::Tileset, relative_frame_pos(tileset_tile_frame_px, TILE_SIZE, PIXEL_SIZE)};
      }
    }

    if (IsMouseButtonDown(1)) {
      Vector2 mouse_pos = GetMousePosition();

      // Erase tile.
      if (CheckCollisionPointRec(mouse_pos, background_frame_px)) {
        IntVec2 int_coord{mod_reduced(mouse_pos.x, TILE_SIZE_PX), mod_reduced(mouse_pos.y, TILE_SIZE_PX)};
        tiles.erase(int_coord);
      }
    }

    // Save map.
    if (IsKeyPressed(KEY_S)) export_to_file();
  }

  void draw() const {
    // Background.
    background.draw({0.f, 0.f});

    // Tiles.
    for (auto const& [k, v] : tiles) v.draw(k.to_vector2(), TILE_SIZE, PIXEL_SIZE);

    // Tilesets.
    DrawTexturePro(*asset_manager.textures[TextureNames::GuiTiles],
                   {0.f, 0.f, gui_tile_frame.width, gui_tile_frame.height}, gui_tile_frame_px, Vector2Zero(), 0.f,
                   WHITE);
    DrawTexturePro(*asset_manager.textures[TextureNames::TilesetTiles],
                   {0.f, 0.f, tileset_tile_frame.width, tileset_tile_frame.height}, tileset_tile_frame_px,
                   Vector2Zero(), 0.f, WHITE);

    Vector2 mouse_pos = GetMousePosition();

    if (CheckCollisionPointRec(mouse_pos, background_frame_px)) {
      tile_selection.draw({static_cast<float>(mod_reduced(mouse_pos.x, TILE_SIZE_PX)),
                           static_cast<float>(mod_reduced(mouse_pos.y, TILE_SIZE_PX))},
                          TILE_SIZE, PIXEL_SIZE);

    } else {
      // Tile highlight.
      DrawRectangle(mod_reduced(mouse_pos.x, TILE_SIZE_PX), mod_reduced(mouse_pos.y, TILE_SIZE_PX), TILE_SIZE_PX,
                    TILE_SIZE_PX, ColorAlpha(WHITE, 0.5f));
      DrawRectangleLinesEx({static_cast<float>(mod_reduced(mouse_pos.x, TILE_SIZE_PX)),
                            static_cast<float>(mod_reduced(mouse_pos.y, TILE_SIZE_PX)),
                            static_cast<float>(TILE_SIZE_PX), static_cast<float>(TILE_SIZE_PX)},
                           PIXEL_SIZE, BLACK);
    }
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
  TileSelection tile_selection{TileSource::Gui, {0, 0}};
  std::unordered_map<IntVec2, TileSelection> tiles{};

  void export_to_file() {
    const char* filename{"assets/maps/map.mp"};

    FILE* file = std::fopen(filename, "w");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot create map file");
      return;
    }

    int values[4] = {tile_width, tile_height, background.get_current_index(), static_cast<int>(tiles.size())};
    fwrite(values, sizeof(int), 4, file);

    for (auto const& [k, v] : tiles) {
      k.write(file);
      v.write(file);
    }

    std::fclose(file);
  }
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
