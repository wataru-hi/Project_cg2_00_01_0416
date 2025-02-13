#pragma once
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include <wrl.h>
#include <d3d12.h>
#include <vector>
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
	static TextureManager* GetInstance();

	void Finitialize();

private:
	struct TextureData {
		std::string filePath;//画像のファイルパス
		DirectX::TexMetadata metaData;//画像の幅や高さ
		Microsoft::WRL::ComPtr<ID3D12Resource> resources;//テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	std::vector<TextureData> textureData;

};

