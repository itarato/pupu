#pragma once

#include <algorithm>
#include <list>

#include "asset_manager.h"
#include "bullet.h"
#include "character.h"
#include "common.h"
#include "map.h"
#include "raylib.h"
#include "raymath.h"
#include "sprite.h"
#include "sprite_group.h"

constexpr float const NPC_GRAVITY{0.96f};
constexpr float const NPC_GRAVITY_INV{1.f / NPC_GRAVITY};
constexpr float const NPC_FALLBACK_THRESHOLD{100.f};
constexpr float const NPC_MAX_FALL_SPEED{400.f};

constexpr size_t const SimpleWalkNpcSpriteFall{0};
constexpr size_t const SimpleWalkNpcSpriteHit{1};
constexpr size_t const SimpleWalkNpcSpriteIdle{2};
constexpr size_t const SimpleWalkNpcSpriteJump{3};
constexpr size_t const SimpleWalkNpcSpriteRun{4};

constexpr size_t const ChargingNpcSpriteCharge{0};
constexpr size_t const ChargingNpcSpriteHit{1};
constexpr size_t const ChargingNpcSpriteIdle{2};
constexpr size_t const ChargingNpcSpriteStun{3};
constexpr size_t const ChargingNpcSpriteWalk{4};

constexpr size_t const ShootingNpcSpriteAttack{0};
constexpr size_t const ShootingNpcSpriteHit{1};
constexpr size_t const ShootingNpcSpriteIdle{2};
constexpr size_t const ShootingNpcSpriteWalk{3};

constexpr size_t const StompingNpcSpriteAttack{0};
constexpr size_t const StompingNpcSpriteFly{1};
constexpr size_t const StompingNpcSpriteHit{2};
constexpr size_t const StompingNpcSpriteIdle{3};

constexpr float const SimpleWalkNpcSpeed{200.f};
constexpr float const ChargingNpcWalkSpeed{100.f};
constexpr float const ChargingNpcChargeSpeed{300.f};
constexpr float const ShootingNpcSpeed{200.f};

constexpr uint8_t const HANDLE_FALL_MASK_FALLING{0b001};
constexpr uint8_t const HANDLE_FALL_MASK_JUST_LANDED{0b010};

enum class SimpleWalkNpcState { Idle, Run, Hit };

struct MobileGameObject {
 public:
  MobileGameObject() = default;
  MobileGameObject(Vector2 const pos) : pos(pos) {
  }
  ~MobileGameObject() = default;
  virtual Rectangle const hitbox() const = 0;

 protected:
  Vector2 pos{};
  Vector2 speed{};

  /**
   * @return int wall collision mask
   */
  int handle_basic_movement(Map const& map) {
    int collision_mask{0};

    Rectangle _hitbox = hitbox();
    int west_wall = map.west_wall_of_range(_hitbox);
    int east_wall = map.east_wall_of_range(_hitbox);

    pos.x += speed.x * GetFrameTime();
    _hitbox = hitbox();

    if (speed.x > 0.f) {  // Walk right.
      float wall_overlap = east_wall - rightx(_hitbox);
      if (wall_overlap < 0.f) {
        pos.x += wall_overlap;
        collision_mask |= COLLISION_TYPE_EAST;
      }
    } else if (speed.x < 0.f) {  // Walk left.
      float wall_overlap = leftx(_hitbox) - west_wall;
      if (wall_overlap < 0.f) {
        pos.x -= wall_overlap;
        collision_mask |= COLLISION_TYPE_WEST;
      }
    }

    return collision_mask;
  }

  /**
   * @return int HANDLE_FALL_MASK_?.
   */
  uint8_t handle_fall(Map const& map) {
    uint8_t fall_mask{0};

    // TODO: see if we can allow NPC to use moving platforms.
    CollisionResult south_wall = map.south_wall_of_range(hitbox());

    bool was_falling{false};
    if (speed.y < 0.f) {
      BAILF("NPC jumps are not supported yet");
    } else if (speed.y > 0.f) {
      // Falling.
      fps_independent_multiply(&speed.y, NPC_GRAVITY_INV);
      if (speed.y > NPC_MAX_FALL_SPEED) speed.y = NPC_MAX_FALL_SPEED;
      was_falling = true;
    } else {
      speed.y = NPC_FALLBACK_THRESHOLD;
    }

    pos.y += speed.y * GetFrameTime();

    float south_wall_dist = south_wall.wall - bottomy(hitbox());
    if (south_wall_dist < 0.f) {
      pos.y += south_wall_dist;
      speed.y = 0.f;

      if (was_falling) fall_mask |= HANDLE_FALL_MASK_JUST_LANDED;
    } else {
      fall_mask |= HANDLE_FALL_MASK_FALLING;
    }

    return fall_mask;
  }
};

