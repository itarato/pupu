#pragma once

#include <algorithm>
#include <cmath>

#include "asset_manager.h"
#include "map.h"
#include "raylib.h"
#include "sprite_group.h"

constexpr float PLAYER_MAX_REL_SPEED = 500.f;
constexpr float PLAYER_MOVEMENT_FRICTION = 0.8f;
constexpr float PLAYER_ZERO_SPEED_THRESHOLD = 0.1f;
constexpr float PLAYER_TEXTURE_HEIGHT_PX = TILE_SIZE_PX * 2.f;

struct Character {
 public:
  void init(Vector2 new_pos) {
    pos = new_pos;
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Run], {32.f, 32.f}, 12, 6});
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Idle], {32.f, 32.f}, 11, 6});
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Hit], {32.f, 32.f}, 7, 6});
  }

  void update(Map const& map) {
    update_movement(map);

    sprite_group.update();
  }

  void draw() const {
    sprite_group.draw(pos);

    DrawLineEx({0.f, static_cast<float>(hit_map.north)},
               {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.north)}, PIXEL_SIZE, ORANGE);
    DrawLineEx({0.f, static_cast<float>(hit_map.south)},
               {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.south)}, PIXEL_SIZE, GREEN);
    DrawLineEx({static_cast<float>(hit_map.west), 0.f},
               {static_cast<float>(hit_map.west), static_cast<float>(GetScreenHeight())}, PIXEL_SIZE, BLUE);
    DrawLineEx({static_cast<float>(hit_map.east), 0.f},
               {static_cast<float>(hit_map.east), static_cast<float>(GetScreenHeight())}, PIXEL_SIZE, RED);
  }

 private:
  SpriteGroup sprite_group{};
  Vector2 pos{};
  Vector2 speed{};

  // Debug
  HitMap hit_map{};

  void update_movement(Map const& map) {
    hit_map = calculate_hitmap(map);

    if (IsKeyDown(KEY_LEFT)) {
      sprite_group.horizontal_flip();
      sprite_group.set_current_sprite(0);
      speed.x -= speed_increments();

      if (speed.x < -max_speed()) speed.x = -max_speed();
    } else if (IsKeyDown(KEY_RIGHT)) {
      sprite_group.horizontal_reset();
      sprite_group.set_current_sprite(0);
      speed.x += speed_increments();
      if (speed.x > max_speed()) speed.x = max_speed();
    } else {
      sprite_group.set_current_sprite(1);
      speed.x *= PLAYER_MOVEMENT_FRICTION;

      if (fabs(speed.x) < PLAYER_ZERO_SPEED_THRESHOLD) speed.x = 0.f;
    }

    pos.x += speed.x;

    // Adjust for wall hit.
    float west_wall_dist = pos.x - hit_map.west;
    if (west_wall_dist < 0) pos.x -= west_wall_dist;
    float east_wall_dist = hit_map.east - top_right_corner().x;
    if (east_wall_dist < 0) pos.x += east_wall_dist - 1.f;

    pos.y += speed.y;
  }

  float max_speed() const {
    return GetFrameTime() * PLAYER_MAX_REL_SPEED;
  }

  float speed_increments() const {
    return GetFrameTime() * PLAYER_MAX_REL_SPEED / 30.f;
  }

  Vector2 const bottom_left_corner() const {
    return Vector2{pos.x, pos.y + PLAYER_TEXTURE_HEIGHT_PX - 1.f};
  }

  Vector2 const bottom_right_corner() const {
    return Vector2{pos.x + PLAYER_TEXTURE_HEIGHT_PX, pos.y + PLAYER_TEXTURE_HEIGHT_PX - 1.f};
  }

  Vector2 const top_left_corner() const {
    return Vector2{pos.x, pos.y};
  }

  Vector2 const top_right_corner() const {
    return Vector2{pos.x + PLAYER_TEXTURE_HEIGHT_PX, pos.y};
  }

  HitMap calculate_hitmap(Map const& map) const {
    HitMap hit_map{};

    HitMap top_left = map.get_hit_map(top_left_corner());
    HitMap top_right = map.get_hit_map(top_right_corner());
    HitMap bottom_left = map.get_hit_map(bottom_left_corner());
    HitMap bottom_right = map.get_hit_map(bottom_right_corner());

    hit_map.east = std::max(top_right.east, bottom_right.east) * TILE_SIZE_PX;
    hit_map.west = std::min(top_left.west, bottom_left.west) * TILE_SIZE_PX;
    hit_map.north = std::max(top_left.north, top_right.north) * TILE_SIZE_PX;
    hit_map.south = std::min(bottom_left.south, bottom_right.south) * TILE_SIZE_PX;

    return hit_map;
  }
};
