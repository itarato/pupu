#pragma once

#include <cstdio>
#include <memory>
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
constexpr float const WALL_CHECK_THRESHOLD{3.f};

struct Map {
 public:
  Map(int const pixel_size) : pixel_size(pixel_size) {
  }

  void reload_world(int background_index, int new_tile_width, int new_tile_height,
                    std::unordered_map<IntVec2, TileSelection>&& tiles) {
    reset();

    tile_width = new_tile_width;
    tile_height = new_tile_height;
    background.preload(background_index, new_tile_width, new_tile_height, pixel_size);

    for (auto&& [tile_pos, tile_selection] : tiles) {
      switch (tile_selection.source) {
        case TileSource::Gui:
        case TileSource::Tileset:
          walls[tile_pos] = tile_selection;
          break;
        case TileSource::Box1:
        case TileSource::Box2:
        case TileSource::Box3:
          boxes[tile_pos] = tile_selection;
          break;
        default:
          BAIL;
      }
    }

    recalculate();
  }

  void update() {
  }

  void draw() const {
    background.draw(Vector2Zero(), pixel_size);

    for (auto const& [k, v] : walls) v.draw(k.scale(pixel_size).to_vector2(), pixel_size);
    for (auto const& [k, v] : boxes) v.draw(k.scale(pixel_size).to_vector2(), pixel_size);
  }

  void unload() {
    background.unload();
  }

  int north_wall_of_range(Rectangle const& rect) const {
    int minx = leftx(rect) / (TILE_SIZE * pixel_size);
    int maxx = rightx(rect) / (TILE_SIZE * pixel_size);
    int y = topy(rect) / (TILE_SIZE * pixel_size);

    int max_y_coord = 0;
    for (int x = minx; x <= maxx; x++) {
      if (!is_tile_coord_valid(x, y)) continue;
      max_y_coord = std::max(max_y_coord, hit_map[y * tile_width + x].north);
    }

    int out = max_y_coord * TILE_SIZE * pixel_size;

    for (auto const& [pos, selection] : boxes) {
      Rectangle abs_hitbox = upscale(selection.hitbox(pos), pixel_size);
      if (is_horizontal_overlap(abs_hitbox, rect)) {
        if (bottomy(abs_hitbox) > out && (bottomy(abs_hitbox) - WALL_CHECK_THRESHOLD * pixel_size) <= topy(rect)) {
          out = bottomy(abs_hitbox);
        }
      }
    }

    return out;
  }

  int south_wall_of_range(Rectangle const& rect) const {
    int minx = leftx(rect) / (TILE_SIZE * pixel_size);
    int maxx = rightx(rect) / (TILE_SIZE * pixel_size);
    int y = bottomy(rect) / (TILE_SIZE * pixel_size);

    int min_y_coord = tile_height;
    for (int x = minx; x <= maxx; x++) {
      if (!is_tile_coord_valid(x, y)) continue;
      min_y_coord = std::min(min_y_coord, hit_map[y * tile_width + x].south);
    }

    int out = min_y_coord * TILE_SIZE * pixel_size - 1;

    for (auto const& [pos, selection] : boxes) {
      Rectangle abs_hitbox = upscale(selection.hitbox(pos), pixel_size);
      if (is_horizontal_overlap(abs_hitbox, rect)) {
        if (topy(abs_hitbox) < out && (topy(abs_hitbox) + WALL_CHECK_THRESHOLD * pixel_size) >= bottomy(rect)) {
          out = abs_hitbox.y - 1;
          // TraceLog(LOG_INFO,
          //          "South wall detected at %d. Hitbox: x=%.2f y=%.2f w=%.2f h=%.2f Victim: minx=%d max=%d y=%d", out,
          //          abs_hitbox.x, abs_hitbox.y, abs_hitbox.width, abs_hitbox.height, abs_minx, abs_maxx, abs_y);
        }
      }
    }

    return out;
  }

