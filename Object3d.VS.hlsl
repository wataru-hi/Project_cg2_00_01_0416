#include "object3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShederInput {
    float32_t4 position : POSITION0;
    float32_t2 teccoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShederInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.teccoord;
    return output;
}