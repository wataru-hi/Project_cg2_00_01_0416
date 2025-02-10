#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

#include <dxgidebug.h>

#include <dxcapi.h>
#include "Mymath.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <cmath>

#include <fstream>
#include <sstream>

#include <wrl.h>

#include"DirectXCommon.h"
#include"WinApp.h"

#include "Logger.h"
using namespace Logger;

#include "StringUtility.h"
using namespace StringUtility;

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include <corecrt_math_defines.h>
#include "Input.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

struct Vector2
{
	float x;
	float y;
};

struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};

Vector3 changeVec3(Vector4 a)
{
	Vector3 result;
	result.x = a.x;
	result.y = a.y;
	result.z = a.z;

	return result;
}

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectrionaLight {
	Vector4 color; //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity; //!< 輝度
};

struct MaterialData {
	std::string textureFilepPath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

//LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
//{
//	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
//	{
//		return true;
//	}
//
//	//メッセージに対してゲーム固有の処理を行う
//	switch (msg) {
//		//ウィンドウが破棄された
//	case WM_DESTROY:
//		//OSに対して、アプリの終了を伝える
//		PostQuitMessage(0);
//		return 0;
//	}
//
//	//標準のメッセージ処理を行う
//	return DefWindowProc(hwnd, msg, wparam, lparam);
//}
//
//std::wstring ConvertString(const std::string& str) {//StringUtility用ファイルを作成
//	if (str.empty()) {
//		return std::wstring();
//	}
//
//	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
//	if (sizeNeeded == 0) {
//		return std::wstring();
//	}
//	std::wstring result(sizeNeeded, 0);
//	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
//	return result;
//}

//std::string ConvertString(const std::wstring& str) {
//	if (str.empty()) {
//		return std::string();
//	}
//
//	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
//	if (sizeNeeded == 0) {
//		return std::string();
//	}
//	std::string result(sizeNeeded, 0);
//	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
//	return result;
//}



//void Log(const std::string& message) {
//	OutputDebugStringA(message.c_str());
//}

//DirectXCommonのCompileShaderに移植
//IDxcBlob* CompileShader
//(
//	// CompilerするShederファイルのパス
//	const std::wstring& filePath,
//	// Compilerに使用するProfile
//	const wchar_t* profile,
//	//初期化で生成したものを３つ
//	IDxcUtils* dxcutils,
//	IDxcCompiler3* dxcCompiler,
//	IDxcIncludeHandler* includeHandle
//)
//{
//	//ここからシェーダーをコンパイルする旨をログに出す
//	Log(ConvertString(std::format(L"begin Compiler, path:{}, profile:{}\n", filePath, profile)));
//	//hlslファイルを読む
//	IDxcBlobEncoding* shaderSource = nullptr;
//	HRESULT hr = dxcutils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
//	//読まれなかったら止める
//	assert(SUCCEEDED(hr));
//	//読み込んだファイル内容を設定する
//	DxcBuffer shaderSourceBuffer;
//	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
//	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
//	shaderSourceBuffer.Encoding = DXC_CP_UTF8;// utf-8の文字コードであることを確認
//
//	LPCWSTR arguments[] = {
//		filePath.c_str(),	//コンパイル対象ファイル
//		L"-E", L"main",	//エントリーポイントの指定。基本的にmain以外にはしない
//		L"-T", profile,		//ShaderProfileに指定
//		L"-Zi", L"-Qembed_debug",	//デバッグ用の情報を埋め込む
//		L"-Od", //	最適化を外しておく
//		L"-Zpr", //メモリレイアウトは行優先
//	};
//	//実際にShaderをコンパイルする
//	IDxcResult* shaderResult = nullptr;
//	hr = dxcCompiler->Compile(
//		&shaderSourceBuffer,	//読み込んだファイル
//		arguments, //コンパイルオプション
//		_countof(arguments), //コンパイルオプションの数
//		includeHandle, //includeが含まれた諸々
//		IID_PPV_ARGS(&shaderResult)		///コンパイル結果
//	);
//	//コンパイルエラーではなくdxcが起動できないなどの致命的な状況
//	assert(SUCCEEDED(hr));
//
//	//警告・エラーが出たらログを出して止める
//	IDxcBlobUtf8* shaderError = nullptr;
//	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
//	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
//		Log(shaderError->GetStringPointer());
//		//警告,エラー[
//		assert(false);
//	}
//
//	//コンパイル結果から実行用バイナリ部分を取得
//	IDxcBlob* shaderBlod = nullptr;
//	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlod), nullptr);
//	//成功したログを出す
//	Log(ConvertString(std::format(L"Cmopile Succeded, path:{}, profile:{}\n", filePath, profile)));
//	//実行用のバイナリを返却
//	return shaderBlod;
//
//}

//DirectXCommonのCreateBufferResourceに移植
//static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr< ID3D12Device> device, size_t sizeInBytes)
//{
//	HRESULT hr;
//
//	// 頂点リソース用のヒープの設定
//	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
//	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // uploadHeapを使う
//
//	// 頂点リソースの決定
//	D3D12_RESOURCE_DESC vertexResourceDesc{};
//	// バッファリソース。手くすりゃの場合は別の設定
//	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	vertexResourceDesc.Width = sizeInBytes; // リソースのサイズ。今回はVector4を三頂点分
//	// バッファの場合はこれらを1にする決まり
//	vertexResourceDesc.Height = 1;
//	vertexResourceDesc.DepthOrArraySize = 1;
//	vertexResourceDesc.MipLevels = 1;
//	vertexResourceDesc.SampleDesc.Count = 1;
//	// バッファの場合はこれをする決まり    
//	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//
//	// 実際に頂点リソースを作る
//	Microsoft::WRL::ComPtr<ID3D12Resource> VertexResource; // ComPtrを適用
//	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
//		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
//		IID_PPV_ARGS(&VertexResource));
//	assert(SUCCEEDED(hr));
//
//	return VertexResource; // ComPtrのまま返す
//}

//DirectXCommonクラスに移植
//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(
//	Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
//{
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
//	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
//	descriptorHeapDesc.Type = heapType;
//	descriptorHeapDesc.NumDescriptors = numDescriptors;
//	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
//	assert(SUCCEEDED(hr));
//	return descriptorHeap;
//}

//DirectXCommonのLoadTextureに移植
//DirectX::ScratchImage LoadTexture(const std::string& filePath)
//{
//	//テクスチャファイルを選んでプログラムで扱えるようにする
//	DirectX::ScratchImage image{};
//	std::wstring filePathW = ConvertString(filePath);
//	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
//
//	//ミップマップの生成
//	DirectX::ScratchImage mipImages{};
//	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
//
//	//ミップマップ月のデータを返す
//	return mipImages;
//}

//DirectXCommonのCreateTextureResouceに移植
//Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResouce(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata)
//{
//	//matadataを基にResourceの設定
//	D3D12_RESOURCE_DESC resourceDesc{};
//	resourceDesc.Width = UINT(metadata.width);//Textureの幅
//	resourceDesc.Height = UINT(metadata.height);//Textureの高さ
//	resourceDesc.MipLevels = UINT(metadata.mipLevels);//mipmapの幅
//	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);//　奥行き　or　配列Textureの配列数
//	resourceDesc.Format = metadata.format;//TextureのFormat
//	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。普段は2次元
//
//	//利用するHeapの設定。非常に特殊
//	D3D12_HEAP_PROPERTIES heapProperties{};
//	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;//細かい設定を行う
//	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//WriteBackポリシーでCPUアクセス可能
//	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置
//
//	//Resouceの生成
//	Microsoft::WRL::ComPtr<ID3D12Resource> resouce = nullptr;
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProperties,//Heapの設定
//		D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定
//		&resourceDesc,//Resourceの設定
//		D3D12_RESOURCE_STATE_GENERIC_READ,//作成するResourceのポインタへのポインタ
//		nullptr,//Clearの最高値。使わないのでnullptr
//		IID_PPV_ARGS(&resouce));
//	assert(SUCCEEDED(hr));
//	return resouce;
//}

//DirectXCommonのCreateTextureResouceに移植
//void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages)
//{
//	//Meta情報を取得
//	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
//	//全Mipmapについて
//	for (size_t mipLevel = 0; mipLevel < metaData.mipLevels; ++mipLevel)
//	{
//		//MipMaplevelを指定して各Imageを取得
//		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
//		//Textureに転送
//		HRESULT hr = texture->WriteToSubresource(
//			UINT(mipLevel),
//			nullptr,//全領域へのコピー
//			img->pixels,//元データアクセス
//			UINT(img->rowPitch),//1ラインサイズ
//			UINT(img->slicePitch)//1枚サイズ
//		);
//		assert(SUCCEEDED(hr));
//	}
//}


//DirectXcommonに移植
//static Microsoft::WRL::ComPtr<ID3D12Resource> createDepthTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
//{
//	// 生成するResourceの設定
//	D3D12_RESOURCE_DESC resourceDesc{};
//	resourceDesc.Width = width; // Textureの幅
//	resourceDesc.Height = height; // Textureの高さ
//	resourceDesc.MipLevels = 1; // mipmapの数
//	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
//	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
//	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、1固定。
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
//	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知
//
//	// 利用するHeapの設定
//	D3D12_HEAP_PROPERTIES heapProperties{};
//	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る
//
//	// 深度値のクリア設定
//	D3D12_CLEAR_VALUE depthClearValue{};
//	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f (最大値) でクリア
//	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる
//
//	// Resourceの生成
//	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProperties, // Heapの設定
//		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし。
//		&resourceDesc, // Resourceの設定
//		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく
//		&depthClearValue, // Clear最適値
//		IID_PPV_ARGS(&resource) // 作成するResourceポインタへのポインタ
//	);
//
//	assert(SUCCEEDED(hr));
//
//	return resource;
//
//}

//DirectXCommonの同名の変数に移植
//D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptoprHeap, uint32_t descripotrSize, uint32_t index)
//{
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptoprHeap->GetCPUDescriptorHandleForHeapStart();
//	handleCPU.ptr += (descripotrSize * index);
//	return handleCPU;
//}

//DirectXCommonの同名の変数に移植
//D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptoprHeap, uint32_t descripotrSize, uint32_t index)
//{
//	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptoprHeap->GetGPUDescriptorHandleForHeapStart();
//	handleGPU.ptr += (descripotrSize * index);
//	return handleGPU;
//}

MaterialData LoadmaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifile;
		std::istringstream s(line);
		s >> identifile;

