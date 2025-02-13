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
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon);
	void Update(WinApp* winApp);
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle);

	const Vector2& GetPosition() const { return position; }
	void SetPosition(const Vector2& position) {this->position = position; }
	
	const float& GetRotate() const { return rotate; }
	void SetRotate(const float& rotate) {this->rotate = rotate; }

	const Vector4& GetColor() const {return color; }
	void SetColor(const Vector4& color) { materialDate->color = color; }

	const Vector2& GetSize() const { return size; }
	void SetSize(const Vector2& size) {this->size = size; }

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
	float rotate = 0.0f;
	Vector4 color = Vector4{ 1.0f,1.0f,1.0f,1.0f};
	Vector2 size = Vector2{ 1.0f, 1.0f};

	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionmatrix;

	void CreateResources();
	void CreateVertexBufferView();
	void CreateIndexBufferView();
	void CreateMaterialResources();
	void CreateTransformMatirxResources();
};

