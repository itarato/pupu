#include <cstdio>
#include <memory>
#include <unordered_map>

#include "asset_manager.h"
#include "background.h"
#include "common.h"
#include "editor/common.h"
#include "editor/editor.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"

/**
 * Missing:
 * - boxes
 *  - have render origin (to position)
 *  - have a hitbox
 *   - how to contribute to walls?
 * - checkpoints (collision)
 * - gems (collision)
 * - enemies (collision - moving path + MORE)
 * - traps
 */

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(1800, 800, "Pupu Level Editor");
  SetTargetFPS(30);
  rlImGuiSetup(true);

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

  rlImGuiShutdown();
  CloseWindow();
}
