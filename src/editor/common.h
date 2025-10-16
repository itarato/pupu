#pragma once

#include "../common.h"
#include "imgui.h"
#include "raylib.h"

IntVec2 relative_frame_pos(ImVec2 const pos, int const tile_size, int const pixel_size) {
  return IntVec2{mod_reduced(pos.x, tile_size * pixel_size) / pixel_size,
                 mod_reduced(pos.y, tile_size * pixel_size) / pixel_size};
}
