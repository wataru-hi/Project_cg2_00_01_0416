#include "Sprite.h"
#include "SpriteCommon.h"

#include "DirectXCommon.h"
#include "WinApp.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon)
{
	spriteCommon_ = spriteCommon;
	dxCommon_ = dxCommon;

	CreateResources();

	CreateVertexBufferView();
	CreateIndexBufferView();
	CreateMaterialResources();
	CreateTransformMatirxResources();

	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
}

void Sprite::Update(WinApp* winApp)
{
	winApp_ = winApp;

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDate));

	transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	worldMatrix = MakeAfineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = makeOrthogphicMatrix(0.0f, 0.0f, float(winApp_->kClientWidth), float(winApp_->kClientHeight), 0.1f, 100.0f);
	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle)
{
	ID3D12GraphicsCommandList* commandList = spriteCommon_->GetDxommon()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferVier);
	commandList->IASetIndexBuffer(&indexBufferVier);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0,0);
}

void Sprite::CreateResources()
{
	
	indexDate[0] = 0; indexDate[1] = 1; indexDate[2] = 2;
	indexDate[3] = 1; indexDate[4] = 4; indexDate[5] = 2;

	VertexData hidariSita;
	hidariSita.position = { 0.0f, 360.0f, 0.0f, 1.0f };
	hidariSita.texcoord = { 0.0f, 1.0f };
	hidariSita.normal = { 0.0f, 0.0f, -1.0f };

	VertexData hidariue;
	hidariue.position = { 0.0f, 0.0f, 0.0f, 1.0f };
	hidariue.texcoord = { 0.0f, 0.0f };
	hidariue.normal = { 0.0f, 0.0f, -1.0f };

	VertexData migiUe;
	migiUe.position = { 640.0f, 0.0f, 0.0f, 1.0f };
	migiUe.texcoord = { 1.0f, 0.0f };
	migiUe.normal = { 0.0f, 0.0f, -1.0f };

	VertexData migiSita;
	migiSita.position = { 640.0f, 360.0f, 0.0f, 1.0f };
	migiSita.texcoord = { 1.0f, 1.0f };
	migiSita.normal = { 0.0f, 0.0f, -1.0f };

	vertexDate[0] = hidariSita;
	vertexDate[1] = hidariue;
	vertexDate[2] = migiSita;

	vertexDate[3] = hidariue;
	vertexDate[4] = migiUe;
	vertexDate[5] = migiSita;

	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDate));
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate));
}

void Sprite::CreateVertexBufferView()
{
	vertexBufferVier.BufferLocation = vertexResource->GetGPUVirtualAddress();

	vertexBufferVier.SizeInBytes = sizeof(VertexData) * 6; // vertexCountは頂点数
	vertexBufferVier.StrideInBytes = sizeof(VertexData);
}

void Sprite::CreateIndexBufferView()
{
	indexBufferVier.BufferLocation = indexResource->GetGPUVirtualAddress();

	indexBufferVier.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferVier.Format = DXGI_FORMAT_R32_UINT;
}

void Sprite::CreateMaterialResources()
{
	materialResource = dxCommon_->CreateBufferResource(sizeof(Material));

	materialDate->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDate->enableLighting = false;
	materialDate->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformMatirxResources()
{
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
}
