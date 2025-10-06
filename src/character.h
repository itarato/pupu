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
constexpr float PLAYER_TEXTURE_SIZE_PX = TILE_SIZE_PX * 2.f;
constexpr float PLAYER_JUMP_SPEED = -1000.f;
constexpr float PLAYER_GRAVITY = 0.95f;
constexpr float PLAYER_GRAVITY_INV = 1.f / PLAYER_GRAVITY;
constexpr float PLAYER_FALL_BACK_THRESHOLD = -1.6f;
constexpr float PLAYER_MAX_FALL_SPEED = PLAYER_MAX_REL_SPEED;
constexpr float PLAYER_MULTI_JUMP_MAX = 2;

struct Character {
 public:
  void init(Vector2 new_pos) {
    pos = new_pos;
    unsigned int sprite_frame_length = static_cast<unsigned int>(GetMonitorRefreshRate(0) / 24);
    sprite_group.push_sprite(Sprite{
        PIXEL_SIZE, asset_manager.textures[TextureNames::Character1__Run], {32.f, 32.f}, 12, sprite_frame_length});
    sprite_group.push_sprite(Sprite{
        PIXEL_SIZE, asset_manager.textures[TextureNames::Character1__Idle], {32.f, 32.f}, 11, sprite_frame_length});
    sprite_group.push_sprite(Sprite{
        PIXEL_SIZE, asset_manager.textures[TextureNames::Character1__Hit], {32.f, 32.f}, 7, sprite_frame_length});
  }

  void update(Map const& map) {
    update_movement(map);
    sprite_group.update();
  }

  void draw() const {
    sprite_group.draw(pos);

    // DrawLineEx({0.f, static_cast<float>(hit_map.north)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.north)}, PIXEL_SIZE, ORANGE);
    // DrawLineEx({0.f, static_cast<float>(hit_map.south)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.south)}, PIXEL_SIZE, GREEN);
    // DrawLineEx({static_cast<float>(hit_map.west), 0.f},
    //            {static_cast<float>(hit_map.west), static_cast<float>(GetScreenHeight())}, PIXEL_SIZE, BLUE);
    // DrawLineEx({static_cast<float>(hit_map.east), 0.f},
    //            {static_cast<float>(hit_map.east), static_cast<float>(GetScreenHeight())}, PIXEL_SIZE, RED);
  }

 private:
  SpriteGroup sprite_group{};
  Vector2 pos{};
  Vector2 speed{};
  int multi_jump_count{0};

  // Debug
  HitMap hit_map{};

  void update_movement(Map const& map) {
    hit_map = calculate_hitmap(map);

    // if (IsKeyPressed(KEY_I)) {
    //   TraceLog(LOG_INFO, "Player top-left=%.2f:%.2f bottom-right=%.2f:%.2f Hitmap north=%d south%d west=%d, east=%d",
    //            top_left_corner().x, top_left_corner().y, bottom_right_corner().x, bottom_right_corner().y,
    //            hit_map.north, hit_map.south, hit_map.west, hit_map.east);
    // }

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
    float west_wall_dist = top_left_corner().x - hit_map.west;
    if (west_wall_dist < 0) {
      pos.x -= west_wall_dist;
      speed.x = 0.f;
    }
    float east_wall_dist = hit_map.east - top_right_corner().x;
    if (east_wall_dist < 0) {
      pos.x += east_wall_dist - 1.f;
      speed.x = 0.f;
    }

    if (IsKeyPressed(KEY_SPACE) && multi_jump_count < PLAYER_MULTI_JUMP_MAX) {
      speed.y = GetFrameTime() * PLAYER_JUMP_SPEED;
      multi_jump_count++;
    }

    // TODO: Figure out not calculating unnecessary directions (horizontal).
    // Recalculate in order to adjust for horizontal movement.
    hit_map = calculate_hitmap(map);

    if (speed.y < 0.f) {
      // Raising.
      fps_independent_multiply(&speed.y, PLAYER_GRAVITY);

      if (speed.y > PLAYER_FALL_BACK_THRESHOLD) {
        speed.y = 100.f * GetFrameTime();
      }
    } else if (speed.y > 0.f) {
      // Falling.
      fps_independent_multiply(&speed.y, PLAYER_GRAVITY_INV);

      if (speed.y > GetFrameTime() * PLAYER_MAX_FALL_SPEED) speed.y = GetFrameTime() * PLAYER_MAX_FALL_SPEED;
    } else {
      speed.y = 0.5f;
    }

    pos.y += speed.y;

    // Adjust for wall hit.
    float north_wall_dist = top_left_corner().y - hit_map.north;
    if (north_wall_dist < 0) {
      pos.y -= north_wall_dist;
      speed.y = 0.f;
    }
    float south_wall_dist = static_cast<float>(hit_map.south) - bottom_left_corner().y;
    // TraceLog(LOG_INFO, "Player top=%.2f bottom=%.2f Hitmap south%d", top_left_corner().y, bottom_right_corner().y,
    //          hit_map.south);
    if (south_wall_dist < 0) {
      pos.y += south_wall_dist;
      // TraceLog(LOG_INFO, "Height adjusted with %.2f to %.2f-%.2f", south_wall_dist, pos.y,
      //          pos.y + PLAYER_TEXTURE_SIZE_PX - 1.f);
      speed.y = 0.f;

      multi_jump_count = 0;
    }
  }

  float max_speed() const {
    return GetFrameTime() * PLAYER_MAX_REL_SPEED;
  }

  float speed_increments() const {
    return GetFrameTime() * PLAYER_MAX_REL_SPEED / 30.f;
  }

  Vector2 const bottom_left_corner() const {
    return Vector2{pos.x + (PLAYER_TEXTURE_SIZE_PX / 4.f), pos.y + PLAYER_TEXTURE_SIZE_PX - 1.f};
  }

  Vector2 const bottom_right_corner() const {
    return Vector2{pos.x + PLAYER_TEXTURE_SIZE_PX - 1.f - (PLAYER_TEXTURE_SIZE_PX / 4.f),
                   pos.y + PLAYER_TEXTURE_SIZE_PX - 1.f};
  }

  Vector2 const top_left_corner() const {
    return Vector2{pos.x + (PLAYER_TEXTURE_SIZE_PX / 4.f), pos.y + (PLAYER_TEXTURE_SIZE_PX / 6.f)};
  }

  Vector2 const top_right_corner() const {
    return Vector2{pos.x + PLAYER_TEXTURE_SIZE_PX - 1.f - (PLAYER_TEXTURE_SIZE_PX / 4.f),
                   pos.y + (PLAYER_TEXTURE_SIZE_PX / 6.f)};
  }

  HitMap calculate_hitmap(Map const& map) const {
    HitMap hit_map{};

    IntVec2 top_left_coord = tile_coord_from_absolute(top_left_corner());
    IntVec2 bottom_right_coord = tile_coord_from_absolute(bottom_right_corner());

    hit_map.east =
        map.east_wall_of_range(bottom_right_coord.x, top_left_coord.y, bottom_right_coord.y) * TILE_SIZE_PX - 1;
    hit_map.north = map.north_wall_of_range(top_left_coord.x, bottom_right_coord.x, top_left_coord.y) * TILE_SIZE_PX;
    hit_map.south =
        map.south_wall_of_range(top_left_coord.x, bottom_right_coord.x, bottom_right_coord.y) * TILE_SIZE_PX - 1;
    hit_map.west = map.west_wall_of_range(top_left_coord.x, top_left_coord.y, bottom_right_coord.y) * TILE_SIZE_PX;

    return hit_map;
  }
};
