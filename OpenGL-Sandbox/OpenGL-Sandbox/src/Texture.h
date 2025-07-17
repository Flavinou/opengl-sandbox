#pragma once

#include <glm/glm.hpp>

class Texture 
{
public:
    Texture(int width, int height);
	Texture(const char* filePath);
	Texture(const char* filePath, bool gammaCorrection);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetWidth() const { return m_Width; }
	unsigned int GetHeight() const { return m_Height; }

	unsigned int GetID() const { return m_ID; }
	const char* GetFilePath() const { return m_FilePath; }

    void SetFilterMode(int mode);
    void SetWrapMode(int mode);
	void SetBorderColor(const glm::vec4& color);

    void SetData(const void* data, unsigned int internalFormat);
    void SetData(const void* data, unsigned int internalFormat, unsigned int dataFormat, unsigned int type) const;
private:
	unsigned int m_ID;
	int m_Width, m_Height, m_NbChannels;
	const char* m_FilePath;
};

class Cubemap
{
public:
	Cubemap(int width, int height);
    ~Cubemap();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    unsigned int GetID() const { return m_ID; }

    void SetFilterMode(int mode);
    void SetWrapMode(int mode);

	void SetData(const void* data, unsigned int internalFormat) const;
private:
    unsigned int m_ID;
	int m_Width, m_Height;
};