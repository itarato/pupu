#pragma once

#include <cstdio>
#include <unordered_map>
#include <vector>

#include "background.h"
#include "common.h"
#include "raylib.h"

struct HitMap {
  int north{-1};
  int south{-1};
  int east{-1};
  int west{-1};
};

constexpr Vector2 const move_map[4] = {
    {0, 1.f},
    {0, -1.f},
    {1.f, 0},
    {-1.f, 0},
};

constexpr HitMap NULL_HIT_MAP{};

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

    recalculate();
  }

  void update() {
  }

  void draw() const {
    background.draw(Vector2Zero());

    for (auto const& [k, v] : tiles) v.draw(k.scale(TILE_SIZE_PX).to_vector2(), TILE_SIZE, PIXEL_SIZE);
  }

  void unload() {
    background.unload();
  }

  /**
   * Absolute window coordinates.
   */
  HitMap const& get_hit_map(Vector2 const& pos) const {
    int x = static_cast<int>(pos.x / TILE_SIZE_PX);
    int y = static_cast<int>(pos.y / TILE_SIZE_PX);
    return get_hit_map(x, y);
  }

  /**
   * Absolute window coordinates.
   */
  HitMap const& get_hit_map(int x, int y) const {
    if (x < 0 || y < 0 || x >= tile_width || y >= tile_height) {
      return NULL_HIT_MAP;
    } else {
      return hit_map[y * tile_width + x];
    }
  }

  int north_wall_of_range(int minx, int maxx, int y) const {
    int out = 0;
    for (int x = minx; x <= maxx; x++) {
      if (!is_tile_coord_valid(x, y)) continue;
      out = std::max(out, hit_map[y * tile_width + x].north);
    }
    return out;
  }

  int south_wall_of_range(int minx, int maxx, int y) const {
    int out = GetScreenHeight();
    for (int x = minx; x <= maxx; x++) {
      if (!is_tile_coord_valid(x, y)) continue;
      out = std::min(out, hit_map[y * tile_width + x].south);
    }
    return out;
  }

  int west_wall_of_range(int x, int miny, int maxy) const {
    int out = 0;
    for (int y = miny; y <= maxy; y++) {
      if (!is_tile_coord_valid(x, y)) continue;
      out = std::max(out, hit_map[y * tile_width + x].west);
    }
    return out;
  }

  int east_wall_of_range(int x, int miny, int maxy) const {
    int out = GetScreenWidth();
    for (int y = miny; y <= maxy; y++) {
      if (!is_tile_coord_valid(x, y)) continue;
      out = std::min(out, hit_map[y * tile_width + x].east);
    }
    return out;
  }

  Vector2 calculate_collision_compensation(Rectangle const victim) const {
    Vector2 total_move = vector_zero;
    Rectangle current_victim{victim};
    Rectangle attacker{0.f, 0.f, TILE_SIZE_PX, TILE_SIZE_PX};

    for (auto const& [k, v] : tiles) {
      attacker.x = k.x * TILE_SIZE_PX;
      attacker.y = k.y * TILE_SIZE_PX;

      Vector2 compensation = calculate_collision_compensation_for_rect(current_victim, attacker);
      if (!Vector2Equals(compensation, vector_zero)) {
        total_move = Vector2Add(total_move, compensation);
        current_victim.x += compensation.x;
        current_victim.y += compensation.y;
      }
    }

    return total_move;
  }

 private:
  Background background{};
  int tile_width{};
  int tile_height{};
  std::unordered_map<IntVec2, TileSelection> tiles{};
  std::vector<HitMap> hit_map{};

  void recalculate() {
    hit_map.clear();
    hit_map.resize(tile_width * tile_height, NULL_HIT_MAP);

    for (int y = 0; y < tile_height; y++) {
      int west_wall = 0;
      int east_wall = tile_width;

      for (int x = 0; x < tile_width; x++) {
        hit_map[y * tile_width + x].west = west_wall;
        if (tiles.contains(IntVec2{x, y})) west_wall = x + 1;

        hit_map[y * tile_width + (tile_width - 1 - x)].east = east_wall;
        if (tiles.contains(IntVec2{(tile_width - 1 - x), y})) east_wall = (tile_width - 1 - x);
      }
    }

    for (int x = 0; x < tile_width; x++) {
      int north_wall = 0;
      int south_wall = tile_height;

      for (int y = 0; y < tile_height; y++) {
        hit_map[y * tile_width + x].north = north_wall;
        if (tiles.contains(IntVec2{x, y})) north_wall = y + 1;

        hit_map[(tile_height - 1 - y) * tile_width + x].south = south_wall;
        if (tiles.contains(IntVec2{x, (tile_height - 1 - y)})) south_wall = (tile_height - 1 - y);
      }
    }
  }

  bool is_tile_coord_valid(int x, int y) const {
    return x >= 0 && y >= 0 && x < tile_width && y < tile_height;
  }

  Vector2 calculate_collision_compensation_for_rect(Rectangle victim, Rectangle attacker) const {
    if (!CheckCollisionRecs(victim, attacker)) return vector_zero;

    float distances[4] = {
        (attacker.y + attacker.height) - victim.y,  // top
        (victim.y + victim.height) - attacker.y,    // bottom
        (attacker.x + attacker.width) - victim.x,   // left
        (victim.x + victim.width) - attacker.x,     // right
    };

    float min_dist = std::numeric_limits<float>::max();
    int min_dist_index = -1;
    for (int i = 0; i < 4; i++) {
      if (distances[i] < min_dist) {
        min_dist = distances[i];
        min_dist_index = i;
      }
    }

    return Vector2Scale(move_map[min_dist_index], distances[min_dist_index]);
  }
};
