#include "asset_manager.h"
#include "background.h"
#include "raylib.h"

constexpr int pixel_size{3};
constexpr int tile_size{16};
constexpr int tile_width{32};
constexpr int tile_height{20};
constexpr int background_tile_width{tile_width / 4};
constexpr int background_tile_height{tile_height / 4};

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
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
};

int main() {
  InitWindow(tile_width * pixel_size * tile_size, tile_height * pixel_size * tile_size + 300, "Pupu Level Editor");
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
