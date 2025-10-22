#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"

struct InteractiveObject {
 public:
  virtual void draw() const = 0;
  virtual void update() = 0;
  virtual ~InteractiveObject() = default;
};

struct DisappearingPlank : InteractiveObject {
 public:
  DisappearingPlank(int const pixel_size, Vector2 const pos) : pixel_size(pixel_size), pos(pos), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap5], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
  }

  void draw() const override {
    sprite.draw(pos);
  }

  void update() override {
    sprite.update();
  }

  ~DisappearingPlank() = default;

 private:
  int const pixel_size;
  Vector2 const pos;
  Sprite sprite;
};
