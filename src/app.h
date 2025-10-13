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
    InitWindow(1024, 768, "Pupu");
    SetTargetFPS(GetMonitorRefreshRate(0));

    asset_manager.preload();
    character.init();

    reset();
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
  bool pause_update{false};
  Map map{DEFAULT_PIXEL_SIZE};
  int pixel_size{DEFAULT_PIXEL_SIZE};
  Character character{DEFAULT_PIXEL_SIZE};

  void reset() {
    map.reload_from_file();
    character.reset({static_cast<float>(GetScreenWidth() / 3), static_cast<float>(TILE_SIZE * 4 * pixel_size)});
    pause_update = false;
  }

  void draw() const {
    map.draw();
    character.draw();

    // HitMap hit_map = map.get_hit_map(GetMousePosition());
    // DrawLineEx({0.f, static_cast<float>(hit_map.north * TILE_SIZE_PX)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.north * TILE_SIZE_PX)}, pixel_size,
    //            ORANGE);
    // DrawLineEx({0.f, static_cast<float>(hit_map.south * TILE_SIZE_PX)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.south * TILE_SIZE_PX)}, pixel_size,
    //            GREEN);
    // DrawLineEx({static_cast<float>(hit_map.west * TILE_SIZE_PX), 0.f},
    //            {static_cast<float>(hit_map.west * TILE_SIZE_PX), static_cast<float>(GetScreenHeight())}, pixel_size,
    //            BLUE);
    // DrawLineEx({static_cast<float>(hit_map.east * TILE_SIZE_PX), 0.f},
    //            {static_cast<float>(hit_map.east * TILE_SIZE_PX), static_cast<float>(GetScreenHeight())}, pixel_size,
    //            RED);

    DrawFPS(0, 0);
  }

  void update() {
    if (!pause_update) {
      map.update();
      character.update(map);
    }

    if (IsKeyPressed(KEY_P)) pause_update = !pause_update;

    if (IsKeyPressed(KEY_R)) reset();
  }
};
