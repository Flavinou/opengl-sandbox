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

Cubemap::Cubemap(const std::string* filePaths)
{
    // Load the cubemap face from the file paths
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

    int width, height, nbChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < 6; i++)
    {
		m_FilePaths[i] = filePaths[i];

        stbi_set_flip_vertically_on_load(false);

        data = stbi_load(m_FilePaths[i].c_str(), &width, &height, &nbChannels, 0);
        if (data)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "[ERROR] Cubemap texture failed to load at path: " << m_FilePaths[i] << std::endl;
        }

        stbi_set_flip_vertically_on_load(true);
    }

    // Assign texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
