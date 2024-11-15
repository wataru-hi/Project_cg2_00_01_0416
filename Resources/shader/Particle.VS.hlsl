#include "Particle.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
StructuredBuffer<TransformationMatrix> gTransformationMatrix : register(t0);

struct VertexShederInput {
    float32_t4 position : POSITION0;
    float32_t2 teccoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShederInput input, uint32_t instanceId : SV_InstanceId)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix[instanceId].WVP);
    output.texcoord = input.teccoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix[instanceId].World));
    return output;
}