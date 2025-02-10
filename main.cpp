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

	
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

#pragma region DirectXCommonクラスに移植(SetDepthStencilState)
	////DepthStencilStateの設定
	//D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	////Depthの機能を有効化する
	//depthStencilDesc.DepthEnable = true;
	////書き込みをする
	//depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	////比較関数はLessEqual,つまり近ければ描画がされます
	//depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

#pragma region(SetDescriptorRange)
	//D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	//descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	//descriptorRange[0].NumDescriptors = 1;//数は1つ
	//descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	//descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offiserを自動計算
#pragma endregion


#pragma region(CreateRootSignature)
	////RootSignatureの生成
	//D3D12_ROOT_SIGNATURE_DESC descripionRootSignature{};
	//descripionRootSignature.Flags =
	//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	////RootParameter作成、複数設定ができるまで配列。今回は一つだけなので長さ１の配列
	//D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを作る
	//rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderを使う
	//rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号θとバインド
	//rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを作る
	//rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexDhaderを使う
	//rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号θとバインド
	//rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriporTableを使う
	//rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	//rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身を配列を指定
	//rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableの中身を配列を指定
	//rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	//rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixcelShaderで使う
	//rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号１
	//descripionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	//descripionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

	//D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	//staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイタリニアフィルタ
	//staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//０～１の範囲外をリピート
	//staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	//staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	//staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	//staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMinmapを使う
	//staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
	//staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixcelShaderで使う
	//descripionRootSignature.pStaticSamplers = staticSamplers;
	//descripionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	////シリアライズしてバイナリにする
	//Microsoft::WRL::ComPtr<ID3DBlob> signatureBlod = nullptr;
	//Microsoft::WRL::ComPtr<ID3DBlob> errorBlod = nullptr;
	//hr = D3D12SerializeRootSignature(&descripionRootSignature,
	//	D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlod, &errorBlod);
	//if (FAILED(hr)) {
	//	Log(reinterpret_cast<char*>(errorBlod->GetBufferPointer()));
	//	assert(false);
	//}
	////バイナリを先に生成
	//Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//hr = dxCommon->GetDevice()->CreateRootSignature(0,
	//	signatureBlod->GetBufferPointer(), signatureBlod->GetBufferSize(),
	//	IID_PPV_ARGS(&rootSignature));
	//assert(SUCCEEDED(hr));
#pragma endregion

#pragma region(SetInputElementDesc)
	//D3D12_INPUT_ELEMENT_DESC inputElementDesc[3] = {};
	//inputElementDesc[0].SemanticName = "POSITION";
	//inputElementDesc[0].SemanticIndex = 0;
	//inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//inputElementDesc[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//inputElementDesc[1].SemanticName = "TEXCOORD";
	//inputElementDesc[1].SemanticIndex = 0;
	//inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	//inputElementDesc[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//inputElementDesc[2].SemanticName = "NORMAL";
	//inputElementDesc[2].SemanticIndex = 0;
	//inputElementDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//inputElementDesc[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	//inputLayoutDesc.pInputElementDescs = inputElementDesc;
	//inputLayoutDesc.NumElements = _countof(inputElementDesc);
#pragma endregion

#pragma region(SetBlendState)
	////BlendStateの設定
	//D3D12_BLEND_DESC blendDesc{};
	////すべての色要素を書き込む
	//blendDesc.RenderTarget[0].RenderTargetWriteMask =
	//	D3D12_COLOR_WRITE_ENABLE_ALL;
#pragma endregion

#pragma region(SetRasterizerState)
	////ResiterzerStartの設定
	//D3D12_RASTERIZER_DESC rasterzerDesc{};
	////裏面(時計回り)を表示しない
	//rasterzerDesc.CullMode = D3D12_CULL_MODE_NONE;
	////三角形の中を塗りつぶす
	//rasterzerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion


#pragma region(SetCompileShader)
	////shaderをコンパイルする
	//Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Resources/shader/Object3d.VS.hlsl",L"vs_6_0");
	//assert(vertexShaderBlob != nullptr);

	//Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Resources/shader/Object3d.PS.hlsl",L"ps_6_0");
	//assert(pixelShaderBlob != nullptr);

	dxCommon->SetCompileVertexShader(L"Resources/shader/Object3d.VS.hlsl",L"vs_6_0");
	dxCommon->SetCompilePixcelShader(L"Resources/shader/Object3d.PS.hlsl",L"vs_6_0");
#pragma endregion

#pragma region(CreateGraphicsPipelineState)
	//D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	//graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//rootsignature
	//graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//intputlatout 
	//graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	//vertexShaderBlob->GetBufferSize() };//vertexShader
	//graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	//pixelShaderBlob->GetBufferSize() };
	//graphicsPipelineStateDesc.BlendState = blendDesc;//blendeState
	//graphicsPipelineStateDesc.RasterizerState = rasterzerDesc;
	////書き込むRTV情報
	//graphicsPipelineStateDesc.NumRenderTargets = 1;
	//graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	////利用するとポロ時(形状)のタイプ
	//graphicsPipelineStateDesc.PrimitiveTopologyType =
	//	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//// DepthStencilの設定
	//graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	//graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	////どのように画面に色を打ち込むかの設定
	//graphicsPipelineStateDesc.SampleDesc.Count = 1;
	//graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	////実際に生成
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	//hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
	//	IID_PPV_ARGS(&graphicsPipelineState));
	//assert(SUCCEEDED(hr));
#pragma endregion

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


	return 0;
}