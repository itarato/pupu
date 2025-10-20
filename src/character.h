#pragma once

#include <algorithm>
#include <cmath>

#include "asset_manager.h"
#include "map.h"
#include "raylib.h"
#include "sprite_group.h"

constexpr int PLAYER_TEXTURE_SIZE{32};

constexpr float PLAYER_MAX_REL_SPEED = 500.f;
constexpr float PLAYER_MOVEMENT_FRICTION = 0.8f;
constexpr float PLAYER_ZERO_SPEED_THRESHOLD = 0.1f;
constexpr float PLAYER_JUMP_SPEED = -1000.f;
constexpr float PLAYER_GRAVITY = 0.95f;
constexpr float PLAYER_GRAVITY_INV = 1.f / PLAYER_GRAVITY;
constexpr float PLAYER_FALL_BACK_THRESHOLD = -1.6f;
constexpr float PLAYER_MAX_FALL_SPEED = PLAYER_MAX_REL_SPEED;
constexpr float PLAYER_MULTI_JUMP_MAX = 2;

constexpr int PLAYER_SPRITE_RUN{0};
constexpr int PLAYER_SPRITE_IDLE{1};
constexpr int PLAYER_SPRITE_HIT{2};
constexpr int PLAYER_SPRITE_JUMP{3};
constexpr int PLAYER_SPRITE_FALL{4};
constexpr int PLAYER_SPRITE_DOUBLE_JUMP{5};
constexpr int PLAYER_SPRITE_WALL_JUMP{6};

enum class JumpState {
  Ground,
  Jump,
  Fall,
  DoubleJump,
};

enum class LifecycleState {
  Appear,
  Live,
  Injured,
};

struct Character {
 public:
  Character(int const pixel_size) : pixel_size(pixel_size) {
  }

  void reset(Vector2 new_pos) {
    pos = new_pos;
    sprite_group.reset();
    jump_state = JumpState::Ground;
    lifecycle_state = LifecycleState::Appear;
    multi_jump_count = PLAYER_MULTI_JUMP_MAX - 1;
  }

