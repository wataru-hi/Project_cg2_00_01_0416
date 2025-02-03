#include "TextureManager.h"
#include "DirectXCommon.h"

TextureManager* TextureManager::instance = nullptr;

uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	textureDatas.reserve(DirectXCommon::kMaxSRVConst);

	dxCommon_ = dxCommon;
}

void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) { return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		// 読み込み済みなら早期return
		return;
	}

	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVConst);

	DirectX::ScratchImage Image = dxCommon_->LoadTexture(filePath);
#pragma region テクスチャデータを読んでプログラムで扱えるようにする
	textureDatas.resize(textureDatas.size() + 1);//テクスチャデータを追加
	TextureData& textureDate = textureDatas.back();//追加したデータの参照を取得する

	textureDate.filePath = filePath;
	textureDate.metadata = Image.GetMetadata();
	textureDate.resource = dxCommon_->CreateTextureResource(textureDate.metadata);

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureDate.srvHandleCPU = dxCommon_->GetSRVCPUDescriptorHandle(srvIndex);
	textureDate.srvHandleGPU = dxCommon_->GetSRVGPUDescriptorHandle(srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDate.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(textureDate.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureDate.resource.Get(), &srvDesc, textureDate.srvHandleCPU);
#pragma endregion
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filepath)
{
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) { return textureData.filePath == filepath; }
	);
	if (it != textureDatas.end()) {
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	assert(textureIndex < textureDatas.size());

	TextureData& textureDate = textureDatas.back();
	return textureDate.srvHandleGPU;
}
