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

std::shared_ptr<Texture> TextureManager::Get(const std::string& path, bool gammaCorrection)
{
    auto it = m_Textures.find(path);
    if (it != m_Textures.end())
    {
        if (auto texture = it->second.lock())
            return texture;
    }

    auto texture = std::make_shared<Texture>(path.c_str(), gammaCorrection);
    m_Textures[path] = texture;
    return texture;
}

std::shared_ptr<Texture> TextureManager::Get(const std::string& name, const unsigned int width, const unsigned int height)
{
    auto it = m_Textures.find(name);
    if (it != m_Textures.end())
    {
        if (auto texture = it->second.lock())
            return texture;
    }

    auto texture = std::make_shared<Texture>(width, height);
    m_Textures[name] = texture;
    return texture;
}
