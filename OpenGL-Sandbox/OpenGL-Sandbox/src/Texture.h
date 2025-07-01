#pragma once

#include <glad/glad.h>

class Texture 
{
public:
	Texture(unsigned int target, int width, int height);
	Texture(const char* filePath);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetWidth() const { return m_Width; }
	unsigned int GetHeight() const { return m_Height; }

	unsigned int GetID() const { return m_ID; }
	const char* GetFilePath() const { return m_FilePath; }

	const unsigned int GetSamples() const { return m_Samples; }

	void SetFilterMode(int mode);
	void SetSamples(int samples) { m_Samples = samples; }
	void SetData(const void* data, unsigned int internalFormat);
private:
	unsigned int m_ID;
	unsigned int m_Target, m_Samples;
	int m_Width, m_Height, m_NbChannels;
	const char* m_FilePath;
};