#pragma once

#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include <wrl.h>
#include <d3d12.h>
#include <vector>

class DirectXCommon;

class TextureManager
{
public:
	static TextureManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	void LoadTexture(const std::string& filePath);

	uint32_t GetTextureIndexByFilePath(const std::string& filepath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);
private:
	static TextureManager* instance;

	struct TextureData {
		std::string filePath; // 画像のファイルパス
		DirectX::TexMetadata metadata; // サイズ、フォーマットなど
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; //テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; // SRV作成時のCPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; // 描画時のGPUハンドル
	};

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

private:
	std::vector<TextureData> textureDatas;

	DirectXCommon* dxCommon_;

	static uint32_t  kSRVIndexTop;
};

