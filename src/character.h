#pragma once

#include <cmath>

#include "asset_manager.h"
#include "raylib.h"
#include "sprite_group.h"

constexpr float PLAYER_MAX_REL_SPEED = 500.f;
constexpr float PLAYER_MOVEMENT_FRICTION = 0.8f;
constexpr float PLAYER_ZERO_SPEED_THRESHOLD = 0.1f;
constexpr float PLAYER_TEXTURE_HEIGHT_PX = TILE_SIZE_PX * 2;

struct Character {
 public:
  void init(Vector2 new_pos) {
    pos = new_pos;
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Run], {32.f, 32.f}, 12, 6});
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Idle], {32.f, 32.f}, 11, 6});
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Hit], {32.f, 32.f}, 7, 6});
  }

  void update() {
    update_movement();

    sprite_group.update();
  }

  void draw() const {
    sprite_group.draw(pos);
  }

 private:
  SpriteGroup sprite_group{};
  Vector2 pos{};
  Vector2 speed{};

  void update_movement() {
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
    pos.y += speed.y;
  }

  float max_speed() const {
    return GetFrameTime() * PLAYER_MAX_REL_SPEED;
  }

  float speed_increments() const {
    return GetFrameTime() * PLAYER_MAX_REL_SPEED / 30.f;
  }

  Vector2 const bottom_left_corner() const {
    return Vector2{pos.x, pos.y + PLAYER_TEXTURE_HEIGHT_PX};
  }

  Vector2 const bottom_right_corner() const {
    return Vector2{pos.x + PLAYER_TEXTURE_HEIGHT_PX, pos.y + PLAYER_TEXTURE_HEIGHT_PX};
  }
};
