struct PI_VSINPUT
{
    float4 vpos: POSITION0;
    float4 vcol: COLOR0;

    float4x4 itransform: POSITION1;
    float4 ialbedo: COLOR1;
};

struct PI_GSINPUT
{};

struct PI_PSINPUT
{
    float4 color: COLOR0;

    float4 pos: SV_Position;
};

cbuffer PI_CBUFFER: register(b0)
{
    float4x4 view_proj;
};

PI_PSINPUT VSmain(PI_VSINPUT input)
{
    float4 vpos = input.vpos;
    vpos = mul(input.itransform, vpos);

    PI_PSINPUT output;
    output.color = input.vcol * input.ialbedo;
    output.pos = mul(view_proj, vpos);

    //output.pos = float4(input.vpos, 1.0f);
    return output;
}

float4 PSmain(PI_PSINPUT input): SV_Target
{
    return input.color;
}
