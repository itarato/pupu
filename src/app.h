#pragma once

#include <memory>

#include "asset_manager.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"

struct App {
 public:
  App() {
  }
  ~App() {
    CloseWindow();
  }

  void init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "Pupu");
    SetTargetFPS(GetMonitorRefreshRate(0));

    asset_manager.preload();

    sprite.init_texture(asset_manager.textures[TextureNames::Character1__Run], {32.f, 32.f}, 12, 6);
  }

  void run() {
    while (!WindowShouldClose()) {
      update();

      BeginDrawing();

      ClearBackground(RAYWHITE);

      draw();

      EndDrawing();
    }

    asset_manager.unload_assets();
  }

 private:
  void draw() const {
    sprite.draw();
    DrawFPS(0, 0);
  }

  void update() {
    sprite.update();
  }

  Sprite sprite{{100.f, 100.f}, 3.f};
};
