#pragma once

#include <algorithm>
#include <cmath>

#include "asset_manager.h"
#include "map.h"
#include "raylib.h"
#include "sprite_group.h"

constexpr int PLAYER_TEXTURE_SIZE{32};

constexpr float const PLAYER_MAX_REL_SPEED = 500.f;
constexpr float const PLAYER_MOVEMENT_FRICTION = 0.9f;
constexpr float const PLAYER_ZERO_SPEED_THRESHOLD = 0.1f;
constexpr float const PLAYER_JUMP_SPEED = -750.f;
constexpr float const PLAYER_GRAVITY = 0.96f;
constexpr float const PLAYER_GRAVITY_INV = 1.f / PLAYER_GRAVITY;
constexpr float const PLAYER_FALL_BACK_THRESHOLD = 100.f;
constexpr float const PLAYER_MAX_FALL_SPEED = PLAYER_MAX_REL_SPEED;
constexpr float const PLAYER_MULTI_JUMP_MAX = 2;
constexpr int const PLAYER_MAX_HEALTH{3};

constexpr int const PLAYER_SPRITE_RUN{0};
constexpr int const PLAYER_SPRITE_IDLE{1};
constexpr int const PLAYER_SPRITE_HIT{2};
constexpr int const PLAYER_SPRITE_JUMP{3};
constexpr int const PLAYER_SPRITE_FALL{4};
constexpr int const PLAYER_SPRITE_DOUBLE_JUMP{5};
constexpr int const PLAYER_SPRITE_WALL_JUMP{6};

constexpr Vector2 const AppearDisappearSpriteOffset{-32.f, -32.f};

enum class JumpState {
  Ground,
  Jump,
  Fall,
  DoubleJump,
};

enum class LifecycleState {
  Appear,
  Disappear,
  Live,
  Injured,
  WaitingForNextLevel,
};

struct Character {
 public:
  Character(int const pixel_size) : pixel_size(pixel_size), appear_sprite(pixel_size), disappear_sprite(pixel_size) {
  }

  void reset(Vector2 new_pos) {
    spawn_location = new_pos;
    pos = new_pos;
    sprite_group.reset();
    jump_state = JumpState::Ground;
    lifecycle_state = LifecycleState::Appear;
    multi_jump_count = PLAYER_MULTI_JUMP_MAX - 1;
    level_complete = false;
    health = PLAYER_MAX_HEALTH;
  }

  void init() {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    set_sprite_group_collection(0);

    appear_sprite.init_texture(asset_manager.textures[TextureNames::Character__Appear], {96.f, 96.f}, 7,
                               sprite_frame_length);

    disappear_sprite.init_texture(asset_manager.textures[TextureNames::Character__Disappear], {96.f, 96.f}, 7,
                                  sprite_frame_length);
  }

  void update(Map const& map) {
    if (lifecycle_state == LifecycleState::Appear) {
      if (appear_sprite.update() == 0) lifecycle_state = LifecycleState::Live;
    } else if (lifecycle_state == LifecycleState::Disappear) {
      if (disappear_sprite.update() == 0) {
        if (level_complete) {
          lifecycle_state = LifecycleState::WaitingForNextLevel;
        } else {
          reset(spawn_location);
        }
      }
    } else if (lifecycle_state == LifecycleState::Live || lifecycle_state == LifecycleState::Injured) {
      update_movement(map);
      update_drop_down(map);
      update_sprite();
      if (map.check_collision_with_fully_solid_walls(shrink(hitbox(), 1.f))) injure(true);
      sprite_group.update();
    } else if (lifecycle_state == LifecycleState::WaitingForNextLevel) {
      // Noop.
    } else {
      BAIL;
    }

    injury_timeout.update();
  }

  void draw() const {
    if (lifecycle_state == LifecycleState::Appear) {
      appear_sprite.draw(Vector2Add(pos, Vector2Scale(AppearDisappearSpriteOffset, pixel_size)));
    } else if (lifecycle_state == LifecycleState::Disappear) {
      disappear_sprite.draw(Vector2Add(pos, Vector2Scale(AppearDisappearSpriteOffset, pixel_size)));
    } else {
      sprite_group.draw(pos);
    }

    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
    // DrawLineEx({0.f, static_cast<float>(hit_map.south.wall)},
    //            {static_cast<float>(GetScreenWidth()), static_cast<float>(hit_map.south.wall)}, pixel_size, MAGENTA);
  }

