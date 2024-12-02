#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>

#include "WinApp.h"


class DirectXCommon
{
public:
	void Initialize(WinApp* winApp); // DirectXの初期化処理全体

	//各種でスクリプターヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDescriptors,
		bool shaderVisible
	); // デスクリプタヒープを生成する

	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index); // CPUがアクセス可能なデスクリプタハンドルを取得

	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index); // GPUがアクセス可能なデスクリプタハンドルを取得

	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index); // SRVのCPUデスクリプタハンドルを取得

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index); // SRVのGPUデスクリプタハンドルを取得


private:
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	//コマンドキューを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//コマンドアロケータを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	//スワップチェーンを生成する
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;

	//各種デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	uint32_t descripotrSizeSRV;
	uint32_t descripotrSizeRTV;
	uint32_t descripotrSizeDSV;

	//スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	// ディスクリプタハンドルを計算
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};


	//Fence
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

	//ビューポート矩形
	D3D12_VIEWPORT viewport{};

	//シザー矩形
	D3D12_RECT scissorRect{};

	//dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;

	//デバイスの初期化
	void DeviceInitialize(); // D3D12デバイスを初期化する

	//コマンドの初期化
	void CreateCommand(); // コマンドキュー、アロケータ、リストを生成する

	//スワップチェインの生成
	void CreateSwapChain(); // スワップチェーンを生成する

	//深度バッファの生成
	void CreateDepthBuffer(); // 深度バッファを生成する

	void CreatVariousDescriptorHeaps(); // 各種デスクリプタヒープ (RTV, DSV, SRVなど) を作成する

	void CreateRenderTargetView(); // レンダーターゲットビューを作成する

	//深度ステンシルビューの初期化
	void InitializeDepthView(); // 深度ステンシルビューを初期化する

	//フェンスの生成
	void CreateFance(); // フェンスを生成する (同期用)

	//ビューポート矩形の初期化
	void InitializeViewPort(); // ビューポートを設定する

	//シザリング矩形
	void ScissorPort(); // シザー矩形を設定する

	//DXCCompilerの生成
	void CreateDXCCompiler(); // DXCコンパイラを生成する

	//ImGuiの初期化
	void InitializeImGui(); // ImGuiを初期化する
};