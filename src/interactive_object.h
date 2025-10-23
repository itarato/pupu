#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"

struct InteractiveObject {
 public:
  virtual ~InteractiveObject() = default;

  virtual void draw() const = 0;
  virtual void update() = 0;
  virtual Rectangle const hitbox() const = 0;
  virtual int collision_directions() const = 0;
};

struct DisappearingPlank : InteractiveObject {
 public:
  DisappearingPlank(int const pixel_size, Vector2 const pos) : pixel_size(pixel_size), pos(pos), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap5], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
    sprite.stop();
  }

  ~DisappearingPlank() = default;

  void draw() const override {
    sprite.draw(pos);
    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
  }

  void update() override {
    sprite.update();
  }

  Rectangle const hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Trap5), pixel_size), pos);
  }

  int collision_directions() const override {
    return COLLISION_TYPE_TOP;
  }

 private:
  int const pixel_size;
  Vector2 const pos;
  Sprite sprite;
};
