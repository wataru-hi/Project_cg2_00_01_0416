#include "TextureManager.h"

#include "DirectXCommon.h"

#include "Logger.h"
using namespace Logger;

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr){
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize() {
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath) {

	//テクスチャファイルを選んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	textureDatas.resize(textureDatas.size() + 1);
	TextureData& textureData = textureDatas.back();

	textureData.filePath = filePath;
	textureData.metadata = 

	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

	if (FAILED(hr)) {
		Log("Failed to load image: \n" + std::to_string(hr));
		return {};
	}

	//ミップマップの生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	if (FAILED(hr)) {
		Log("GenerateMipMaps failed: " + std::to_string(hr));
		throw std::runtime_error("GenerateMipMaps failed!");
	}
}