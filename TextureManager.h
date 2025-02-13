#pragma once
#include <string>
class TextureManager
{
private:
	static TextureManager* instance;

public:
	static TextureManager* GetInstance();

	void Finitialize();

private:
	struct TextureData {
		std::string filePath;
		DirectX::TexMetaData metaData;
	};

#pragma region 複製の禁止
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = default;
#pragma endregion
};

