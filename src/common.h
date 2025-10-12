#pragma once

#include <cstdio>
#include <functional>

#include "raylib.h"
#include "raymath.h"

constexpr int const REFERENCE_FPS{144};

constexpr int const PIXEL_SIZE{2};
constexpr int const TILE_SIZE{16};
constexpr int const TILE_SIZE_PX{TILE_SIZE * PIXEL_SIZE};

constexpr Vector2 const vector_zero{0.f, 0.f};

constexpr int const COLLISION_TYPE_NOTHING{0b0};
constexpr int const COLLISION_TYPE_TOP{0b0001};
constexpr int const COLLISION_TYPE_BOTTOM{0b0010};
constexpr int const COLLISION_TYPE_LEFT{0b0100};
constexpr int const COLLISION_TYPE_RIGHT{0b1000};
constexpr int const COLLISION_TYPE_ALL{0b1111};

constexpr int tileset_tile_collision_map[16 * 11]{
    // clang-format off
    1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
    // clang-format on
};

#define PANIC(...)                \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE);           \
  }

struct IntVec2 {
  int x{0};
  int y{0};

  bool operator==(const IntVec2& other) const {
    return x == other.x && y == other.y;
  }

  Vector2 to_vector2() const {
    return Vector2{static_cast<float>(x), static_cast<float>(y)};
  }

  void write(FILE* file) const {
    int pack[2]{x, y};
    fwrite(pack, sizeof(int), 2, file);
  }

  IntVec2 scale(int const scale) const {
    return IntVec2{x * scale, y * scale};
  }
};

constexpr IntVec2 const intvec2_0_0{0, 0};
constexpr IntVec2 const intvec2_4_4{4, 4};

IntVec2 intvec2_from_file(FILE* file) {
  IntVec2 out{};

  if (std::fread(&out.x, sizeof(int), 1, file) != 1) PANIC("Failed reading x");
  if (std::fread(&out.y, sizeof(int), 1, file) != 1) PANIC("Failed reading y");

  return out;
}

IntVec2 tile_coord_from_absolute(Vector2 const v) {
  return IntVec2{static_cast<int>(v.x / TILE_SIZE_PX), static_cast<int>(v.y / TILE_SIZE_PX)};
}

namespace std {
template <>
struct hash<IntVec2> {
  std::size_t operator()(const IntVec2& v) const noexcept {
    std::size_t h1 = std::hash<int>{}(v.x);
    std::size_t h2 = std::hash<int>{}(v.y);
    return h1 ^ (h2 << 1);
  }
};
}  // namespace std

struct Stepper {
 public:
  Stepper() {
  }
  Stepper(unsigned int threshold) : threshold(threshold) {
  }

  void set_threshold(unsigned int new_threshold) {
    threshold = new_threshold;
    reset();
  }

  void reset() {
    counter = 0;
  }

  bool update() {
    if (counter >= threshold) {
      counter = 0;
      return true;
    } else {
      counter++;
      return false;
    }
  }

 private:
  unsigned int counter{0};
  unsigned int threshold{};
};

enum TileSource {
  Gui,
  Tileset,
  Box1,
};

constexpr IntVec2 const TILESIZE_DEFAULT{TILE_SIZE, TILE_SIZE};
constexpr IntVec2 const TILESIZE_BOX{32, 32};

struct TileSelection {
  TileSource source{};
  IntVec2 tile_coord{};

  void draw(Vector2 const pos) const {
    std::shared_ptr<Texture2D> texture;
    if (source == TileSource::Gui) {
      texture = asset_manager.textures[TextureNames::GuiTiles];
    } else if (source == TileSource::Tileset) {
      texture = asset_manager.textures[TextureNames::TilesetTiles];
    } else if (source == TileSource::Box1) {
      texture = asset_manager.textures[TextureNames::Box1__Idle];
    } else {
      PANIC("Invalid tile source");
    }

    IntVec2 _tile_size{tile_size()};
    DrawTexturePro(
        *texture,
        {static_cast<float>(tile_coord.x * TILE_SIZE), static_cast<float>(tile_coord.y * TILE_SIZE),
         static_cast<float>(_tile_size.x), static_cast<float>(_tile_size.y)},
        {pos.x, pos.y, static_cast<float>(_tile_size.x * PIXEL_SIZE), static_cast<float>(_tile_size.y * PIXEL_SIZE)},
        Vector2Zero(), 0.f, WHITE);
  }

  void write(FILE* file) const {
    fwrite(&source, sizeof(int), 1, file);
    tile_coord.write(file);
  }

  bool collide_from(int direction) const {
    if (source == TileSource::Gui) {
      return true;
    } else if (source == TileSource::Tileset) {
      return (tileset_tile_collision_map[tile_coord.y * 16 + tile_coord.x] & direction) > 0;
    } else {
      PANIC("Invalid tile source for collision check");
    }
  }

  IntVec2 const tile_size() const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
        return TILESIZE_DEFAULT;
      case TileSource::Box1:
        return TILESIZE_BOX;
      default:
        PANIC("Invalid tile source for tilesize");
    }
  }

  int const snap() const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
        return 16;
      case TileSource::Box1:
        return 1;
      default:
        PANIC("Invalid tile source for snap");
    }
  }
};

TileSelection tile_selection_from_file(FILE* file) {
  int tile_source_raw{};
  if (fread(&tile_source_raw, sizeof(int), 1, file) != 1) PANIC("Cannot read tile source");
  IntVec2 pos = intvec2_from_file(file);

  TileSource source{};
  switch (tile_source_raw) {
    case 0:
      source = TileSource::Gui;
      break;
    case 1:
      source = TileSource::Tileset;
      break;
    default:
      TraceLog(LOG_ERROR, "Invalid tile source at reading: %d", tile_source_raw);
      break;
  }

  return TileSelection{source, pos};
}

inline int mod_reduced(const int v, const int mod) {
  return v - (v % mod);
}

constexpr Rectangle upscale(Rectangle const rect, float const scale) {
  return Rectangle{rect.x * scale, rect.y * scale, rect.width * scale, rect.height * scale};
}

IntVec2 relative_frame_pos(Rectangle const frame, int const tile_size, int const pixel_size) {
  Vector2 mouse_pos = GetMousePosition();
  return IntVec2{mod_reduced(mouse_pos.x - frame.x, tile_size * pixel_size) / pixel_size,
                 mod_reduced(mouse_pos.y - frame.y, tile_size * pixel_size) / pixel_size};
}

float fps_independent_multiplier() {
  return static_cast<float>(REFERENCE_FPS) / static_cast<float>(GetFPS());
}

void fps_independent_multiply(float* v, float mul) {
  *v *= powf(mul, fps_independent_multiplier());
}
