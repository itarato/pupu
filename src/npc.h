#pragma once

#include "asset_manager.h"
#include "common.h"
#include "map.h"
#include "raylib.h"
#include "raymath.h"
#include "sprite.h"
#include "sprite_group.h"

constexpr Vector2 const SIMPLE_WALK_NPC_SIZE{48.f, 48.f};

constexpr size_t const SimpleWalkNpcSpriteFall{0};
constexpr size_t const SimpleWalkNpcSpriteHit{1};
constexpr size_t const SimpleWalkNpcSpriteIdle{2};
constexpr size_t const SimpleWalkNpcSpriteJump{3};
constexpr size_t const SimpleWalkNpcSpriteRun{4};

constexpr float const SimpleWalkNpcSpeed{200.f};

enum class SimpleWalkNpcState { Idle, Run, Hit };

struct Npc {
 public:
  virtual void draw() const = 0;
  virtual void update(Map const& map) = 0;

  virtual ~Npc() = default;
};

struct SimpleWalkNpc : Npc {
 public:
  SimpleWalkNpc(IntVec2 const pos, TileSource const tile_source, int const pixel_size)
      : pos(pos.to_vector2()), pixel_size(pixel_size) {
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

        sprite_group.set_current_sprite(SimpleWalkNpcSpriteRun);
        break;
      default:
        BAILF("Invalid: %d", tile_source);
    }
  }

  ~SimpleWalkNpc() = default;

  void draw() const override {
    sprite_group.draw(Vector2Scale(pos, pixel_size));
  }

  void update(Map const& map) override {
    sprite_group.update();

    Rectangle hitbox = upscale(tile_source_hitbox(TileSource::Enemy1, vector2_to_intvec2(pos)), pixel_size);

    int west_wall = map.west_wall_of_range(hitbox.x, hitbox.y, hitbox.y + hitbox.height - 1);
    int east_wall = map.east_wall_of_range(hitbox.x + hitbox.width - 1, hitbox.y, hitbox.y + hitbox.height - 1);

    TraceLog(LOG_INFO, "West=%d East=%d", west_wall, east_wall);

    pos.x += speed.x * GetFrameTime();

    // Handle walls.
    if (speed.x > 0) {
      // Walk right.
      float wall_overlap = east_wall - (hitbox.x + hitbox.width - 1.f);
      if (wall_overlap < 0.f) {
        pos.x += wall_overlap;
        sprite_group.horizontal_flip();
        speed.x = -SimpleWalkNpcSpeed;
      }
    } else if (speed.x < 0) {
      // Walk left.
      float wall_overlap = hitbox.x - west_wall;
      if (wall_overlap < 0.f) {
        pos.x -= wall_overlap;
        sprite_group.horizontal_flip();
        speed.x = SimpleWalkNpcSpeed;
      }
    }
  }

 private:
  Vector2 pos;
  Vector2 speed{-SimpleWalkNpcSpeed, 0.f};
  int pixel_size;
  SpriteGroup sprite_group{};
  SimpleWalkNpcState state{SimpleWalkNpcState::Run};
};