  int west_wall_of_range(Rectangle const& rect) const {
    int x = leftx(rect) / (TILE_SIZE * pixel_size);
    int miny = topy(rect) / (TILE_SIZE * pixel_size);
    int maxy = bottomy(rect) / (TILE_SIZE * pixel_size);

    int max_x_coord = 0;
    for (int y = miny; y <= maxy; y++) {
      if (!is_tile_coord_valid(x, y)) continue;
      max_x_coord = std::max(max_x_coord, hit_map[y * tile_width + x].west);
    }

    int out = max_x_coord * TILE_SIZE * pixel_size;

    for (auto const& [pos, selection] : boxes) {
      Rectangle abs_hitbox = upscale(selection.hitbox(pos), pixel_size);
      if (is_vertical_overlap(abs_hitbox, rect)) {
        if (rightx(abs_hitbox) > out && (rightx(abs_hitbox) - WALL_CHECK_THRESHOLD * pixel_size) <= leftx(rect)) {
          out = rightx(abs_hitbox);
        }
      }
    }

    return out;
  }

  int east_wall_of_range(Rectangle const& rect) const {
    int x = leftx(rect) / (TILE_SIZE * pixel_size);
    int miny = topy(rect) / (TILE_SIZE * pixel_size);
    int maxy = bottomy(rect) / (TILE_SIZE * pixel_size);

    int min_x_coord = tile_width;
    for (int y = miny; y <= maxy; y++) {
      if (!is_tile_coord_valid(x, y)) continue;
      min_x_coord = std::min(min_x_coord, hit_map[y * tile_width + x].east);
    }

    int out = min_x_coord * TILE_SIZE * pixel_size - 1;

    for (auto const& [pos, selection] : boxes) {
      Rectangle abs_hitbox = upscale(selection.hitbox(pos), pixel_size);
      if (is_vertical_overlap(abs_hitbox, rect)) {
        if (leftx(abs_hitbox) < out && (leftx(abs_hitbox) + WALL_CHECK_THRESHOLD * pixel_size) >= leftx(rect)) {
          out = abs_hitbox.x - 1;
          // TraceLog(LOG_INFO,
          //          "East wall detected at %d. Hitbox: x=%.2f y=%.2f w=%.2f h=%.2f Victim: x=%d miny=%d maxy=%d", out,
          //          abs_hitbox.x, abs_hitbox.y, abs_hitbox.width, abs_hitbox.height, abs_x, abs_miny, abs_maxy);
        }
      }
    }

    return out;
  }

 private:
  Background background{};
  int tile_width{};
  int tile_height{};
  std::unordered_map<IntVec2, TileSelection> walls{};
  std::unordered_map<IntVec2, TileSelection> boxes{};
  std::vector<HitMap> hit_map{};
  int const pixel_size;

  void reset() {
    walls.clear();
    hit_map.clear();
  }

  void recalculate() {
    hit_map.clear();
    hit_map.resize(tile_width * tile_height, NULL_HIT_MAP);

    for (int y = 0; y < tile_height; y++) {
      int west_wall = 0;
      int east_wall = tile_width;

      for (int x = 0; x < tile_width; x++) {
        hit_map[y * tile_width + x].west = west_wall;
        IntVec2 coord{x * TILE_SIZE, y * TILE_SIZE};
        if (walls.contains(coord) && walls[coord].collide_from(COLLISION_TYPE_LEFT)) west_wall = x + 1;

        hit_map[y * tile_width + (tile_width - 1 - x)].east = east_wall;
        IntVec2 coord_inv{(tile_width - 1 - x) * TILE_SIZE, y * TILE_SIZE};
        if (walls.contains(coord_inv) && walls[coord_inv].collide_from(COLLISION_TYPE_RIGHT))
          east_wall = (tile_width - 1 - x);
      }
    }

    for (int x = 0; x < tile_width; x++) {
      int north_wall = 0;
      int south_wall = tile_height;

      for (int y = 0; y < tile_height; y++) {
        hit_map[y * tile_width + x].north = north_wall;
        IntVec2 coord{x * TILE_SIZE, y * TILE_SIZE};
        if (walls.contains(coord) && walls[coord].collide_from(COLLISION_TYPE_BOTTOM)) north_wall = y + 1;

        hit_map[(tile_height - 1 - y) * tile_width + x].south = south_wall;
        IntVec2 coord_inv{x * TILE_SIZE, (tile_height - 1 - y) * TILE_SIZE};
        if (walls.contains(coord_inv) && walls[coord_inv].collide_from(COLLISION_TYPE_TOP))
          south_wall = (tile_height - 1 - y);
      }
    }
  }

  bool is_tile_coord_valid(int x, int y) const {
    return x >= 0 && y >= 0 && x < tile_width && y < tile_height;
  }
};
