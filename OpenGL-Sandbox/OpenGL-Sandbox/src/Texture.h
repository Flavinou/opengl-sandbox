#pragma once

class Texture 
{
public:
	Texture(const char* filePath);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetWidth() const { return m_Width; }
	unsigned int GetHeight() const { return m_Height; }

	unsigned int GetID() const { return m_ID; }
	const char* GetFilePath() const { return m_FilePath; }
private:
	unsigned int m_ID;
	int m_Width, m_Height, m_NbChannels;
	const char* m_FilePath;
};