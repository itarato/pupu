#pragma once

#include <cstdio>
#include <functional>

#include "raylib.h"
#include "raymath.h"

#define INFO(...) log("\x1b[90mINFO\x1b[0m", __FILE__, __LINE__, __VA_ARGS__)
#define WARN(...) log("\x1b[93mWARN\x1b[0m", __FILE__, __LINE__, __VA_ARGS__)
#define DEBUG(...) log("\x1b[94mDEBG\x1b[0m", __FILE__, __LINE__, __VA_ARGS__)
#define PANIC(...)                                                \
  {                                                               \
    log("\x1b[94mPANIC\x1b[0m", __FILE__, __LINE__, __VA_ARGS__); \
    exit(EXIT_FAILURE);                                           \
  }
#define BAIL                                                             \
  {                                                                      \
    fprintf(stderr, "\x1b[94mBAIL\x1b[0m in %s:%d", __FILE__, __LINE__); \
    exit(EXIT_FAILURE);                                                  \
  }

constexpr int const REFERENCE_FPS{144};
constexpr int const DEFAULT_PIXEL_SIZE{2};

constexpr int const TILE_SIZE{16};

constexpr Vector2 const vector_zero{0.f, 0.f};

constexpr int const COLLISION_TYPE_NOTHING{0b0};
constexpr int const COLLISION_TYPE_TOP{0b0001};
constexpr int const COLLISION_TYPE_BOTTOM{0b0010};
constexpr int const COLLISION_TYPE_LEFT{0b0100};
constexpr int const COLLISION_TYPE_RIGHT{0b1000};
constexpr int const COLLISION_TYPE_ALL{0b1111};

constexpr Rectangle const DEFAULT_TILE_HITBOX{0.f, 0.f, TILE_SIZE, TILE_SIZE};
constexpr Rectangle const BOX_HITBOX{5.f, 5.f, 22.f, 22.f};

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

void log_va_list(const char* level, const char* fileName, int lineNo, const char* s, va_list args) {
  printf("[%s][\x1b[93m%s\x1b[39m:\x1b[96m%d\x1b[0m] \x1b[94m", level, fileName, lineNo);
  vprintf(s, args);
  printf("\x1b[0m\n");
}

void log(const char* level, const char* fileName, int lineNo, const char* s, ...) {
  va_list args;
  va_start(args, s);
  log_va_list(level, fileName, lineNo, s, args);
  va_end(args);
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

IntVec2 tile_coord_from_absolute(Vector2 const v, int const pixel_size) {
  return IntVec2{static_cast<int>(v.x / (TILE_SIZE * pixel_size)), static_cast<int>(v.y / (TILE_SIZE * pixel_size))};
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

constexpr Rectangle upscale(Rectangle const rect, float const scale) {
  return Rectangle{rect.x * scale, rect.y * scale, rect.width * scale, rect.height * scale};
}

constexpr Rectangle move(Rectangle const rect, IntVec2 const v) {
  return Rectangle{rect.x + v.x, rect.y + v.y, rect.width, rect.height};
}

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

  void draw(Vector2 const pos, int const pixel_size) const {
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
        {pos.x, pos.y, static_cast<float>(_tile_size.x * pixel_size), static_cast<float>(_tile_size.y * pixel_size)},
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
        BAIL;
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
        BAIL;
    }
  }

  Rectangle const hitbox(IntVec2 const pos) const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
        return move(DEFAULT_TILE_HITBOX, pos);
      case TileSource::Box1:
        return move(BOX_HITBOX, pos);
      default:
        BAIL;
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
    case 2:
      source = TileSource::Box1;
      break;
    default:
      PANIC("Invalid tile source at reading: %d", tile_source_raw);
  }

  return TileSelection{source, pos};
}

inline int mod_reduced(const int v, const int mod) {
  return v - (v % mod);
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
