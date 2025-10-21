#pragma once

#include <memory>
#include <vector>

#include "asset_manager.h"
#include "character.h"
#include "map.h"
#include "npc.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"
#include "trap.h"

struct App {
 public:
  void init() {
    SetTraceLogLevel(LOG_DEBUG);

    InitWindow(1024, 768, "Pupu");

    GameFPS = GetMonitorRefreshRate(0);
    FPSMultiplier = static_cast<float>(ReferenceFPS) / static_cast<float>(GameFPS);
    SetTargetFPS(GameFPS);

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
  std::vector<std::shared_ptr<Npc>> npcs{};
  std::vector<std::shared_ptr<Trap>> traps{};

  void reset() {
    character.reset({static_cast<float>(GetScreenWidth() / 3), static_cast<float>(TILE_SIZE * 4 * pixel_size)});
    npcs.clear();
    traps.clear();
    pause_update = false;

    reload_world_from_file();
  }

  void reload_world_from_file() {
    FILE* file = std::fopen("assets/maps/map.mp", "r");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot open map file");
      exit(EXIT_FAILURE);
    }

    int background_index{};
    int tiles_count{};
    int tile_width{};
    int tile_height{};

    if (std::fread(&tile_width, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tile_height, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&background_index, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tiles_count, sizeof(int), 1, file) != 1) BAIL;

    character.set_position(intvec2_from_file(file));

    SetWindowSize(tile_width * TILE_SIZE * pixel_size, tile_height * TILE_SIZE * pixel_size);

    std::unordered_map<IntVec2, TileSelection> map_tiles{};
    for (int i = 0; i < tiles_count; i++) {
      IntVec2 tile_pos = intvec2_from_file(file);
      TileSelection tile_selection{tile_selection_from_file(file)};

      switch (tile_selection.source) {
        case TileSource::Gui:
        case TileSource::Tileset:
        case TileSource::Box1:
        case TileSource::Box2:
        case TileSource::Box3:
          map_tiles[tile_pos] = tile_selection;
          break;
        case TileSource::Enemy1:
        case TileSource::Enemy2:
          npcs.push_back(std::make_shared<SimpleWalkNpc>(tile_pos, tile_selection.source, pixel_size));
          break;
        case TileSource::Enemy3:
          npcs.push_back(std::make_shared<ChargingNpc>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        case TileSource::Enemy4:
          npcs.push_back(std::make_shared<ShootingNpc>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        case TileSource::Enemy5:
          npcs.push_back(std::make_shared<StompingNpc>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        case TileSource::Trap1:
          traps.push_back(std::make_shared<BouncingTrap>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        case TileSource::Trap2:
          traps.push_back(std::make_shared<CircleSawTrap>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        default:
          BAILF("Invalid: %d", tile_selection.source);
      }
    }

    std::fclose(file);

    map.reload_world(background_index, tile_width, tile_height, std::move(map_tiles));
  }

  void draw() const {
    map.draw();
    for (auto const& npc : npcs) npc->draw();
    for (auto const& trap : traps) trap->draw();
    character.draw();

    DrawFPS(0, 0);
  }

  void update() {
    if (!pause_update) {
      map.update();
      for (auto& npc : npcs) npc->update(map, character);
      for (auto& trap : traps) trap->update(map, character);
      character.update(map);

      update__character_collisions();
    }

    if (IsKeyPressed(KEY_P)) pause_update = !pause_update;

    if (IsKeyPressed(KEY_R)) reset();
  }

  void update__character_collisions() {
    for (auto& npc : npcs) {
      Rectangle npc_hitbox{npc->hitbox()};
      Rectangle character_hitbox{character.hitbox()};

      if (CheckCollisionRecs(npc_hitbox, character_hitbox)) {
        if (character.is_falling()) {
          if (!npc->is_injured()) {
            npc->injure();
            character.enemy_head_bounce();
          }
        } else {
          if (!npc->is_injured()) {
            character.injure();
          }
        }
      }
    }
  }
};
