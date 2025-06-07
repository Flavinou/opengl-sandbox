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

void Texture::SetWrapMode(int wrapS, int wrapT) const
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)wrapT);
}
