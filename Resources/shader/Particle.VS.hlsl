#include "Particle.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShederInput {
    float32_t4 position : POSITION0;
    float32_t2 teccoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShederInput input, uint32_t instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix[instanceID].WVP);
    output.texcoord = input.teccoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3)gTransformationMatrix[instanceID].World));
    return output;
}