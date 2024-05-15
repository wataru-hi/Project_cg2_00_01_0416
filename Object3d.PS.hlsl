struct PixelShederOutput
{
    float32_t4 color : SV_Target0;
};

PixelShederOutput main()
{
    PixelShederOutput output;
    output.color = float32_t4(1.0, 1.0, 1.0, 1.0);
    return output;
}