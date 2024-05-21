struct VertexShaderOutput {
    float32_t4 position : SV_POSITION;
};

struct VertexShederInput {
    float32_t4 position : POSITION0;
};

VertexShaderOutput main(VertexShederInput input)
{
    VertexShaderOutput output;
    output.position = input.position;
    return output;
}