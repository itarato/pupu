#pragma once

#include <cstdio>
#include <functional>

#include "raylib.h"

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
    std::fprintf(file, "%d%d", &x, &y);
  }
};

IntVec2 intvec2_from_file(FILE* file) {
  int x, y;
  std::fscanf(file, "%d%d", x, y);

  return IntVec2{x, y};
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
};

struct TileSelection {
  TileSource source;
  IntVec2 tile_pos;

  void draw(Vector2 const pos, int const tile_size, int const pixel_size) const {
    std::shared_ptr<Texture2D> texture;
    if (source == TileSource::Gui) {
      texture = asset_manager.textures[TextureNames::GuiTiles];
    } else if (source == TileSource::Tileset) {
      texture = asset_manager.textures[TextureNames::TilesetTiles];
    } else {
      TraceLog(LOG_ERROR, "Invalid tile source");
      return;
    }

    DrawTexturePro(
        *texture,
        {static_cast<float>(tile_pos.x), static_cast<float>(tile_pos.y), static_cast<float>(tile_size),
         static_cast<float>(tile_size)},
        {pos.x, pos.y, static_cast<float>(tile_size * pixel_size), static_cast<float>(tile_size * pixel_size)},
        Vector2Zero(), 0.f, WHITE);
  }

  void write(FILE* file) const {
    std::fprintf(file, "%d", source);
    tile_pos.write(file);
  }
};

TileSelection tile_selection_from_file(FILE* file) {
  int tile_source_raw;
  std::fscanf(file, "%d", &tile_source_raw);
  IntVec2 pos = intvec2_from_file(file);

  TileSource source;
  switch (tile_source_raw) {
    case 0:
      source = TileSource::Gui;
      break;
    case 1:
      source = TileSource::Tileset;
      break;
    default:
      TraceLog(LOG_ERROR, "Invalid tile source at reading");
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