		//identifileに応じた処理
		if (identifile == "map_kd") {
			std::string textureFilename;
			if (!(s >> textureFilename)) { // ファイル名の読み込みが成功したかどうかを確認する
				Log(".mtl ファイルのテクスチャファイル名を読み込めませんでした");
				continue; // この行をスキップするか、エラーを適切に処理する
			}

			materialData.textureFilepPath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {

			VertexData triangle[3];

			// 面は三角形限定．その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				// 頂点の要素へのIndexを「位置/UV/法線」で格納されているので，分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}

				// 要素へのIndexから，実際の要素の値を取得して，頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = { position, texcoord, normal };
				//modelData.vertices.push_back(vertex);

				triangle[faceVertex] = { position, texcoord, normal };
			}
			//頂点を逆人で登録
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadmaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

struct D3dResoucecLackCheker
{
	~D3dResoucecLackCheker()
	{
		//リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
		{
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}

	}
};

// Windowsアプリのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	D3dResoucecLackCheker lackCheker;

	CoInitializeEx(0, COINIT_MULTITHREADED);

	HRESULT hr;

	WinApp* winApp = nullptr;
	winApp = new WinApp();
	winApp->Initialize();

	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//WNDCLASS wc{};
	////ウィンドウプロシージャ
	//wc.lpfnWndProc = WindowProc;
	////ウィンドウクラス名
	//wc.lpszClassName = L"CG2WindowClass";
	////インスタンスハンドル
	//wc.hInstance = GetModuleHandle(nullptr);
	////カーソル
	//wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	////ウィンドウクラスを登録する
	//RegisterClass(&wc);

	////クライアントの領域サイズ
	//const int32_t kClientWidth = 1280;
	//const int32_t kClientHeight = 720;

	////ウィンドウクラスを表す構造体にクライアント領域を入れる
	//RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	////クラインと領域をもとに実際のサイズにwrcを変更してもらう
	//AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//HWND hwnd = CreateWindow(
	//	wc.lpszClassName,		//利用するクラス名
	//	L"CG2",					//タイトルバーの文字（なんでもいい)
	//	WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
	//	CW_USEDEFAULT,			//表示X座標
	//	CW_USEDEFAULT,			//表示Y座標
	//	wrc.right - wrc.left,	//ウィンドウ縦幅
	//	wrc.bottom - wrc.top,	//ウィンドウ横幅
	//	nullptr,				//親ウィンドウハンドル
	//	nullptr,				//メニューウハンドル
	//	wc.hInstance,			//インスタンスハンドル
	//	nullptr					//メニューウハンドル
	//);

	


//DirectXCommonのDeviceInitializeに移植
//#ifdef _DEBUG
//	Microsoft::WRL::ComPtr<ID3D12Debug1> debugContoroller = nullptr;
//	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugContoroller)))) {
//		//デバッグプレイヤーを有効化する
//		debugContoroller->EnableDebugLayer();
//		//さらにGPU側でもチェックを行うようにする
//		debugContoroller->SetEnableGPUBasedValidation(TRUE);
//	}
//#endif // _DEBUG


	////ウィンドウを表示する
	//ShowWindow(hwnd, SW_SHOW);

	//DirectXCommonのDeviceInitializeに移植
	////DXGIファクトリーの生成
	//Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	////HRESULはWindows系のエラーコードあり
	////関数が成功したかどうかをSUCCEEDEマクロで判定できる
	//HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	////曽木課の根本な部分でエラーが出た場合が多いのでassertにしておく
	//assert(SUCCEEDED(hr));

	//DirectXCommonのDeviceInitializeに移植
	////使用するアダプタ用の変数。最初にnullpterを入れておく
	//Microsoft::WRL::ComPtr<IDXGIAdapter4> UseAdapter = nullptr;
	////いい順にアダプターを頼む
	//for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&UseAdapter))
	//	!= DXGI_ERROR_NOT_FOUND; ++i)
	//{
	//	//アダプターの情報を取得する
	//	DXGI_ADAPTER_DESC3 adapterDesc{};
	//	hr = UseAdapter->GetDesc3(&adapterDesc);
	//	assert(SUCCEEDED(hr));
	//	//ソフトウェアアダプタでなければ採用
	//	if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
	//		//採用したアダプタの情報をログに出力
	//		Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
	//		break;
	//	}
	//	UseAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
	//}
	////適切なアダプターが見つからなかったので起動できない
	//assert(UseAdapter != nullptr);


	//DirectXCommonのDeviceInitializeに移植
//	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
//	//昨日レベルとログ出力用の文字列
//	D3D_FEATURE_LEVEL fealtureLevels[] = {
//		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
//	};
//	const char* featureLevelString[] = { "12.2", "12.1" , "12.0" };
//	//高い順に生成できるか試していく
//	for (size_t i = 0; i < _countof(fealtureLevels); ++i)
//	{
//		//採用したアダプターでデバイスを生成
//		hr = D3D12CreateDevice(UseAdapter.Get(), fealtureLevels[i], IID_PPV_ARGS(&device));
//		//指定した機能レベルでデバイス生成
//		if (SUCCEEDED(hr)) {
//			//生成できたのでログ出力を行ってループを抜ける
//			Log(std::format("Featurrelevel : {} \n", featureLevelString[i]));
//			break;
//		}
//	}
//	//デバイスの生成がうまくいかなかったので起動できない
//	assert(device != nullptr);
//	Log("Compleate create DeD12Device!!!\n");//初期化完了のログを出す
//
//#ifdef _DEBUG
//	Microsoft::WRL::ComPtr<ID3D12InfoQueue> InfoQueue = nullptr;
//	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&InfoQueue))))
//	{
//		//ヤバイエラー時に止まる
//		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
//		//エラー時に止まる
//		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
//		//警告時に止まる
//		InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
//
//		//抑制するメッセージID
//		D3D12_MESSAGE_ID denyIds[] = {
//			//
//			//
//			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
//		};
//		//抑圧するレベル
//		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
//		D3D12_INFO_QUEUE_FILTER filter{};
//		filter.DenyList.NumIDs = _countof(denyIds);
//		filter.DenyList.pIDList; denyIds;
//		filter.DenyList.NumSeverities = _countof(severities);
//		filter.DenyList.pSeverityList = severities;
//		//指定したメッセージの表示を抑制する
//		InfoQueue->PushStorageFilter(&filter);
//	}
//#endif

	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	////コマンドキューを生成する////DirectXCommonクラスに移植済
	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	////コマンドキューの生成がうまくいかなかったので起動できない
	//assert(SUCCEEDED(hr));

	////コマンドアロケータを生成する////DirectXCommonクラスに移植済
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	////コマンドキューの生成がうまくいかなかったので起動できない
	//assert(SUCCEEDED(hr));

	////コマンドリストを生成する////DirectXCommonクラスに移植済
	//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> dxCommon->GetCommandList() = nullptr;
	//hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&dxCommon->GetCommandList()));
	////コマンドキューの生成がうまくいかなかったので起動できない
	//assert(SUCCEEDED(hr));

	////スワップチェーンを生成する////DirectXCommonクラスに移植済
	//Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//swapChainDesc.Width = winApp->kClientWidth;//画面の幅と高さをクライアントと同じにする
	//swapChainDesc.Height = winApp->kClientHeight;
	//swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	//swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲット
	//swapChainDesc.BufferCount = 2;//ダブルバッファ
	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに移したら中身を破棄
	////コマンドキュー,ウィンドウハンドル,設定を渡して生成する
	//hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	//assert(SUCCEEDED(hr));

	//DirectXCommonのInitializeDepthViewに移植
	////=======================
	////depthStencilTextureをウィンドウサイズで作成
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResouce = createDepthTextureResource(device, winApp->kClientWidth, winApp->kClientHeight);
	////=======================

	//DirectXCommonのCreatVariousDescriptorHeapsに移植
	//ディスクリプターヒープの生成
	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = createDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = createDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = createDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);*/

