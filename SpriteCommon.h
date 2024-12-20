#pragma once

#include "DirectXCommon.h"

#include <d3d12.h>
#include <wrl.h>

class SpriteCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);

	//共通描画処理
	void CommonDrawingProcess();

	DirectXCommon* GetDxommon() const { return dxCommon_;}
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	DirectXCommon* dxCommon_ = nullptr;

	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインステートの作成
	void CreateGraphicsPipelineState();
};

