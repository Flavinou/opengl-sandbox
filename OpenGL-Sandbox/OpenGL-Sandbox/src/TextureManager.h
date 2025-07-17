#pragma once

#include "Texture.h"

#include <memory>
#include <string>
#include <unordered_map>

class TextureManager
{
public:
    static TextureManager& Instance();

    std::shared_ptr<Texture> Get(const std::string& path);
    std::shared_ptr<Texture> Get(const std::string& path, bool gammaCorrection);
    std::shared_ptr<Texture> Get(const std::string& name, const unsigned int width, const unsigned int height);
private:
    TextureManager() = default;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_Textures;
};