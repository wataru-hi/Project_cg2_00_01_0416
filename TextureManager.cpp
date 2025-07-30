#include "TextureManager.h"
#include "DirectXCommon.h"
#include "Logger.h"
using namespace Logger;

#include "StringUtility.h"
using namespace StringUtility;
#include <cassert>

//ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::ksrvIndexTop = 1;

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
	//読み込み済みテクスチャを検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textuerDate) {return textuerDate.filePath == filePath; }
	);
	if (it != textureDatas.end()){
		return;
	}

	//テクスチャ枚数上限チェック
	assert(textureDatas.size() + ksrvIndexTop < DirectXCommon::kMaxSRVcount);

	//テクスチャファイルを選んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	
	//テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	//追加したテクスチャデータの参照を取得
	TextureData& textureData = textureDatas.back();

	textureData.filePath = filePath;
	textureData.metaData = image.GetMetadata();
	textureData.resources = dxCommon->CreateTextureResource(dxCommon->GetDevice(), textureData.metaData);

	//テクスチャデータの要素取得番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + ksrvIndexTop;

	textureData.srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metaData.format; // 画像のフォーマットに合わせる
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャとして扱う
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // デフォルトのマッピング
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(textureData.metaData.mipLevels); // ミップマップレベル数
	// SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureData.resources.Get(), &srvDesc, textureData.srvHandleCPU);
}

uint32_t TextureManager::GetTextureIndexFilePath(const std::string& filePath)
{
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textuerDate) {return textuerDate.filePath == filePath; }
	);

	if (it != textureDatas.end()) {
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandelGpu(uint32_t textureIndex)
{
	assert(textureIndex < textureDatas.size());
	TextureData& textureData = textureDatas[textureIndex];
	return textureData.srvHandleGPU;
}
