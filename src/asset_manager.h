#pragma once

#include <memory>
#include <unordered_map>

#include "raylib.h"

enum TextureNames {
  Character1__Run,
  Character1__Idle,
  Character1__Hit,
};

struct AssetManager {
 public:
  std::unordered_map<int, std::shared_ptr<Texture2D>> textures{};

  // Must be the last thing called.
  void unload_assets() {
    TraceLog(LOG_INFO, "Unload all textures");

    for (auto [k, v] : textures) {
      UnloadTexture(*v);
    }
  }

  void preload() {
    textures[TextureNames::Character1__Run] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Run.png"));
    textures[TextureNames::Character1__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Idle.png"));
    textures[TextureNames::Character1__Hit] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Hit.png"));
  }

 private:
};

static AssetManager asset_manager{};