  bool is_falling() const {
    return jump_state == JumpState::Fall;
  }

  Rectangle const hitbox() const {
    return move(upscale(CHARACTER_HITBOX, pixel_size), pos);
  }

  void injure(bool const is_fatal = false) {
    if (is_injured()) return;

    health = std::max(0, health - (is_fatal ? 3 : 1));

    if (health <= 0) {
      lifecycle_state = LifecycleState::Disappear;
      injury_timeout.cancel();
      health = 0;
      return;
    }

    injury_timeout.cancel();
    lifecycle_state = LifecycleState::Injured;
    sprite_group.set_current_sprite(PLAYER_SPRITE_HIT);

    injury_timeout.set_on_timeout([&]() { end_injury(); }, 2.f);
  }

  void enemy_head_bounce() {
    speed.y = PLAYER_JUMP_SPEED / 1.5f;
    jump_state = JumpState::Jump;
  }

  void set_position(IntVec2 const& v) {
    pos.x = static_cast<float>(v.x);
    pos.y = static_cast<float>(v.y);
  }

  bool is_injured() const {
    return lifecycle_state == LifecycleState::Injured;
  }

  void bouncing_trap_interact() {
    multi_jump_count = 1;
    speed.y = PLAYER_JUMP_SPEED * 2.5f;
    jump_state = JumpState::Jump;
  }

  void set_level_complete() {
    if (level_complete) return;

    level_complete = true;
    lifecycle_state = LifecycleState::Disappear;
  }

  bool const ready_for_next_level() const {
    return lifecycle_state == LifecycleState::WaitingForNextLevel;
  }

 private:
  const int pixel_size{DEFAULT_PIXEL_SIZE};
  SpriteGroup sprite_group{};
  Sprite appear_sprite;
  Sprite disappear_sprite;
  Vector2 pos{};
  Vector2 speed{};
  int multi_jump_count{0};
  JumpState jump_state{JumpState::Ground};
  LifecycleState lifecycle_state{LifecycleState::Appear};
  Timeout injury_timeout{};
  Vector2 spawn_location{};
  bool level_complete{false};
  int health{PLAYER_MAX_HEALTH};

  // Debug.
  HitAndDragMap hit_map;

  bool is_live() const {
    return lifecycle_state == LifecycleState::Live;
  }

