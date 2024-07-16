#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLightng;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
    
struct PixcelShaderOutput
{
    float32_t4 color : SV_Target0;
};

struct DirectrionaLight
{
    float32_t4 color; //!< ライトの色
    float32_t3 direction; //!< ライトの向き
    float intensity;
};

ConstantBuffer<DirectrionaLight> gDirectrionaLight : register(b1);

PixcelShaderOutput main(VertexShaderOutput input)
{   
    PixcelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = gMaterial.color * textureColor;
    return output;
}