	//DirectXCommonに移植
	////SwapChaonから Resouceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	//hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	////うまく取得できなければ起動しない
	//assert(SUCCEEDED(hr));
	//hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	//assert(SUCCEEDED(hr));

	////RTVのma
	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//２ｄテクスチャとして書き込む
	////ディスクリプタの先頭を取得する
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	////RTVを２つ作るディスクリプタを２つ用意
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	////まず1つ目を作る。１つ目の最初のところに作る。作る場所を指定してあげる必要がある
	//rtvHandles[0] = rtvStartHandle;
	//device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	////2つ目のディスクリプタハンドルを得る
	//rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	////２つ目を作る
	//device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

	//DirectXCommonのCreateFenceに移植
	////初期値0でFenceを作る
	//Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//uint64_t fenceValue = 0;
	//hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	//assert(SUCCEEDED(hr));

	//FenceのSignalを待つためのイベントを作成する
	/*HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);*/

	//DirectXCommonのDXCCompilerに移植
	////dxcCompilerを初期化
	//Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;  // ComPtrで宣言
	//Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;  // ComPtrで宣言

	//hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	//assert(SUCCEEDED(hr));
	//hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	//assert(SUCCEEDED(hr));
	
	//DirectXCommonのCreateDepthViewに移植
	////DSVの設定
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	////DSVheapの先頭に
	//device->CreateDepthStencilView(depthStencilResouce.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みをする
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual,つまり近ければ描画がされます
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//DirectXCommonのCreateDXCCompilerに移植
	////現時点でIncudeはしないが、Incudeに対応するための設定を作っておく
	//Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
	//hr = dxcUtils->CreateDefaultIncludeHandler(includeHandler.GetAddressOf()); // GetAddressOf()を使用
	//assert(SUCCEEDED(hr));

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offiserを自動計算

