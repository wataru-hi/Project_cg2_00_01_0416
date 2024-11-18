#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "WinApp.h"

class DirectXCommon
{
public:
	void Initialize(WinApp* winApp);

	//デバイスの初期化
	void DeviceInitialize();

	//コマンドの初期化
	void CreateCommand();

	//スワップチェインの生成
	void CreateSwapChain();

	//深度バッファの生成
	void CreateDepthBuffer();

	//各種でスクリプターヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		UINT numDescriptors,
		bool shaderVisible
	);

	void CreateRenderTargetView();

	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	//スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swa>;

	WinApp* winApp_ = nullptr;
};

