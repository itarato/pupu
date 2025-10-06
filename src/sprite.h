#pragma once

#include <memory>

#include "common.h"
#include "raylib.h"
#include "raymath.h"

struct Sprite {
 public:
  ~Sprite() {
  }
  Sprite(float pixel_size = 1.f) : pixel_size(pixel_size) {
  }
  Sprite(float pixel_size, std::shared_ptr<Texture2D> texture, Vector2 size, int frame_count, unsigned int frame_length)
      : pixel_size(pixel_size),
        texture(std::move(texture)),
        size(size),
        frame_count(frame_count),
        frame_stepper(frame_length) {
  }

  void reset() {
    frame_stepper.reset();
    current_frame = 0;
    horizontal_reverse = 1;
  }

  void init_texture(std::shared_ptr<Texture2D> new_texture, Vector2 new_size, int new_frame_count,
                    unsigned int frame_length) {
    texture = std::move(new_texture);
    size = new_size;
    frame_count = new_frame_count;
    frame_stepper.set_threshold(frame_length);
  }

  void draw(Vector2 const &pos) const {
    DrawTexturePro(*texture, {size.x * current_frame, 0.f, size.x * horizontal_reverse, size.y},
                   {pos.x - origin.x, pos.y - origin.y, size.x * pixel_size, size.y * pixel_size}, origin, 0.f, WHITE);
  }

  /**
   * Returns true on finishing a cycle.
   */
  bool update() {
    if (frame_count == 1) return false;

    if (frame_stepper.update()) {
      current_frame = (current_frame + 1) % frame_count;

      return current_frame == 0;
    }

    return false;
  }

  void set_pixel_size(float new_pixel_size) {
    pixel_size = new_pixel_size;
  }

  void horizontal_flip() {
    horizontal_reverse = -1;
  }

  void horizontal_reset() {
    horizontal_reverse = 1;
  }

 private:
  float pixel_size{1.f};
  std::shared_ptr<Texture2D> texture;
  Vector2 size{};
  int frame_count{1};
  Stepper frame_stepper{};
  Vector2 origin{0.f, 0.f};
  int current_frame{0};
  int horizontal_reverse{1};
};
