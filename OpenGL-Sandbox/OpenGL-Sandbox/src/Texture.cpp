#include "Texture.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

#include <iostream>

Texture::Texture(const char* filePath)
	: m_FilePath(filePath), m_ID(0), m_Width(0), m_Height(0), m_NbChannels(0)
{
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file path
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load image data
	unsigned char* data = stbi_load(filePath, &m_Width, &m_Height, &m_NbChannels, 0);

	// Support both RGB and RGBA
	GLenum internalFormat = 0, dataFormat = 0;
	if (m_NbChannels == 4)
	{
		internalFormat = GL_RGBA;
		dataFormat = GL_RGBA;
	}
	else if (m_NbChannels == 3)
	{
		internalFormat = GL_RGB;
		dataFormat = GL_RGB;
	}

	// Check if the image was loaded successfully, then set the texture data
	if (data)
	{
	    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
	    glGenerateMipmap(GL_TEXTURE_2D);
	    stbi_image_free(data);
	}
	else // otherwise, print an error message
	{
		std::cout << "[ERROR]: Failed to load texture '" << m_FilePath << "' !" << std::endl;
	}
}

Texture::Texture(const char* filePath, bool gammaCorrection)
    : m_FilePath(filePath), m_ID(0), m_Width(0), m_Height(0), m_NbChannels(0)
{
    stbi_set_flip_vertically_on_load(true);

    // Load the texture from the file path
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image data
    unsigned char* data = stbi_load(filePath, &m_Width, &m_Height, &m_NbChannels, 0);

    // Support both RGB and RGBA
    GLenum internalFormat = 0, dataFormat = 0;
    if (m_NbChannels == 4)
    {
        internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
    }
    else if (m_NbChannels == 3)
    {
        internalFormat = gammaCorrection ? GL_SRGB : GL_SRGB;
        dataFormat = GL_RGB;
    }

    // Check if the image was loaded successfully, then set the texture data
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else // otherwise, print an error message
    {
        std::cout << "[ERROR]: Failed to load texture '" << m_FilePath << "' !" << std::endl;
    }
}


Texture::Texture(int width, int height)
    : m_FilePath(nullptr), m_ID(0), m_Width(width), m_Height(height)
{
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_ID);
}

void Texture::Bind(unsigned int slot) const
{
	// Bind the texture to the specified slot
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetFilterMode(int mode)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
}

void Texture::SetWrapMode(int mode)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
}

void Texture::SetBorderColor(const glm::vec4& color)
{
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
}

void Texture::SetData(const void* data, unsigned int internalFormat)
{
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
}

void Texture::SetData(const void* data, unsigned int internalFormat, unsigned int dataFormat, unsigned int type) const
{
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, type, data);
}

Cubemap::Cubemap(int width, int height)
	: m_Width(width), m_Height(height)
{
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

Cubemap::~Cubemap()
{
    glDeleteTextures(1, &m_ID);
}

void Cubemap::Bind(unsigned int slot /*= 0*/) const
{
    // Bind the texture to the specified slot
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

void Cubemap::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::SetFilterMode(int mode)
{
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mode);
}

void Cubemap::SetWrapMode(int mode)
{
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, mode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mode);
}

void Cubemap::SetData(const void* data, unsigned int internalFormat) const
{
	for (unsigned int i = 0; i < 6; i++)
	{
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
            m_Width, m_Height, 0, internalFormat, GL_FLOAT, data);
	}
}
