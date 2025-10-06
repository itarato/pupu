#pragma once

#include <memory>
#include <unordered_map>

#include "raylib.h"

constexpr int BACKGROUND_COUNT{6};

enum TextureNames {
  Character1__Run,
  Character1__Idle,
  Character1__Hit,
  Character1__Jump,
  Character1__Fall,
  Background__0,
  Background__1,
  Background__2,
  Background__3,
  Background__4,
  Background__5,
  GuiTiles,
  TilesetTiles,
};

struct AssetManager {
 public:
  std::unordered_map<int, std::shared_ptr<Texture2D>> textures{};

  // Must be the last thing called.
  void unload_assets() {
    TraceLog(LOG_INFO, "Unload all textures");

    for (auto [k, v] : textures) UnloadTexture(*v);
  }

  void preload() {
    textures[TextureNames::Character1__Run] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Run.png"));
    textures[TextureNames::Character1__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Idle.png"));
    textures[TextureNames::Character1__Hit] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Hit.png"));
    textures[TextureNames::Character1__Jump] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Jump.png"));
    textures[TextureNames::Character1__Fall] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Fall.png"));

    textures[TextureNames::Background__0] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/1.png"));
    textures[TextureNames::Background__1] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/2.png"));
    textures[TextureNames::Background__2] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/3.png"));
    textures[TextureNames::Background__3] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/4.png"));
    textures[TextureNames::Background__4] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/5.png"));
    textures[TextureNames::Background__5] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/6.png"));

    textures[TextureNames::GuiTiles] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/GUI.png"));
    textures[TextureNames::TilesetTiles] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Tileset.png"));
  }

 private:
};

static AssetManager asset_manager{};