  void update_movement(Map const& map) {
    // More of a hack but this helps vertical stabilization when above is a vertically moving platform.
    pos.y += map.south_wall_of_range(hitbox()).wall_vertical_speed;

    Rectangle _hitbox{hitbox()};

    // Adjusting for vertical above platforms. This helps the character not to be pushed
    // left when a platform is getting closer from above.
    float north_wall_pre_adjust = topy(_hitbox) - map.north_wall_of_range(_hitbox);
    if (north_wall_pre_adjust < 0.f) {
      pos.y -= north_wall_pre_adjust;
    }

    if (is_live() && IsKeyDown(KEY_LEFT)) {
      sprite_group.horizontal_flip();
      sprite_group.set_current_sprite(PLAYER_SPRITE_RUN);
      speed.x -= speed_increments();

      if (speed.x < -PLAYER_MAX_REL_SPEED) speed.x = -PLAYER_MAX_REL_SPEED;
    } else if (is_live() && IsKeyDown(KEY_RIGHT)) {
      sprite_group.horizontal_reset();
      sprite_group.set_current_sprite(PLAYER_SPRITE_RUN);
      speed.x += speed_increments();

      if (speed.x > PLAYER_MAX_REL_SPEED) speed.x = PLAYER_MAX_REL_SPEED;
    } else {
      sprite_group.set_current_sprite(PLAYER_SPRITE_IDLE);
      fps_independent_multiply(&speed.x, PLAYER_MOVEMENT_FRICTION);

      if (fabs(speed.x) < PLAYER_ZERO_SPEED_THRESHOLD) speed.x = 0.f;
    }

    hit_map = calculate_hitmap(map);

    pos.x += speed.x * world_rate();

    _hitbox = hitbox();
    static WallGrabJumpCounter wall_grab_counter{};
    bool is_grab_wall{false};

    // Adjust for wall hit.
    float west_wall_dist = leftx(_hitbox) - hit_map.west;
    if (west_wall_dist < 0.f) {
      pos.x -= west_wall_dist;
      speed.x = 0.f;
      is_grab_wall = hit_map.west > 0;  // Only grab real walls.
      wall_grab_counter.at_west();
    }
    float east_wall_dist = hit_map.east - rightx(_hitbox);
    if (east_wall_dist < 0.f) {
      pos.x += east_wall_dist;
      speed.x = 0.f;
      is_grab_wall = hit_map.east < GetScreenWidth() - 1;  // Only grab real walls.
      wall_grab_counter.at_east();
    }

    if (is_live() && IsKeyPressed(KEY_SPACE) &&
        (multi_jump_count < PLAYER_MULTI_JUMP_MAX || wall_grab_counter.can_jump())) {
      wall_grab_counter.count_jump();
      speed.y = PLAYER_JUMP_SPEED;
      multi_jump_count++;

      PlaySound(*asset_manager.sounds[SoundName::JumpSound]);

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

      if (speed.y > -PLAYER_FALL_BACK_THRESHOLD) {
        speed.y = PLAYER_FALL_BACK_THRESHOLD;  // Start falling.
        jump_state = JumpState::Fall;
      }
    } else if (speed.y > 0.f) {
      // Falling.
      fps_independent_multiply(&speed.y, PLAYER_GRAVITY_INV);

      if (speed.y > PLAYER_MAX_FALL_SPEED) speed.y = PLAYER_MAX_FALL_SPEED;
      if (is_grab_wall && speed.y > PLAYER_MAX_FALL_SPEED / 5.f) {
        speed.y = PLAYER_MAX_FALL_SPEED / 5.f;
      }
      jump_state = JumpState::Fall;
    } else {
      jump_state = JumpState::Ground;
      speed.y = PLAYER_FALL_BACK_THRESHOLD;
    }

    pos.y += speed.y * world_rate();

    // Adjust for wall hit.
    float north_wall_dist = topy(hitbox()) - hit_map.north;
    if (north_wall_dist < 0.f) {
      pos.y -= north_wall_dist;
      speed.y = 0.f;
    }
    float south_wall_dist = static_cast<float>(hit_map.south.wall) - bottomy(hitbox());
    if (south_wall_dist < 0.f) {
      pos.y += south_wall_dist;
      speed.y = 0.f;
      multi_jump_count = 0;

      // Touching south wall -> apply south wall drag.
      pos.x += hit_map.south.wall_horizontal_speed;
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

  void update_drop_down(Map const& map) {
    if (IsKeyPressed(KEY_DOWN)) {
      if (jump_state == JumpState::Ground) {
        Rectangle const _hitbox{hitbox()};
        Rectangle const drop_down_hitbox = movey(_hitbox, _hitbox.height);
        if (!map.check_collision_with_fully_solid_walls(drop_down_hitbox)) {
          pos.y += pixel_size * 4;
        }
      }
    }
  }

  float speed_increments() const {
    return (PLAYER_MAX_REL_SPEED / (30.f / FPS_MULTIPLIER));
  }

  HitAndDragMap calculate_hitmap(Map const& map) const {
    HitAndDragMap hit_map{};
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

  void update_sprite() {
    if (IsKeyPressed(KEY_ONE)) set_sprite_group_collection(0);
    if (IsKeyPressed(KEY_TWO)) set_sprite_group_collection(1);
    if (IsKeyPressed(KEY_THREE)) set_sprite_group_collection(2);
  }

  void set_sprite_group_collection(int const index) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    sprite_group.clear();
    size_t old_sprite_index = sprite_group.get_current_sprite_index();
    int index_offset = index * 7;

    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Run + index_offset],
                                    {32.f, 32.f},
                                    12,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Idle + index_offset],
                                    {32.f, 32.f},
                                    11,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Hit + index_offset],
                                    {32.f, 32.f},
                                    7,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Jump + index_offset],
                                    {32.f, 32.f},
                                    1,
                                    0});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Fall + index_offset],
                                    {32.f, 32.f},
                                    1,
                                    0});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Double_Jump + index_offset],
                                    {32.f, 32.f},
                                    6,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Character1__Wall_Jump + index_offset],
                                    {32.f, 32.f},
                                    5,
                                    sprite_frame_length});

    sprite_group.set_current_sprite(old_sprite_index);
  }
};
