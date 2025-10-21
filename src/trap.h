#pragma once

#include "character.h"
#include "common.h"
#include "raylib.h"

struct Trap {
 public:
  virtual void draw() const = 0;
  virtual void update(Map const& map, Character& character) = 0;
  virtual Rectangle hitbox() const = 0;

  virtual ~Trap() = default;
};

struct BouncingTrap : Trap {
 public:
  BouncingTrap(Vector2 pos, int const pixel_size) : pos(pos), pixel_size(pixel_size), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap1], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
  }

  void draw() const override {
    sprite.draw(pos);
  }

  virtual void update(Map const& map, Character& character) override {
    sprite.update();
  }

  virtual Rectangle hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Trap1), pixel_size), pos);
  }

  virtual ~BouncingTrap() = default;

 private:
  Vector2 pos;
  int const pixel_size;
  Sprite sprite;
};
