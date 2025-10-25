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
 * - checkpoints (collision)
 * - gems (collision)
 * - enemies (collision - moving path + MORE)
 * - traps
 */

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(1800, 1200, "Pupu Level Editor");
  SetTargetFPS(30);
  rlImGuiSetup(true);

  asset_manager.preload();

  Editor editor{};
  editor.load_from_file();

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
