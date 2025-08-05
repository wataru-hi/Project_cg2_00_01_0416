#pragma once

#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h" // 追加

#include <string>
#include <vector>

#include <wrl/client.h>



class TextureManager
{
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

public:
	//シングルトンインスタンスの取得
	static TextureManager* GetInstance();

	void Initialize();

	void Finalize();

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <pram name="filePath">テクスチャファイルのパス</param>
	void LoadTexture(const std::string& filePath);

private:
	// テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	//テクスチャデータ
	std::vector<TextureData> textureDatas;
};

