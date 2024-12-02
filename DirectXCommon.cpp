#include "DirectXCommon.h"

#include <cassert>
#include <dxcapi.h>

#include "Logger.h"
#include "StringUtility.h"
#include <format>

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "Input.h"


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
using namespace Logger;
using namespace StringUtility;

void DirectXCommon::Initialize(WinApp* winApp)
{
	assert(winApp);
	winApp_ = winApp;

	DeviceInitialize(); // デバイスの初期化
	CreateCommand(); // コマンド関連オブジェクトの生成
	CreateSwapChain(); // スワップチェーンの生成
	CreatVariousDescriptorHeaps(); // デスクリプタヒープの生成
	CreateRenderTargetView(); // レンダーターゲットビューの生成
	CreateDepthBuffer(); // 深度バッファの生成
	InitializeDepthView(); // 深度ステンシルビューの初期化
	CreateFance(); // フェンスの生成
	InitializeViewPort(); // ビューポートの初期化
	ScissorPort(); // シザー矩形の初期化
	CreateDXCCompiler(); // DXCコンパイラの生成
	InitializeImGui(); // ImGuiの初期化
}

void DirectXCommon::DeviceInitialize()
{
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugContoroller = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugContoroller)))) {
		//デバッグプレイヤーを有効化する
		debugContoroller->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugContoroller->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	//HRESULはWindows系のエラーコードあり
	//関数が成功したかどうかをSUCCEEDEマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//曽木課の根本な部分でエラーが出た場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	//使用するアダプタ用の変数。最初にnullpterを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> UseAdapter = nullptr;
	//いい順にアダプターを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&UseAdapter))
		!= DXGI_ERROR_NOT_FOUND; ++i)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = UseAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		UseAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプターが見つからなかったので起動できない
	assert(UseAdapter != nullptr);

	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL fealtureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelString[] = { "12.2", "12.1" , "12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(fealtureLevels); ++i)
	{
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(UseAdapter.Get(), fealtureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイス生成
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Log(std::format("Featurrelevel : {} \n", featureLevelString[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Log("Compleate create DeD12Device!!!\n");//初期化完了のログを出す

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> InfoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&InfoQueue))))
	{
		//ヤバイエラー時に止まる
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージID
		D3D12_MESSAGE_ID denyIds[] = {
			//
			//
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑圧するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList; denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		InfoQueue->PushStorageFilter(&filter);
	}
#endif
}

void DirectXCommon::CreateCommand()
{
	HRESULT hr;

	//コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドアロケータを生成する
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateSwapChain()
{
	HRESULT hr;

	swapChainDesc.Width = winApp_->kClientWidth;//画面の幅と高さをクライアントと同じにする
	swapChainDesc.Height = winApp_->kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲット
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら中身を破棄
	//コマンドキュー,ウィンドウハンドル,設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDepthBuffer()
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = winApp_->kClientWidth; // Textureの幅
	resourceDesc.Height = winApp_->kClientHeight; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値) でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる

	// Resourceの生成
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし。
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく
		&depthClearValue, // Clear最適値
		IID_PPV_ARGS(&resource) // 作成するResourceポインタへのポインタ
	);

	assert(SUCCEEDED(hr));

}

void DirectXCommon::CreatVariousDescriptorHeaps()
{
	descripotrSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descripotrSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descripotrSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//ディスクリプターヒープの生成
	rtvDescriptorHeap = createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvDescriptorHeap = createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::createDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)

{
	//デスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateRenderTargetView()
{
	HRESULT hr;

	// SwapChainからResourceを引っ張ってくる
	for (int i = 0; i < 2; i++) {
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainResources[i]));
		// うまく取得できなければ起動しない
		assert(SUCCEEDED(hr));
	}


	// RTVのディスクリプタヒープの先頭を取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// RTVの設定
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2Dテクスチャとして書き込む

	for (int i = 0; i < 2; i++) {
		// ディスクリプタハンドルを計算
		rtvHandle.ptr = rtvStartHandle.ptr + i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// RTVを作成
		device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandle);
	}

#pragma region rtvの設定(旧)
	////RTVのma
	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//２ｄテクスチャとして書き込む
	////ディスクリプタの先頭を取得する
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	////RTVを２つ作るディスクリプタを２つ用意
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
#pragma endregion
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap, descripotrSizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap, descripotrSizeSRV, index);
}

void DirectXCommon::InitializeDepthView()
{
	//depthStencilTextureをウィンドウサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResouce;

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVheapの先頭に
	device->CreateDepthStencilView(depthStencilResouce.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void DirectXCommon::CreateFance()
{
	HRESULT hr;

	//初期値0でFenceを作る
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeViewPort()
{
	viewport.Width = winApp_->kClientWidth;
	viewport.Height = winApp_->kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::ScissorPort()
{
	//基本的にビューポートと同じ矩形が個性されるようにする
	scissorRect.left = 0;
	scissorRect.right = winApp_->kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = winApp_->kClientHeight;
}

void DirectXCommon::CreateDXCCompiler()
{
	HRESULT hr;

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	//現時点でIncudeはしないが、Incudeに対応するための設定を作っておく
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
	hr = dxcUtils->CreateDefaultIncludeHandler(includeHandler.GetAddressOf()); // GetAddressOf()を使用
	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeImGui()
{
	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}
