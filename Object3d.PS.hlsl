struct Material
{
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);
struct PixcelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixcelShaderOutput main()
{
    PixcelShaderOutput output;
    output.color = float32_t4(1,1,1,1);
    return output;
}