#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLightng;
    float32_t 
};

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

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectrionaLight> gDirectrionaLight : register(b1);

PixcelShaderOutput main(VertexShaderOutput input)
{   
    PixcelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float32_t3 toEye = normalize(gCamera.worldPosition - input.wotldPosition);
    
    if (gMaterial.enableLightng != 0)//Litingする場合
    {
        float NdotL = dot(normalize(input.normal), -gDirectrionaLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color = gMaterial.color * textureColor * gDirectrionaLight.color * cos * gDirectrionaLight.intensity;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    
    return output;
}