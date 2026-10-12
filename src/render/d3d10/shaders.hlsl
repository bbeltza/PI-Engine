/*
    Preprocessor definitions
    - Analogous to the PI_WORLDEFFECT_ flags:
        #define WORLDEFFECT_LIGHTING
            Enable lighting.
        #define WORLDEFFECT_SPECULAR
            Enable specular shading (will likely be in blinn-phong). WORLDEFFECT_LIGHTING must be defined.
 */

#include "shader_properties.h"

#define WORLDEFFECT_LIGHTING

#ifndef WORLDEFFECT_LIGHTING
    #ifdef WORLDEFFECT_SPECULAR
        #error "Can't compile with no WORLDEFFECT_LIGHTING but WORLDEFFECT_SPECULAR defined."
    #endif
#else
    struct PI_POINTLIGHT {
        float4 position; /* "w" component ignored. Position of the light */
        float4 color; /* Color and intensity (in the "a" channel) of the light */
        float range;
        float falloff;

        float2 _padding;
    };
    struct PI_DIRLIGHT {
        float4 direction;
        float4 color;
    };
    
        /* 80 bytes */
    struct PI_LIGHTGROUP {
        PI_DIRLIGHT dirlight;
        PI_POINTLIGHT ptlight;
    };

    // PI_SPOTLIGHT maybe to be defined later... I don't think I want it now

    cbuffer PI_LIGHTS: register( b1 ) {
        uint NUM_DIRLIGHTS;
        uint NUM_POINTLIGHTS;
        uint2 _padding;
        PI_LIGHTGROUP LIGHTS[MAX_COMBINED_LIGHTS];
    };
#endif

#ifdef WORLDEFFECT_SPECULAR
    cbuffer PI_SPECULARSETTING: register( b2 ) {
        float4 color;
        float exponent;
    };
#endif

struct PI_VSINPUT
{
    float4 vpos: POSITION0;
#ifdef WORLDEFFECT_LIGHTING
    float4 vnormal: COLOR0;
#else
    float4 vcol: COLOR0;
#endif

    float4x4 itransform: POSITION1;
    float4 ialbedo: COLOR1;
};


struct PI_GSINPUT
{};

struct PI_PSINPUT
{
    float4 color: COLOR0;
#ifdef WORLDEFFECT_LIGHTING
    float3 normal: NORMAL0;
    float3 worldpos: POSITION0;
#endif

    float4 pos: SV_Position;
};

cbuffer PI_CBUFFER: register(b0)
{
    float4x4 view_proj;

    float3 c_ambient; // Only used when lighting is on
};


PI_PSINPUT VSmain(PI_VSINPUT input)
{
    float4 vpos = input.vpos;
    vpos = mul(input.itransform, vpos);

    PI_PSINPUT output;
    output.pos = mul(view_proj, vpos);
    output.color = input.ialbedo;
#ifdef WORLDEFFECT_LIGHTING
    output.normal = input.vnormal;
    output.worldpos = vpos;
#else
    output.color *= input.vcol;
#endif
    return output;
}


float4 PSmain(PI_PSINPUT input): SV_Target
{
    float4 color = input.color;
#ifdef WORLDEFFECT_LIGHTING
    {
        float3 lcol = c_ambient; 

        for (uint i = 0; i < NUM_DIRLIGHTS; i++) {
            const PI_DIRLIGHT light = LIGHTS[i].dirlight;
            float diffuse = dot(light.direction.xyz, -input.normal);
            diffuse = max(diffuse, 0.0);

            lcol += diffuse * light.color.rgb * light.color.a;
        }
        for (uint i = 0; i < NUM_POINTLIGHTS; i++) {
            const PI_POINTLIGHT light = LIGHTS[i].ptlight;
            float3 direction = light.position.xyz - input.worldpos;
            direction = normalize(direction);
            float diffuse = dot(direction, input.normal);
            diffuse = max(diffuse, 0.0);

            lcol += diffuse * light.color.rgb * light.color.a;
        }

        color.rgb *= lcol;
    }    
#endif
    return color;
}

//