  void init() {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);

    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Run],
                                    {32.f, 32.f},
                                    12,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Idle],
                                    {32.f, 32.f},
                                    11,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Hit],
                                    {32.f, 32.f},
                                    7,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{
        static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Character1__Jump], {32.f, 32.f}, 1, 0});
    sprite_group.push_sprite(Sprite{
        static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Character1__Fall], {32.f, 32.f}, 1, 0});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Double_Jump],
                                    {32.f, 32.f},
                                    6,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Wall_Jump],
                                    {32.f, 32.f},
                                    5,
                                    sprite_frame_length});

    appear_sprite.init_texture(asset_manager.textures[TextureNames::Character__Appear], {96.f, 96.f}, 7,
                               sprite_frame_length);
  }

  void update(Map const& map) {
    if (lifecycle_state == LifecycleState::Appear) {
      if (appear_sprite.update() == 0) lifecycle_state = LifecycleState::Live;
    } else if (lifecycle_state == LifecycleState::Live || lifecycle_state == LifecycleState::Injured) {
      update_movement(map);
      sprite_group.update();
    } else {
      TraceLog(LOG_ERROR, "Invalid lifecycle state");
    }

    injury_timeout.update();
  }

  void draw() const {
    if (lifecycle_state == LifecycleState::Appear) {
      appear_sprite.draw(pos);
    } else {
      sprite_group.draw(pos);
    }
  }

  bool is_falling() const {
    return jump_state == JumpState::Fall;
  }

  Rectangle const hitbox() const {
    return move(upscale(CHARACTER_HITBOX, pixel_size), pos);
  }

  void injure() {
    if (is_injured()) return;

    injury_timeout.cancel();
    lifecycle_state = LifecycleState::Injured;
    sprite_group.set_current_sprite(PLAYER_SPRITE_HIT);

    injury_timeout.set_on_timeout([&]() { end_injury(); }, 2.f);
  }

  void enemy_head_bounce() {
    speed.y = GetFrameTime() * PLAYER_JUMP_SPEED / 1.5f;
    jump_state = JumpState::Jump;
  }

  void set_position(IntVec2 const& v) {
    pos.x = static_cast<float>(v.x);
    pos.y = static_cast<float>(v.y);
  }

  bool is_injured() const {
    return lifecycle_state == LifecycleState::Injured;
  }

 private:
  const int pixel_size{DEFAULT_PIXEL_SIZE};
  SpriteGroup sprite_group{};
  Sprite appear_sprite{};
  Vector2 pos{};
  Vector2 speed{};
  int multi_jump_count{0};
  JumpState jump_state{JumpState::Ground};
  LifecycleState lifecycle_state{LifecycleState::Appear};
  Timeout injury_timeout{};

  void update_movement(Map const& map) {
    HitMap hit_map = calculate_hitmap(map);
    bool is_grab_wall{false};

    if (!is_injured() && IsKeyDown(KEY_LEFT)) {
      sprite_group.horizontal_flip();
      sprite_group.set_current_sprite(PLAYER_SPRITE_RUN);
      speed.x -= speed_increments();

      if (speed.x < -PLAYER_MAX_REL_SPEED) speed.x = -PLAYER_MAX_REL_SPEED;
    } else if (!is_injured() && IsKeyDown(KEY_RIGHT)) {
      sprite_group.horizontal_reset();
      sprite_group.set_current_sprite(PLAYER_SPRITE_RUN);
      speed.x += speed_increments();

      if (speed.x > PLAYER_MAX_REL_SPEED) speed.x = PLAYER_MAX_REL_SPEED;
    } else {
      sprite_group.set_current_sprite(PLAYER_SPRITE_IDLE);
      speed.x *= PLAYER_MOVEMENT_FRICTION;

      if (fabs(speed.x) < PLAYER_ZERO_SPEED_THRESHOLD) speed.x = 0.f;
    }

    pos.x += speed.x * GetFrameTime();
    Rectangle _hitbox{hitbox()};

    // Adjust for wall hit.
    float west_wall_dist = leftx(_hitbox) - hit_map.west;
    if (west_wall_dist < 0) {
      pos.x -= west_wall_dist;
      speed.x = 0.f;
      is_grab_wall = true;
      multi_jump_count = PLAYER_MULTI_JUMP_MAX - 1;
    }
    float east_wall_dist = hit_map.east - rightx(_hitbox);
    if (east_wall_dist < 0) {
      pos.x += east_wall_dist;
      speed.x = 0.f;
      is_grab_wall = true;
      multi_jump_count = PLAYER_MULTI_JUMP_MAX - 1;
    }

    if (!is_injured() && IsKeyPressed(KEY_SPACE) && multi_jump_count < PLAYER_MULTI_JUMP_MAX) {
      speed.y = GetFrameTime() * PLAYER_JUMP_SPEED;
      multi_jump_count++;
      if (multi_jump_count == 1) {
        jump_state = JumpState::Jump;
      } else {
        jump_state = JumpState::DoubleJump;
      }
    }

    hit_map = calculate_hitmap(map);

    if (speed.y < 0.f) {
      // Raising.
      fps_independent_multiply(&speed.y, PLAYER_GRAVITY);

      if (speed.y > PLAYER_FALL_BACK_THRESHOLD) {
        speed.y = 100.f * GetFrameTime();
        jump_state = JumpState::Fall;
      }
    } else if (speed.y > 0.f) {
      // Falling.
      fps_independent_multiply(&speed.y, PLAYER_GRAVITY_INV);

      if (speed.y > GetFrameTime() * PLAYER_MAX_FALL_SPEED) speed.y = GetFrameTime() * PLAYER_MAX_FALL_SPEED;
      if (is_grab_wall && speed.y > GetFrameTime() * PLAYER_MAX_FALL_SPEED / 5.f)
        speed.y = GetFrameTime() * PLAYER_MAX_FALL_SPEED / 5.f;
      jump_state = JumpState::Fall;
    } else {
      jump_state = JumpState::Ground;
      speed.y = 0.5f;
    }

    pos.y += speed.y;
    _hitbox = hitbox();

    // Adjust for wall hit.
    float north_wall_dist = topy(_hitbox) - hit_map.north;
    if (north_wall_dist < 0) {
      pos.y -= north_wall_dist;
      speed.y = 0.f;
    }
    float south_wall_dist = static_cast<float>(hit_map.south) - bottomy(_hitbox);
    if (south_wall_dist < 0) {
      pos.y += south_wall_dist;
      speed.y = 0.f;
      multi_jump_count = 0;
    }

    // Override sprite when jumping / wall grabbing.
    if (is_grab_wall) {
      sprite_group.set_current_sprite(PLAYER_SPRITE_WALL_JUMP);
    } else if (jump_state == JumpState::Jump) {
      sprite_group.set_current_sprite(PLAYER_SPRITE_JUMP);
    } else if (jump_state == JumpState::Fall) {
      sprite_group.set_current_sprite(PLAYER_SPRITE_FALL);
    } else if (jump_state == JumpState::DoubleJump) {
      sprite_group.set_current_sprite(PLAYER_SPRITE_DOUBLE_JUMP);
    } else if (lifecycle_state == LifecycleState::Injured) {
      sprite_group.set_current_sprite(PLAYER_SPRITE_HIT);
    }
  }

  float speed_increments() const {
    return (PLAYER_MAX_REL_SPEED / (30.f * FPSMultiplier));
  }

  HitMap calculate_hitmap(Map const& map) const {
    HitMap hit_map{};
    Rectangle const _hitbox{hitbox()};

    hit_map.east = map.east_wall_of_range(_hitbox);
    hit_map.north = map.north_wall_of_range(_hitbox);
    hit_map.south = map.south_wall_of_range(_hitbox);
    hit_map.west = map.west_wall_of_range(_hitbox);

    return hit_map;
  }

  void end_injury() {
    lifecycle_state = LifecycleState::Live;
  }
};
