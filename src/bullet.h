#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite_group.h"

struct Bullet {
 public:
  Bullet(Vector2 const pos, int const pixel_size, float const speed, int const west_wall, int const east_wall)
      : pos(pos), pixel_size(pixel_size), speed(speed), west_wall(west_wall), east_wall(east_wall) {
    sprite_group.push_sprite(
        Sprite{static_cast<float>(pixel_size), asset_manager.textures[TextureNames::BulletShort],
               Vector2{static_cast<float>(asset_manager.textures[TextureNames::BulletShort]->width),
                       static_cast<float>(asset_manager.textures[TextureNames::BulletShort]->height)},
               1, 0});
  }

  void draw() const {
    sprite_group.draw(pos);
  }

  void update() {
    sprite_group.update();
    pos.x += speed * GetFrameTime();
  }

  bool is_dead() const {
    return pos.x < west_wall || pos.x > east_wall;
  }

  Rectangle hitbox() const {
    std::shared_ptr<Texture2D> const texture = sprite_group.current_sprite().get_texture();
    return Rectangle{pos.x, pos.y, static_cast<float>(texture->width), static_cast<float>(texture->height)};
  }

  void set_target_hit() {
    target_hit = true;
  }

 private:
  Vector2 pos;
  int const pixel_size;
  float const speed;
  SpriteGroup sprite_group{};
  int const west_wall;
  int const east_wall;
  bool target_hit{false};
};