	//RootSignatureの生成
	D3D12_ROOT_SIGNATURE_DESC descripionRootSignature{};
	descripionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	//RootParameter作成、複数設定ができるまで配列。今回は一つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを作る
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号θとバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを作る
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexDhaderを使う
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号θとバインド
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriporTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身を配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableの中身を配列を指定
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixcelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号１
	descripionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descripionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイタリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//０～１の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMinmapを使う
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixcelShaderで使う
	descripionRootSignature.pStaticSamplers = staticSamplers;
	descripionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlod = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlod = nullptr;
	hr = D3D12SerializeRootSignature(&descripionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlod, &errorBlod);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlod->GetBufferPointer()));
		assert(false);
	}
	//バイナリを先に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0,
		signatureBlod->GetBufferPointer(), signatureBlod->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_ELEMENT_DESC inputElementDesc[3] = {};
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDesc[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDesc[1].SemanticName = "TEXCOORD";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDesc[2].SemanticName = "NORMAL";
	inputElementDesc[2].SemanticIndex = 0;
	inputElementDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = _countof(inputElementDesc);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//ResiterzerStartの設定
	D3D12_RASTERIZER_DESC rasterzerDesc{};
	//裏面(時計回り)を表示しない
	rasterzerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterzerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Resources/shader/Object3d.VS.hlsl",L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Resources/shader/Object3d.PS.hlsl",L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//rootsignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//intputlatout 
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//vertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;//blendeState
	graphicsPipelineStateDesc.RasterizerState = rasterzerDesc;
	//書き込むRTV情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するとポロ時(形状)のタイプ
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//int vertexCount = 1536;

	//ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * vertexCount);
	//ID3D12Resource* indexResource = CreateBufferResource(device, sizeof(uint32_t) * vertexCount);

	////マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	//ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Material));
	////マテリアルにデータを書き込む
	//Material* materialDate = nullptr;
	////書き込むためのアドレスを取得
	//materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	////今回は赤を書き込んでみる
	//materialDate->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialDate->enableLighting = true;

	////頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	////リソースの先頭のアドレスから使う
	//vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	////使用するリソースサイズは頂点3つ分のサイズ
	//vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	////1頂点当たりのサイズ
	//vertexBufferView.StrideInBytes = sizeof(VertexData);
	//
	////頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW indexBufferView{};
	////リソースの先頭のアドレスから使う
	//indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	////使用するリソースサイズは頂点3つ分のサイズ
	//indexBufferView.SizeInBytes = sizeof(uint32_t) * vertexCount;
	////1頂点当たりのサイズ
	//indexBufferView.StrideInBytes = DXGI_FORMAT_R32_UINT;

	////頂点リソースにデータを書き込む
	//VertexData* vertexData = nullptr;
	////書き込むためのアドレスを取得
	//vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//
	////頂点リソースにデータを書き込む
	//uint32_t* indexData = nullptr;
	////書き込むためのアドレスを取得
	//indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	//// 緯度方向の分割数
	//const int kSubdivision = 16;
	//// 経度分割1つ分の角度
	//const float kPhaiEvery = M_PI * 2.0f / float(kSubdivision);
	//// 緯度分割1つ分の角度
	//const float kShitaEvery = M_PI / float(kSubdivision);
	//// 緯度の方向に分割
	//for (int latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float shita = -M_PI / 2.0f + kShitaEvery * latIndex;//θ

	//	// 経度の方向に分割しながら線を描く
	//	for (int lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
	//		float phai = lonIndex * kPhaiEvery;//φ

	//		/*float u = float(lonIndex / kSubdivision);
	//		float v = 1.0f - float(latIndex / kSubdivision);*/

	//		float u = float(lonIndex) / float(kSubdivision);
	//		float v = 1.0f - float(latIndex) / float(kSubdivision);

	//		VertexData vLB = {
	//			{
	//				cos(shita) * cos(phai),
	//				sin(shita),
	//				cos(shita) * sin(phai),
	//				1.0f				
	//			},
	//			{
	//				u,
	//				v
	//			}
	//		};
	//		vLB.normal = changeVec3(vLB.position);

	//		VertexData vLT = {
	//			{
	//				cos(shita + kShitaEvery) * cos(phai),
	//				sin(shita + kShitaEvery),
	//				cos(shita + kShitaEvery) * sin(phai),
	//				1.0f				
	//			},
	//			{
	//				u,
	//				v - 1.0f / float(kSubdivision)
	//			}
	//		};
	//		vLT.normal = changeVec3(vLT.position);

	//		VertexData vRB = {
	//			{
	//				cos(shita) * cos(phai + kPhaiEvery),
	//				sin(shita),
	//				cos(shita) * sin(phai + kPhaiEvery),
	//				1.0f
	//			},
	//			{
	//				u + 1.0f / float(kSubdivision) ,
	//				v
	//			}
	//		};
	//		vRB.normal = changeVec3(vRB.position);

	//		VertexData vRT = {
	//			{
	//				cos(shita + kShitaEvery) * cos(phai + kPhaiEvery),
	//				sin(shita + kShitaEvery),
	//				cos(shita + kShitaEvery) * sin(phai + kPhaiEvery),
	//				1.0f				
	//			},
	//			{
	//				u + 1.0f / float(kSubdivision),
	//				v - 1.0f / float(kSubdivision)
	//			}
	//		};
	//		vRT.normal = changeVec3(vRT.position);

	//		// 原点aにデータを入力する
	//		vertexData[start] = vRT;

	//		// b の頂点データを計算
	//		vertexData[start + 1] = vRB;
	//		vertexData[start + 3] = vRB;

	//		// c の頂点データを計算
	//		vertexData[start + 2] = vLT;
	//		vertexData[start + 4] = vLT;

	//		// d の頂点データを計算
	//		vertexData[start + 5] = vLB;

	//		indexData[0] = start + 0; indexData[1] = start + 1; indexData[2] = start + 2;
	//		indexData[3] = start + 1; indexData[4] = start + 2; indexData[5] = start + 5;
	//	}
	//}

	// モデルを読み込み
	ModelData modelData = LoadObjFile("Resources/06_02", "axis.obj");

	// 頂点リソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの仮想のアドレスから使う
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData); // 頂点あたりのサイズ

	//マテリアル用のリソースを作る。今回はColor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialDate = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	//今回は赤を書き込んでみる
	materialDate->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDate->enableLighting = true;

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)); // 書き込むためのアドレスを取得
	uint32_t modelSize = static_cast<uint32_t>(sizeof(VertexData) * modelData.vertices.size());
	std::memcpy(vertexData, modelData.vertices.data(), modelSize); // 頂点データをリソースにコピー
	//vertexResource->Unmap(0, nullptr);


	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
	Material* materialDateSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDateSprite));
	materialDateSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDateSprite->enableLighting = false;

	//頂点バッファビューを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースサイズは頂点3つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//1頂点当たりのサイズ
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//頂点リソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));

	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 4; indexDataSprite[5] = 2;

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

	vertexDataSprite[0] = hidariSita;
	vertexDataSprite[1] = hidariue;
	vertexDataSprite[2] = migiSita;

	vertexDataSprite[3] = hidariue;
	vertexDataSprite[4] = migiUe;
	vertexDataSprite[5] = migiSita;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectrionaLight));

	DirectrionaLight* directrionaLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directrionaLightData));

	directrionaLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directrionaLightData->direction = { 0.0f, -1.0f, 0.0f };
	directrionaLightData->intensity = 1.0f;

	////DirectXCommonのCreatVariousDescriptorHeapsに移植
	//const uint32_t descripotrSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//const uint32_t descripotrSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//const uint32_t descripotrSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	dxCommon->UploadTextureData(textureResource.Get(), mipImages);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescripotrSizeSRV(), 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescripotrSizeSRV(), 2);

	//Textureを読んで転送する
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture("Resources/monsterBall.png");
	const DirectX::TexMetadata metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	dxCommon->UploadTextureData(textureResource2.Get(), mipImages2);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 =  dxCommon->GetCPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescripotrSizeSRV(), 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 =  dxCommon->GetGPUDescriptorHandle(dxCommon->GetSrvDescriptorHeap(), dxCommon->GetDescripotrSizeSRV(), 2);

	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	//WVP用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	//Sprite用のTransformMatrix用のリソースを作る。
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transfromationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transfromationMatrixDataSprite));
	//単位行列を書き込んでおく
	transfromationMatrixDataSprite->WVP = MakeIdentity4x4();
	transfromationMatrixDataSprite->World = MakeIdentity4x4();

	input->Update();

	//DirectXCommonのCreateViewportに移植
	////ビューポート
	//D3D12_VIEWPORT viewport{};
	////クライアント領域のサイズと一緒にして画面全体に表示
	//viewport.Width = winApp->kClientWidth;
	//viewport.Height = winApp->kClientHeight;
	//viewport.TopLeftX = 0;
	//viewport.TopLeftY = 0;
	//viewport.MinDepth = 0.0f;
	//viewport.MaxDepth = 1.0f;

	//DirectXCommonのCreateScissorに移植
	////シザー矩形
	//D3D12_RECT scissorRect{};
	////基本的にビューポートと同じ矩形が個性されるようにする
	//scissorRect.left = 0;
	//scissorRect.right = winApp->kClientWidth;
	//scissorRect.top = 0;
	//scissorRect.bottom = winApp->kClientHeight;

	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 10.0f} };
	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};

	materialDate->uvTransform = MakeIdentity4x4();
	materialDateSprite->uvTransform = MakeIdentity4x4();

	float TransformUi[3][3];

	bool useMonsterBall = true;

	float LightColor[4];
	float LightDirection[3];
	float LightIntensity;

	directrionaLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directrionaLightData->direction = { 0.0f, -1.0f, 0.0f };
	directrionaLightData->intensity = 1.0f;

	//DirectXCommonのCreateIMGUIに移植
	////ImGuiの初期化
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	//ImGui_ImplWin32_Init(winApp->GetHwnd());
	//ImGui_ImplDX12_Init(device.Get(),
	//	swapChainDesc.BufferCount,
	//	rtvDesc.Format,
	//	srvDescriptorHeap.Get(),
	//	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	//	srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//ウィンドウの×ボタンが押されるまでループ
	while (true)//ゲームループ
	{
		if (winApp->ProcessMassage())
		{
			break;
		}

		//ゲームの処理

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		//transform.rotate.y += 0.03f;

		TransformUi[0][0] = transformSprite.scale.x;
		TransformUi[0][1] = transformSprite.scale.y;
		TransformUi[0][2] = transformSprite.scale.z;

		TransformUi[1][0] = transformSprite.rotate.x;
		TransformUi[1][1] = transformSprite.rotate.y;
		TransformUi[1][2] = transformSprite.rotate.z;

		TransformUi[2][0] = transformSprite.translate.x;
		TransformUi[2][1] = transformSprite.translate.y;
		TransformUi[2][2] = transformSprite.translate.z;

		LightColor[0] = directrionaLightData->color.x;
		LightColor[1] = directrionaLightData->color.y;
		LightColor[2] = directrionaLightData->color.z;
		LightColor[3] = directrionaLightData->color.w;

		LightDirection[0] = directrionaLightData->direction.x;
		LightDirection[1] = directrionaLightData->direction.y;
		LightDirection[2] = directrionaLightData->direction.z;

		LightIntensity = directrionaLightData->intensity;

		// X、Y、Zの位置をスライダーで変更
		ImGui::SliderFloat("X Position", &transform.rotate.x, -10.0f, 10.0f);
		ImGui::SliderFloat("Y Position", &transform.rotate.y, -10.0f, 10.0f);
		ImGui::DragFloat("Z Position", &transform.rotate.z, 0.1f, 1.0f);

		ImGui::DragFloat3("spriteS", TransformUi[0], 0.1f, 1.0f);
		ImGui::DragFloat3("spriteR", TransformUi[1], 0.1f, 1.0f);
		ImGui::DragFloat3("spriteT", TransformUi[2], 0.1f, 1.0f);

		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		ImGui::DragFloat4("LightColor", LightColor, 0.01f, 1.0f);
		ImGui::DragFloat3("LightDirection", LightDirection, 0.01f, 1.0f);
		ImGui::DragFloat("LightIntensity", &LightIntensity, 0.01f, 1.0f);

		ImGui::DragFloat2("uvTransform", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("uvScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("uvRotate", &uvTransformSprite.rotate.z);


		transformSprite.scale.x = TransformUi[0][0];
		transformSprite.scale.y = TransformUi[0][1];
		transformSprite.scale.z = TransformUi[0][2];

		transformSprite.rotate.x = TransformUi[1][0];
		transformSprite.rotate.y = TransformUi[1][1];
		transformSprite.rotate.z = TransformUi[1][2];

		transformSprite.translate.x = TransformUi[2][0];
		transformSprite.translate.y = TransformUi[2][1];
		transformSprite.translate.z = TransformUi[2][2];

		directrionaLightData->color.x = LightColor[0];
		directrionaLightData->color.y = LightColor[1];
		directrionaLightData->color.z = LightColor[2];
		directrionaLightData->color.w = LightColor[3];

		directrionaLightData->direction.x = LightDirection[0];
		directrionaLightData->direction.y = LightDirection[1];
		directrionaLightData->direction.z = LightDirection[2];

		directrionaLightData->intensity = LightIntensity;


		Matrix4x4 worldMatrix = MakeAfineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAfineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = makePerspectiveMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionmatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionmatrix;
		wvpData->World = worldMatrix;

		Matrix4x4 worldMatrixSprite = MakeAfineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = makeOrthogphicMatrix(0.0f, 0.0f, float(winApp->kClientWidth), float(winApp->kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionmatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transfromationMatrixDataSprite->WVP = worldViewProjectionmatrixSprite;
		transfromationMatrixDataSprite->World = worldMatrixSprite;

		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRoatateZMatix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDateSprite->uvTransform = uvTransformMatrix;

		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		dxCommon->PreDraw();

		
		//RootSignatureを設定。PS0に設定しているけど別途設定が必要
		dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());//PS0を設定
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);//VBVを設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//形状を設定。PS0に設定しているものとはまた別。同じものを設定すると考えておけば良い
		dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//マテリアルｃBufferの設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

		//wvp用のCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

		//DirectXCommonのPreDrawに移植
		////描画用のDescriptorHeapの設定
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptoHeaps[] = { srvDescriptorHeap };
		//dxCommon->GetCommandList()->SetDescriptorHeaps(1, descriptoHeaps->GetAddressOf());

		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

		DirectX::ScratchImage mipImage2 = dxCommon->LoadTexture(modelData.material.textureFilepPath);

		//dxCommon->GetCommandList()->DrawInstanced(vertexCount, 1, 0, 0);
		dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);//VBVを設定
		dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//VBVを設定0400

		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);//0501

		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());//0400
		dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

		//実際のdxCommon->GetCommandList()のImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		dxCommon->PostDraw();


	}


	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello.DirectX!\n");

	//解放処理
	delete input;
	delete dxCommon;
	winApp->Finalize();

	delete winApp;

	//CloseHandle(fenceEvent);


	return 0;
}