struct Npc {
 public:
  virtual void draw() const = 0;
  virtual void update(Map& map, Character& character) = 0;
  virtual Rectangle const hitbox() const = 0;
  virtual void injure() = 0;
  virtual bool is_injured() const = 0;

  virtual ~Npc() = default;
};

struct SimpleWalkNpc : Npc, MobileGameObject {
 public:
  SimpleWalkNpc(IntVec2 const coord, TileSource const tile_source, int const pixel_size)
      : pixel_size(pixel_size), tile_source(tile_source) {
    speed = {-SimpleWalkNpcSpeed, 0.f};
    pos = coord.scale(pixel_size).to_vector2();

    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    switch (tile_source) {
      case TileSource::Enemy1:
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Fall], LARGE_48x48_TILE_SIZE, 3,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Hit], LARGE_48x48_TILE_SIZE, 5,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Idle], LARGE_48x48_TILE_SIZE, 11,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Jump], LARGE_48x48_TILE_SIZE, 3,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Run], LARGE_48x48_TILE_SIZE, 12,
                                        sprite_frame_length});

        break;
      case TileSource::Enemy2:
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Fall], LARGE_48x48_TILE_SIZE, 1,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Hit], LARGE_48x48_TILE_SIZE, 5,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Idle], LARGE_48x48_TILE_SIZE, 11,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Jump], LARGE_48x48_TILE_SIZE, 1,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Run], LARGE_48x48_TILE_SIZE, 12,
                                        sprite_frame_length});

        break;
      default:
        BAILF("Invalid: %d", tile_source);
    }

    sprite_group.set_current_sprite(SimpleWalkNpcSpriteRun);
  }

  ~SimpleWalkNpc() = default;

  void draw() const override {
    sprite_group.draw(pos);
    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
  }

  void update(Map& map, Character& character) override {
    movement_timeout.update();
    sprite_group.update();

    if (state == SimpleWalkNpcState::Run) {
      // Move and handle collisions.
      int fall_mask = handle_fall(map);
      if (fall_mask & HANDLE_FALL_MASK_FALLING) sprite_group.set_current_sprite(SimpleWalkNpcSpriteFall);
      if (fall_mask & HANDLE_FALL_MASK_JUST_LANDED) sprite_group.set_current_sprite(SimpleWalkNpcSpriteRun);
      if (!(fall_mask & HANDLE_FALL_MASK_FALLING)) {
        const int collision_mask = handle_basic_movement(map);
        if (collision_mask & COLLISION_TYPE_EAST) {
          sprite_group.horizontal_reset();
          speed.x = -SimpleWalkNpcSpeed;
        }
        if (collision_mask & COLLISION_TYPE_WEST) {
          sprite_group.horizontal_flip();
          speed.x = SimpleWalkNpcSpeed;
        }

        if (movement_timer.update()) {
          if (randf() >= 0.8) {
            state = SimpleWalkNpcState::Idle;
            sprite_group.set_current_sprite(SimpleWalkNpcSpriteIdle);
            movement_timeout.set_on_timeout([&]() { this->resume_to_run_state(); }, randf() * 3.f);
          }
        }
      }
    } else if (state == SimpleWalkNpcState::Idle) {
      if (movement_timer.update()) {
        if (randf() > 0.6) turn_horizontally();
      }
    }
  }

  Rectangle const hitbox() const override {
    return move(upscale(tile_source_hitbox(tile_source), pixel_size), pos);
  }

  void injure() override {
    sprite_group.set_current_sprite(SimpleWalkNpcSpriteHit);
    state = SimpleWalkNpcState::Hit;

    movement_timeout.cancel();
    movement_timeout.set_on_timeout([&]() { resume_to_run_state(); }, 3.f);
  }

  bool is_injured() const override {
    return state == SimpleWalkNpcState::Hit;
  }

 private:
  int const pixel_size;
  SpriteGroup sprite_group{};
  SimpleWalkNpcState state{SimpleWalkNpcState::Run};
  Timeout movement_timeout{};
  RepeatTimer movement_timer{0.3};
  TileSource const tile_source;

  void resume_to_run_state() {
    state = SimpleWalkNpcState::Run;
    sprite_group.set_current_sprite(SimpleWalkNpcSpriteRun);
  }

  void turn_horizontally() {
    if (speed.x < 0) {
      sprite_group.horizontal_flip();
      speed.x = SimpleWalkNpcSpeed;
    } else {
      sprite_group.horizontal_reset();
      speed.x = -SimpleWalkNpcSpeed;
    }
  }
};

