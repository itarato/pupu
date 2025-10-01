#pragma once

#include <memory>

#include "asset_manager.h"
#include "character.h"
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

    character.init({static_cast<float>(GetScreenWidth() / 2), static_cast<float>(GetScreenHeight() / 2)});
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
  Character character{};

  void draw() const {
    character.draw();
    DrawFPS(0, 0);
  }

  void update() {
    character.update();
  }
};
