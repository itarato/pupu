#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "raymath.h"
#include "sprite.h"
#include "sprite_group.h"

constexpr Vector2 const SIMPLE_WALK_NPC_SIZE{48.f, 48.f};

constexpr int const SimpleWalkNpsSpriteFall{0};
constexpr int const SimpleWalkNpsSpriteHit{1};
constexpr int const SimpleWalkNpsSpriteIdle{2};
constexpr int const SimpleWalkNpsSpriteJump{3};
constexpr int const SimpleWalkNpsSpriteRun{4};

struct Npc {
 public:
  virtual void draw() const = 0;
  virtual void update() = 0;
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

        sprite_group.set_current_sprite(SimpleWalkNpsSpriteIdle);
        break;
      default:
        BAILF("Invalid: %d", tile_source);
    }
  }

  void draw() const override {
    sprite_group.draw(Vector2Scale(pos, pixel_size));
  }

  void update() override {
    sprite_group.update();
  }

 private:
  Vector2 pos;
  int pixel_size;
  SpriteGroup sprite_group{};
};