enum class ChargingNpcState {
  Walking,
  Charging,
  Stunned,
  Hit,
};

struct ChargingNpc : Npc, MobileGameObject {
 public:
  ChargingNpc(Vector2 const pos, int const pixel_size) : MobileGameObject(pos), pixel_size(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Enemy3__Charge], LARGE_48x48_TILE_SIZE, 12,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Hit],
                                    LARGE_48x48_TILE_SIZE, 5, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Idle],
                                    LARGE_48x48_TILE_SIZE, 11, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Stun],
                                    LARGE_48x48_TILE_SIZE, 8, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Walk],
                                    LARGE_48x48_TILE_SIZE, 12, sprite_frame_length});

    sprite_group.set_current_sprite(ChargingNpcSpriteWalk);
  }

  ~ChargingNpc() = default;

  void draw() const override {
    sprite_group.draw(pos);
  }

  void update(Map& map, Character& character) override {
    sprite_group.update();
    charge_stunned_timeout.update();
    hit_timeout.update();

    bool did_fall = handle_fall(map);
    if (!did_fall) {
      Rectangle _hitbox = hitbox();
      int west_wall = map.west_wall_of_range(_hitbox);
      int east_wall = map.east_wall_of_range(_hitbox);

      if (is_walking()) {
        Rectangle character_hitbox{character.hitbox()};
        if (can_charge_character_horizontal(west_wall, east_wall, _hitbox, character_hitbox)) {
          state = ChargingNpcState::Charging;
          sprite_group.set_current_sprite(ChargingNpcSpriteCharge);
          if (character_hitbox.x <= _hitbox.x) {  // Charge left.
            sprite_group.horizontal_reset();
            is_direction_left = true;
          } else {  // Charge right.
            sprite_group.horizontal_flip();
            is_direction_left = false;
          }
        }
      }

      speed.x = calculate_speed() * (is_direction_left ? -1.f : 1.f);
      int collision_mask = handle_basic_movement(map);
      if (collision_mask & COLLISION_TYPE_WEST) {
        sprite_group.horizontal_flip();
        is_direction_left = false;
      } else if (collision_mask & COLLISION_TYPE_EAST) {
        sprite_group.horizontal_reset();
        is_direction_left = true;
      }

      if (collision_mask & (COLLISION_TYPE_EAST | COLLISION_TYPE_WEST)) {
        if (is_charging()) {
          if (collision_mask & COLLISION_TYPE_WEST) {
            map.try_demolition(movex(hitbox(), -pixel_size));
          }
          if (collision_mask & COLLISION_TYPE_EAST) {
            map.try_demolition(movex(hitbox(), pixel_size));
          }

          state = ChargingNpcState::Stunned;
          sprite_group.set_current_sprite(ChargingNpcSpriteStun);
          charge_stunned_timeout.cancel();
          charge_stunned_timeout.set_on_timeout(
              [&]() {
                state = ChargingNpcState::Walking;
                sprite_group.set_current_sprite(ChargingNpcSpriteWalk);
              },
              2.f);
        }
      }
    }
  }

  Rectangle const hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Enemy3), pixel_size), pos);
  }

  void injure() override {
    state = ChargingNpcState::Hit;
    sprite_group.set_current_sprite(ChargingNpcSpriteHit);
    hit_timeout.cancel();
    charge_stunned_timeout.cancel();
    hit_timeout.set_on_timeout(
        [&]() {
          state = ChargingNpcState::Walking;
          sprite_group.set_current_sprite(ChargingNpcSpriteWalk);
        },
        3.f);
  }

  bool is_injured() const override {
    return state == ChargingNpcState::Stunned || state == ChargingNpcState::Hit;
  }

 private:
  int const pixel_size;
  SpriteGroup sprite_group{};
  bool is_direction_left{true};
  Timeout charge_stunned_timeout{};
  ChargingNpcState state{ChargingNpcState::Walking};
  Timeout hit_timeout{};

  bool is_charging() const {
    return state == ChargingNpcState::Charging;
  }

  bool is_walking() const {
    return state == ChargingNpcState::Walking;
  }

  float calculate_speed() const {
    switch (state) {
      case ChargingNpcState::Charging:
        return ChargingNpcChargeSpeed;
      case ChargingNpcState::Walking:
        return ChargingNpcWalkSpeed;
      case ChargingNpcState::Stunned:
      case ChargingNpcState::Hit:
        return 0.f;
      default:
        BAIL;
    }
  }
};

