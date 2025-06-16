#include "TextureManager.h"

TextureManager& TextureManager::Instance()
{
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::Get(const std::string& path)
{
    auto it = m_Textures.find(path);
    if (it != m_Textures.end())
    {
        if (auto texture = it->second.lock())
            return texture;
    }

    auto texture = std::make_shared<Texture>(path.c_str());
    m_Textures[path] = texture;
    return texture;
}
