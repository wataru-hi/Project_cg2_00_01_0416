#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLightng;
    float32_t shininess;
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
    
struct Camera
{
    float32_t worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectrionaLight> gDirectrionaLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

PixcelShaderOutput main(VertexShaderOutput input)
{   
    PixcelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float32_t3 toEye = normalize(gCamera.worldPosition - input.wotldPosition);
    float32_t3 reflectLight = reflect(gDirectrionaLight.direction, normalize(input.normal));
        
    float RdotE = dot(reflectLight, toEye);
    float specularPow = pow(saturate(RdotE), gMaterial.shininess);
        
    //拡散反射
    float32_t3 diffuse =
    gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;

    // 鏡面反射
    float32_t3 specular =
    gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);

    //拡散反射+鏡面反射
    output.color.rgb = diffuse + specular; // 物体の鏡面反射色。ここでは白にしている materialで設定できたりすると良い

// アルファは今まで通り
    output.color.a = gMaterial.color.a * textureColor.a;
        
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