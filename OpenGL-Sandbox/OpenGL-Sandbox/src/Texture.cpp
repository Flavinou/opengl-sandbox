#include "Texture.h"

#include <stb_image/stb_image.h>

#include <iostream>

Texture::Texture(unsigned int target, int width, int height)
	: m_Target(target), m_FilePath(nullptr), m_ID(0), m_Width(width), m_Height(height), m_NbChannels(0), m_Samples(0)
{
	glGenTextures(1, &m_ID);
	glBindTexture(m_Target, m_ID);
}

Texture::Texture(const char* filePath)
	: m_Target(GL_TEXTURE_2D), m_FilePath(filePath), m_ID(0), m_Width(0), m_Height(0), m_NbChannels(0), m_Samples(0)
{
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file path
	glGenTextures(1, &m_ID);
	glBindTexture(m_Target, m_ID);

	// Set texture parameters
	glTexParameteri(m_Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(m_Target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
	    glTexImage2D(m_Target, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
	    glGenerateMipmap(m_Target);
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
	glBindTexture(m_Target, m_ID);
}

void Texture::Unbind() const
{
	glBindTexture(m_Target, 0);
}

void Texture::SetFilterMode(int mode)
{
    glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, mode);
    glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, mode);
}

void Texture::SetData(const void* data, unsigned int internalFormat)
{
	switch (m_Target)
	{
		case GL_TEXTURE_2D:
			glTexImage2D(m_Target, 0, internalFormat, m_Width, m_Height, 0, internalFormat, GL_UNSIGNED_BYTE, data);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTexImage2DMultisample(m_Target, m_Samples, internalFormat, m_Width, m_Height, GL_TRUE);
			break;
	}
}
