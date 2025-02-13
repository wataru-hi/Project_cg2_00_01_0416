#include "TextureManager.h"
#include "DirectXCommon.h"
#include "Logger.h"
using namespace Logger;

#include "StringUtility.h"
using namespace StringUtility;


void TextureManager::Initialize(DirectXCommon* dxc)
{
	dxCommon = dxc;
	textureDatas.reserve(dxCommon->kMaxSRVcount);
}

TextureManager* TextureManager::GetInstance()
{
	
	if (instance == nullptr)
	{
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finitialize()
{
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	//テクスチャファイルを選んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	
	textureDatas.resize(textureDatas.size() + 1);
	TextureData& textureData = textureDatas.back();

	textureData.filePath = filePath;
	textureData.metaData = image.GetMetadata();
	textureData.resources = dxCommon->CreateTextureResource(dxCommon->GetDevice(), textureData.metaData);

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1);

	textureData.srvHandleCPU = dxCommon->GetCPUDescriptorHandle()

	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

	if (FAILED(hr)) {
		Log("Failed to load image: " + std::to_string(hr));
		return;
	}

	//ミップマップの生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	if (FAILED(hr)) {
		Log("GenerateMipMaps failed: " + std::to_string(hr));
		throw std::runtime_error("GenerateMipMaps failed!");
	}
}