enum class ShootingNpcState {
  Walk,
  Idle,
  Attack,
  Hit,
};

struct ShootingNpc : Npc, MobileGameObject {
 public:
  ShootingNpc(Vector2 const pos, int const pixel_size) : MobileGameObject(pos), pixel_size(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Enemy4__Attack], LARGE_48x48_TILE_SIZE, 7,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy4__Hit],
                                    LARGE_48x48_TILE_SIZE, 5, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy4__Idle],
                                    LARGE_48x48_TILE_SIZE, 11, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy4__Walk],
                                    LARGE_48x48_TILE_SIZE, 12, sprite_frame_length});
  }

  void draw() const override {
    sprite_group.draw(pos);
    for (auto const& bullet : bullets) bullet.draw();
  }

  void update(Map& map, Character& character) override {
    int sprite_group_sequence = sprite_group.update();
    hit_timeout.update();
    shoot_delay_timer.update();

    for (auto& bullet : bullets) {
      bullet.update();
      if (CheckCollisionRecs(character.hitbox(), bullet.hitbox())) {
        character.injure();
        bullet.set_target_hit();
      }
    }
    std::erase_if(bullets, [](auto const& bullet) { return bullet.is_dead(); });

    speed.x = (state == ShootingNpcState::Walk ? ShootingNpcSpeed : 0.f) * (is_direction_left ? -1.f : 1.f);

    int fall_mask = handle_fall(map);
    if (!(fall_mask & HANDLE_FALL_MASK_FALLING)) {
      int collision_mask = handle_basic_movement(map);
      if (collision_mask & COLLISION_TYPE_WEST) {
        sprite_group.horizontal_flip();
        is_direction_left = false;
      } else if (collision_mask & COLLISION_TYPE_EAST) {
        sprite_group.horizontal_reset();
        is_direction_left = true;
      }
    }

    Rectangle _hitbox = hitbox();
    int west_wall = map.west_wall_of_range(_hitbox);
    int east_wall = map.east_wall_of_range(_hitbox);
    Rectangle character_hitbox{character.hitbox()};

    if (state == ShootingNpcState::Walk) {
      if (!character.is_injured() && can_charge_character_horizontal(west_wall, east_wall, _hitbox, character_hitbox) &&
          shoot_delay_timer.is_passed()) {
        state = ShootingNpcState::Attack;
        sprite_group.set_current_sprite(ShootingNpcSpriteAttack);
        if (character_hitbox.x <= _hitbox.x) {  // Charge left.
          sprite_group.horizontal_reset();
          is_direction_left = true;
        } else {  // Charge right.
          sprite_group.horizontal_flip();
          is_direction_left = false;
        }
      }
    }
    if (state == ShootingNpcState::Attack) {
      if (sprite_group_sequence == 4) {
        bullets.emplace_back(bullet_spawn_point(), pixel_size, is_direction_left ? -400.f : 400.f, west_wall,
                             east_wall);
        shoot_delay_timer.reset(0.8);
      }
      if (sprite_group_sequence == 0) {
        if (!can_charge_character_horizontal(west_wall, east_wall, _hitbox, character_hitbox) ||
            character.is_injured() || !shoot_delay_timer.is_passed()) {
          state = ShootingNpcState::Walk;
          sprite_group.set_current_sprite(ShootingNpcSpriteWalk);
        }
      }
    }
  }

  Rectangle const hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Enemy4), pixel_size), pos);
  }

  void injure() override {
    hit_timeout.cancel();
    state = ShootingNpcState::Hit;
    sprite_group.set_current_sprite(ShootingNpcSpriteHit);
    hit_timeout.set_on_timeout(
        [&]() {
          state = ShootingNpcState::Walk;
          sprite_group.set_current_sprite(ShootingNpcSpriteWalk);
        },
        3.f);
  }

  bool is_injured() const override {
    return state == ShootingNpcState::Hit;
  }

  ~ShootingNpc() = default;

 private:
  int const pixel_size;
  SpriteGroup sprite_group{};
  bool is_direction_left{true};
  Timeout hit_timeout{};
  ShootingNpcState state{ShootingNpcState::Walk};
  std::list<Bullet> bullets{};
  Timer shoot_delay_timer{};

  Vector2 bullet_spawn_point() const {
    if (is_direction_left) {
      return Vector2{pos.x + 6.f * pixel_size, pos.y + 24.f * pixel_size};
    } else {
      return Vector2{pos.x + 35.f * pixel_size, pos.y + 24.f * pixel_size};
    }
  }
};

