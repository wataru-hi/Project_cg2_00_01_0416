#pragma once
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include <wrl.h>
#include <d3d12.h>
#include <vector>

class DirectXCommon;

class TextureManager
{
private:
	static TextureManager* instance;

#pragma region 複製の禁止
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = default;
#pragma endregion

public:
	void Initialize(DirectXCommon* dxc);

	static TextureManager* GetInstance();

	void Finitialize();

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	void LoadTexture(const std::string& filePath);


	uint32_t GetTextureIndexFilePath(const std::string& filePath);

	//テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandelGpu(uint32_t textureIndex);

private:
	struct TextureData {
		std::string filePath;//画像のファイルパス
		DirectX::TexMetadata metaData;//画像の幅や高さ
		Microsoft::WRL::ComPtr<ID3D12Resource> resources;//テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	DirectXCommon* dxCommon;

	std::vector<TextureData> textureDatas;

	//SRVインデックス開始番号
	static uint32_t ksrvIndexTop;


};

