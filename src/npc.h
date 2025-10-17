#pragma once

#include "asset_manager.h"
#include "common.h"
#include "map.h"
#include "raylib.h"
#include "raymath.h"
#include "sprite.h"
#include "sprite_group.h"

constexpr Vector2 const SIMPLE_WALK_NPC_SIZE{48.f, 48.f};
constexpr Vector2 const ChargingNpcSize{48.f, 48.f};

constexpr size_t const SimpleWalkNpcSpriteFall{0};
constexpr size_t const SimpleWalkNpcSpriteHit{1};
constexpr size_t const SimpleWalkNpcSpriteIdle{2};
constexpr size_t const SimpleWalkNpcSpriteJump{3};
constexpr size_t const SimpleWalkNpcSpriteRun{4};

constexpr size_t const ChargingNpcSpriteCharge{0};
constexpr size_t const ChargingNpcSpriteHit{1};
constexpr size_t const ChargingNpcSpriteIdle{2};
constexpr size_t const ChargingNpcSpriteJump{3};
constexpr size_t const ChargingNpcSpriteWalk{4};

constexpr float const SimpleWalkNpcSpeed{200.f};
constexpr float const ChargingNpcWalkSpeed{150.f};
constexpr float const ChargingNpcChargeSpeed{250.f};

enum class SimpleWalkNpcState { Idle, Run, Hit };

struct Npc {
 public:
  virtual void draw() const = 0;
  virtual void update(Map const& map) = 0;
  virtual Rectangle hitbox() const = 0;
  virtual void injure() = 0;
  virtual bool is_injured() const = 0;

  virtual ~Npc() = default;
};

struct SimpleWalkNpc : Npc {
 public:
  SimpleWalkNpc(IntVec2 const pos, TileSource const tile_source, int const pixel_size)
      : pos(pos.scale(pixel_size).to_vector2()), pixel_size(pixel_size), tile_source(tile_source) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GetMonitorRefreshRate(0) / 24);

    switch (tile_source) {
      case TileSource::Enemy1:
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Fall], SIMPLE_WALK_NPC_SIZE, 3,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Hit], SIMPLE_WALK_NPC_SIZE, 5,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Idle], SIMPLE_WALK_NPC_SIZE, 11,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Jump], SIMPLE_WALK_NPC_SIZE, 3,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy1__Run], SIMPLE_WALK_NPC_SIZE, 12,
                                        sprite_frame_length});

        break;
      case TileSource::Enemy2:
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Fall], SIMPLE_WALK_NPC_SIZE, 1,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Hit], SIMPLE_WALK_NPC_SIZE, 5,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Idle], SIMPLE_WALK_NPC_SIZE, 11,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Jump], SIMPLE_WALK_NPC_SIZE, 1,
                                        sprite_frame_length});
        sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                        asset_manager.textures[TextureNames::Enemy2__Run], SIMPLE_WALK_NPC_SIZE, 12,
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
  }

  void update(Map const& map) override {
    movement_timeout.update();
    sprite_group.update();

    if (state == SimpleWalkNpcState::Run) {
      Rectangle _hitbox = hitbox();
      int west_wall = map.west_wall_of_range(_hitbox.x, _hitbox.y, _hitbox.y + _hitbox.height - 1);
      int east_wall = map.east_wall_of_range(_hitbox.x + _hitbox.width - 1, _hitbox.y, _hitbox.y + _hitbox.height - 1);

      pos.x += speed.x * GetFrameTime();
      _hitbox = hitbox();

      // Handle walls.
      if (speed.x > 0) {  // Walk right.
        float wall_overlap = east_wall - (_hitbox.x + _hitbox.width - 1.f);
        if (wall_overlap < 0.f) {
          pos.x += wall_overlap;
          sprite_group.horizontal_reset();
          speed.x = -SimpleWalkNpcSpeed;
        }
      } else if (speed.x < 0) {  // Walk left.
        float wall_overlap = _hitbox.x - west_wall;
        if (wall_overlap < 0.f) {
          pos.x -= wall_overlap;
          sprite_group.horizontal_flip();
          speed.x = SimpleWalkNpcSpeed;
        }
      }

      if (movement_timer.update()) {
        if (randf() >= 0.8) {
          state = SimpleWalkNpcState::Idle;
          sprite_group.set_current_sprite(SimpleWalkNpcSpriteIdle);
          movement_timeout.set_on_timeout([&]() { this->resume_to_run_state(); }, randf() * 3.f);
        }
      }
    } else if (state == SimpleWalkNpcState::Idle) {
      if (movement_timer.update()) {
        if (randf() > 0.6) turn_horizontally();
      }
    }
  }

  Rectangle hitbox() const override {
    return move(upscale(tile_source_hitbox(tile_source, intvec2_0_0), pixel_size), pos);
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
  Vector2 pos;
  Vector2 speed{-SimpleWalkNpcSpeed, 0.f};
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

struct ChargingNpc : Npc {
 public:
  ChargingNpc(Vector2 const pos, int const pixel_size) : pos(pos), pixel_size(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GetMonitorRefreshRate(0) / 24);

    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size),
                                    asset_manager.textures[TextureNames::Enemy3__Charge], ChargingNpcSize, 12,
                                    sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Hit],
                                    ChargingNpcSize, 5, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Idle],
                                    ChargingNpcSize, 11, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Stun],
                                    ChargingNpcSize, 8, sprite_frame_length});
    sprite_group.push_sprite(Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::Enemy3__Walk],
                                    ChargingNpcSize, 12, sprite_frame_length});

    sprite_group.set_current_sprite(ChargingNpcSpriteIdle);
  }

  void draw() const override {
    sprite_group.draw(pos);
  }

  void update(Map const& map) override {
    sprite_group.update();
  }

  Rectangle hitbox() const override {
  }

  void injure() override {
  }

  bool is_injured() const override {
  }

  virtual ~ChargingNpc() override {
  }

 private:
  Vector2 pos;
  int const pixel_size;
  SpriteGroup sprite_group{};
};