enum class StompingNpcState {
  Attack,
  Fly,
  Hit,
  Idle,
};

struct StompingNpc : Npc {
 public:
  StompingNpc(Vector2 const pos, int const pixel_size) : pos(pos), pixel_size(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Enemy5__Attack], LARGE_48x48_TILE_SIZE, 8,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy5__Fly],
                                    LARGE_48x48_TILE_SIZE, 6, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy5__Hit],
                                    LARGE_48x48_TILE_SIZE, 5, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy5__Idle],
                                    LARGE_48x48_TILE_SIZE, 6, sprite_frame_length});

    sprite_group.set_current_sprite(StompingNpcSpriteFly);
  }

  void draw() const override {
    sprite_group.draw(pos);
  }

  void update(Map& map, Character& character) override {
    sprite_group.update();
    hit_timeout.update();

    Rectangle _hitbox = hitbox();
    int north_wall = map.north_wall_of_range(_hitbox);
    CollisionResult south_collision_result = map.south_wall_of_range(_hitbox);
    int south_wall = south_collision_result.wall;

    pos.y += speed() * GetFrameTime();
    _hitbox = hitbox();
    Rectangle character_hitbox{character.hitbox()};

    if (state == StompingNpcState::Idle) {
      if (can_charge_character_vertical(south_wall, _hitbox, character_hitbox)) {
        state = StompingNpcState::Attack;
        sprite_group.set_current_sprite(StompingNpcSpriteAttack);
      }
    }

    // Handle walls.
    if (state == StompingNpcState::Fly) {  // Upwards.
      float wall_overlap = _hitbox.y - north_wall;
      if (wall_overlap < 0.f) {
        pos.y -= wall_overlap;
        state = StompingNpcState::Idle;
        sprite_group.set_current_sprite(StompingNpcSpriteIdle);
      }
    } else if (state == StompingNpcState::Attack) {  // Downwards.
      float wall_overlap = south_wall - (_hitbox.y + _hitbox.height - 1.f);
      if (wall_overlap < 0.f) {
        pos.y += wall_overlap;
        state = StompingNpcState::Fly;
        sprite_group.set_current_sprite(StompingNpcSpriteFly);
      }
    }
  }

  Rectangle const hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Enemy5), pixel_size), pos);
  }

  void injure() override {
    hit_timeout.cancel();
    state = StompingNpcState::Hit;
    sprite_group.set_current_sprite(StompingNpcSpriteHit);
    hit_timeout.set_on_timeout(
        [&]() {
          sprite_group.set_current_sprite(StompingNpcSpriteFly);
          state = StompingNpcState::Fly;
        },
        3.f);
  }

  bool is_injured() const override {
    return state == StompingNpcState::Hit;
  }

  ~StompingNpc() = default;

 private:
  Vector2 pos;
  int const pixel_size;
  SpriteGroup sprite_group{};
  Timeout hit_timeout{};
  StompingNpcState state{StompingNpcState::Fly};

  float speed() const {
    switch (state) {
      case StompingNpcState::Attack:
        return 300.f;
      case StompingNpcState::Fly:
        return -50.f;
      case StompingNpcState::Hit:
      case StompingNpcState::Idle:
        return 0.f;
      default:
        BAIL;
    }
  }
};
