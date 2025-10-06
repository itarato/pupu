#pragma once

#include <memory>

#include "asset_manager.h"
#include "character.h"
#include "map.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"

struct App {
 public:
  App() {
  }

  void init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "Pupu");
    SetTargetFPS(GetMonitorRefreshRate(0));

    asset_manager.preload();
    character.init({static_cast<float>(GetScreenWidth() / 2), static_cast<float>(GetScreenHeight() / 3)});
    map.reload_from_file();
  }

  void run() {
    while (!WindowShouldClose()) {
      update();

      BeginDrawing();

      ClearBackground(RAYWHITE);

      draw();

      EndDrawing();
    }

    map.unload();
    asset_manager.unload_assets();

    CloseWindow();
  }

 private:
  Map map{};
  Character character{};

  void draw() const {
    map.draw();
    character.draw();

    // HitMap hit_map = map.get_hit_map(GetMousePosition());
    // DrawLineEx({0.f, static_cast<float>(hit_map.north * TILE_SIZE_PX)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.north * TILE_SIZE_PX)}, PIXEL_SIZE,
    //            ORANGE);
    // DrawLineEx({0.f, static_cast<float>(hit_map.south * TILE_SIZE_PX)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.south * TILE_SIZE_PX)}, PIXEL_SIZE,
    //            GREEN);
    // DrawLineEx({static_cast<float>(hit_map.west * TILE_SIZE_PX), 0.f},
    //            {static_cast<float>(hit_map.west * TILE_SIZE_PX), static_cast<float>(GetScreenHeight())}, PIXEL_SIZE,
    //            BLUE);
    // DrawLineEx({static_cast<float>(hit_map.east * TILE_SIZE_PX), 0.f},
    //            {static_cast<float>(hit_map.east * TILE_SIZE_PX), static_cast<float>(GetScreenHeight())}, PIXEL_SIZE,
    //            RED);

    DrawFPS(0, 0);
  }

  void update() {
    map.update();
    character.update(map);
  }
};
