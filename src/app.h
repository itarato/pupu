#pragma once

#include <memory>
#include <vector>

#include "asset_manager.h"
#include "character.h"
#include "checkpoint.h"
#include "gem.h"
#include "interactive_group.h"
#include "map.h"
#include "npc.h"
#include "pointer.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"
#include "trap.h"

int GAME_FPS{};

struct App {
 public:
  void init(char* map_filename = nullptr) {
    SetTraceLogLevel(LOG_DEBUG);

    InitWindow(1024, 768, "Pupu");
    InitAudioDevice();

    GAME_FPS = GetMonitorRefreshRate(0);
    FPS_MULTIPLIER = static_cast<float>(REFERENCE_FPS) / static_cast<float>(GAME_FPS);
    SetTargetFPS(GAME_FPS);

    asset_manager.preload();
    character.init();

    force_map_filename = map_filename;

    reset();
  }

  void run() {
    PlayMusicStream(*asset_manager.musics[MusicName::Soundtrack]);

    while (!WindowShouldClose()) {
      update();

      BeginDrawing();

      ClearBackground(RAYWHITE);

      draw();

      EndDrawing();
    }

    map.unload();
    asset_manager.unload_assets();

    CloseAudioDevice();
    CloseWindow();
  }

 private:
  bool pause_update{false};
  Map map{DEFAULT_PIXEL_SIZE};
  int pixel_size{DEFAULT_PIXEL_SIZE};
  Character character{DEFAULT_PIXEL_SIZE};
  std::vector<std::shared_ptr<Npc>> npcs{};
  std::vector<std::shared_ptr<Trap>> traps{};
  std::list<Gem> gems{};
  std::vector<Checkpoint> checkpoints{};
  std::vector<Pointer> pointers{};
  size_t map_index{0};
  char* force_map_filename{nullptr};

  void reset() {
    npcs.clear();
    traps.clear();
    gems.clear();
    checkpoints.clear();
    pointers.clear();
    pause_update = false;

    reload_world_from_file();
  }

  void reload_world_from_file() {
    FILE* file = std::fopen(current_map_filename(), "r");
    if (!file) BAIL;

    int version{};
    int background_index{};
    int tiles_count{};
    int tile_width{};
    int tile_height{};
    int interactive_object_count{};

    if (std::fread(&version, sizeof(int), 1, file) != 1) BAIL;
    if (version != 10) BAIL;

    if (std::fread(&tile_width, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tile_height, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&background_index, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tiles_count, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&interactive_object_count, sizeof(int), 1, file) != 1) BAIL;

    character.reset(intvec2_from_file(file).scale(pixel_size).to_vector2());

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
        case TileSource::Trap5:
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
        case TileSource::Trap4:
          traps.push_back(std::make_shared<SpikeTrap>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        case TileSource::Trap6:
          traps.push_back(std::make_shared<ShockTowerTrap>(tile_pos.scale(pixel_size).to_vector2(), pixel_size));
          break;
        case TileSource::Gem1:
        case TileSource::Gem2:
        case TileSource::Gem3:
        case TileSource::Gem4:
        case TileSource::Gem5:
        case TileSource::Gem6:
          gems.emplace_back(pixel_size, tile_pos.scale(pixel_size).to_vector2(), tile_selection.source);
          break;
        case TileSource::Checkpoint:
          checkpoints.emplace_back(pixel_size, tile_pos.scale(pixel_size).to_vector2());
          break;
        case TileSource::Pointer:
          pointers.emplace_back(pixel_size, tile_pos.scale(pixel_size).to_vector2());
          break;
        default:
          BAILF("Invalid: %d", tile_selection.source);
      }
    }

    std::vector<InteractiveGroup> interactive_groups{};
    for (int i = 0; i < interactive_object_count; i++) {
      interactive_groups.push_back(interactive_group_from_file(file));
    }

    std::fclose(file);

    map.reload_world(background_index, tile_width, tile_height, std::move(map_tiles), std::move(interactive_groups));
  }

  void draw() const {
    map.draw();
    for (auto const& npc : npcs) npc->draw();
    for (auto const& trap : traps) trap->draw();
    for (auto const& checkpoint : checkpoints) checkpoint.draw();
    for (auto const& pointer : pointers) pointer.draw();
    character.draw();
    for (auto const& gem : gems) gem.draw();

    // DrawFPS(0, 0);
  }

  void update() {
    UpdateMusicStream(*asset_manager.musics[MusicName::Soundtrack]);

    if (character.ready_for_next_level()) {
      load_next_level();
      return;
    }

    if (!pause_update || IsKeyPressed(KEY_S)) {
      map.update(character.hitbox());
      for (auto& npc : npcs) npc->update(map, character);
      for (auto& trap : traps) trap->update(map, character);
      for (auto& gem : gems) gem.update();
      for (auto& checkpoint : checkpoints) checkpoint.update();
      for (auto& pointer : pointers) pointer.update();
      character.update(map);

      update__character_collisions();
    }

    if (IsKeyPressed(KEY_P)) pause_update = !pause_update;

    if (IsKeyPressed(KEY_R)) reset();
  }

  void update__character_collisions() {
    Rectangle const character_hitbox{character.hitbox()};

    for (auto& npc : npcs) {
      Rectangle npc_hitbox{npc->hitbox()};
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

    bool consumed_gem{false};
    for (auto& gem : gems) {
      if (CheckCollisionRecs(gem.hitbox(), character_hitbox)) {
        gem.consume();
        consumed_gem = true;
      }
    }
    if (consumed_gem) {
      std::erase_if(gems, [](auto const& gem) { return gem.is_consumed(); });
      if (gems.empty()) {
        for (auto& checkpoint : checkpoints) {
          checkpoint.victory();
        }
      }
    }

    for (auto& checkpoint : checkpoints) {
      if (CheckCollisionRecs(checkpoint.hitbox(), character_hitbox)) {
        checkpoint.touch();
      }
    }

    for (auto const& pointer : pointers) {
      if (CheckCollisionRecs(pointer.hitbox(), character_hitbox)) {
        character.set_level_complete();
      }
    }
  }

  void load_next_level() {
    // Change to the next map.
    size_t map_count = sizeof(MAP_FILENAMES) / sizeof(MAP_FILENAMES[0]);
    map_index = (map_index + 1) % map_count;
    reset();
  }

  const char* current_map_filename() {
    if (force_map_filename == nullptr) {
      return MAP_FILENAMES[map_index];
    } else {
      return force_map_filename;
    }
  }
};
