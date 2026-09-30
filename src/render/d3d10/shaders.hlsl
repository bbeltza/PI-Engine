struct PI_VSINPUT
{
    float3 vpos: XYZ;
    float3 vcol: RGB;

    float4x4 itransform: TRANSFORM;
    float4 ialbedo: ALBEDO;
};

struct PI_GSINPUT
{};

struct PI_PSINPUT
{
    float4 color: COLOR;

    float4 pos: SV_Position;
};

cbuffer PI_CBUFFER: register(b0)
{
    float4x4 view_proj;
};

PI_PSINPUT VSmain(PI_VSINPUT input)
{
    float4 vpos = float4(input.vpos, 1.0f);
    vpos = mul(input.itransform, vpos);

    PI_PSINPUT output;
    output.color = float4(input.vcol, 1.0f) * input.ialbedo;
    output.pos = mul(view_proj, vpos);
    //output.pos = float4(input.vpos, 1.0f);
    return output;
}

float4 PSmain(PI_PSINPUT input): SV_Target
{
    return input.color;
}
