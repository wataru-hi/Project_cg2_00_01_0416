#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>

#include "Mymath.h"
using namespace mymath;


#include "Transform.h"

class DirectXCommon;
class SpriteCommon;
class WinApp;

class Sprite
{
public:
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon, std::string textureFilePath);
	void Update(WinApp* winApp);
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle);

	const Vector2& GetPosition() const { return position; }
	void SetPosition(const Vector2& position) {this->position = position; }

	~Sprite();
private:
	SpriteCommon* spriteCommon_;
	DirectXCommon* dxCommon_;
	WinApp* winApp_;

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//バッファリソース内のデータを示すポインタ
	VertexData* vertexDate = nullptr;
	uint32_t* indexDate = nullptr;
	Material* materialDate = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	//バッファリソースの使い方を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferVier;
	D3D12_INDEX_BUFFER_VIEW indexBufferVier;

	Transform transform;
	Vector2 position = {0.0f, 0.0f};

	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionmatrix;

	uint32_t textureIndex = 0;

	void CreateResources();
	void CreateVertexBufferView();
	void CreateIndexBufferView();
	void CreateMaterialResources();
	void CreateTransformMatirxResources();
};

