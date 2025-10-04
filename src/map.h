#pragma once

#include <cstdio>
#include <unordered_map>

#include "background.h"
#include "common.h"
#include "raylib.h"

struct Map {
 public:
  void reload_from_file() {
    FILE* file = std::fopen("assets/maps/map.mp", "r");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot open map file");
      exit(EXIT_FAILURE);
    }

    int background_index{};
    int tiles_count{};
    if (std::fread(&tile_width, sizeof(int), 1, file) != 1) PANIC("Failed reading input: tile width");
    if (std::fread(&tile_height, sizeof(int), 1, file) != 1) PANIC("Failed reading input: tile height");
    if (std::fread(&background_index, sizeof(int), 1, file) != 1) PANIC("Failed reading input: background index");
    if (std::fread(&tiles_count, sizeof(int), 1, file) != 1) PANIC("Failed reading input: tile count");

    SetWindowSize(tile_width * TILE_SIZE_PX, tile_height * TILE_SIZE_PX);

    background.preload(background_index, tile_width, tile_height, PIXEL_SIZE);

    tiles.clear();

    for (int i = 0; i < tiles_count; i++) {
      IntVec2 tile_pos = intvec2_from_file(file);
      TileSelection tile_selection{tile_selection_from_file(file)};
      tiles[tile_pos] = tile_selection;
    }

    std::fclose(file);
  }

  void update() {
  }

  void draw() const {
    background.draw(Vector2Zero());

    for (auto const& [k, v] : tiles) v.draw(k.to_vector2(), TILE_SIZE, PIXEL_SIZE);
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
  int tile_width{};
  int tile_height{};
  std::unordered_map<IntVec2, TileSelection> tiles{};
};
