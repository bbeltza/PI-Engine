/*
    Preprocessor definitions
    - Analogous to the PI_WORLDEFFECT_ flags:
        #define WORLDEFFECT_LIGHTING
            Enable lighting.
        #define WORLDEFFECT_SPECULAR
            Enable specular shading (will likely be in blinn-phong). WORLDEFFECT_LIGHTING must be defined.
 */

#include "shader_properties.h"

#ifndef WORLDEFFECT_LIGHTING
    #ifdef WORLDEFFECT_SPECULAR
        #error "Can't compile with no WORLDEFFECT_LIGHTING but WORLDEFFECT_SPECULAR defined."
    #endif
#else
    struct PI_POINTLIGHT {
        float4 position; /* "w" component ignored. Position of the light */
        float4 color; /* Color and intensity (in the "a" channel) of the light */
        float range;
        float attenuation;

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


struct PI_VSINPUT
{
    float4 vpos: POSITION0;
#ifdef WORLDEFFECT_LIGHTING
    float3 vnormal: COLOR0;
#else
    float4 vcol: COLOR0;
#endif

    float4x4 itransform: POSITION1;
    float4 ialbedo: COLOR1;

    float3x3 inormalmat: NORMAL0;
};


struct PI_GSINPUT
{};

struct PI_PSINPUT
{
    float4 color: COLOR0;
#ifdef WORLDEFFECT_LIGHTING
    float3 normal: NORMAL0;
    float3 worldpos: POSITION0;
    float3 dif_color: COLOR1;
#endif

    float4 pos: SV_Position;
};

    /* VS environment (constant buffer bound at slot 0 with VSSetConstantBuffers) */
cbuffer VI_VSENV: register(b0)
{
    float4x4 cvs_viewproj;
}

    /* PS environment (constant buffer bound at slot 0 with PSSetConstantBuffers) */
cbuffer PI_PSENV: register(b0)
{
    float3 cps_ambient;
        /* Only when specular lighting is on */
    float3 cps_viewpos;
    float cps_specular_exponent;
    float4 cps_specular_color;
}

PI_PSINPUT VSmain(PI_VSINPUT input)
{
    float4 vpos = input.vpos;
    vpos = mul(input.itransform, vpos);

    PI_PSINPUT output;
    output.pos = mul(cvs_viewproj, vpos);
    output.color = input.ialbedo;
#ifdef WORLDEFFECT_LIGHTING
    float3 normal = mul(input.inormalmat, input.vnormal);
    normal = normalize(normal);

    output.normal = normal;
    output.worldpos = vpos;
    output.dif_color = float3(0.0, 0.0, 0.0);

    for (uint i = 0; i < NUM_DIRLIGHTS; i++) {
        const PI_DIRLIGHT light = LIGHTS[i].dirlight;
        float diffuse = dot(light.direction.xyz, -normal);
        diffuse = max(diffuse, 0.0);
        diffuse = pow(diffuse, 2.2);

        output.dif_color += diffuse * light.color.rgb * light.color.a;
    }
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
        float3 lcol = cps_ambient + input.dif_color; 
        
    #ifdef WORLDEFFECT_SPECULAR
        float3 lspec = float3(0.0, 0.0, 0.0);
        float3 viewdir = cps_viewpos - input.worldpos;
        viewdir = normalize(viewdir);
    #endif
        for (uint i = 0; i < NUM_POINTLIGHTS; i++) {
            const PI_POINTLIGHT light = LIGHTS[i].ptlight;
            float3 direction = light.position.xyz - input.worldpos;
            direction = normalize(direction);
            float diffuse = dot(direction, input.normal);
            diffuse = max(diffuse, 0.0);
            diffuse = pow(diffuse, 2.2); // Diffuse has to be converted to linear space.

            float d = distance(light.position.xyz, input.worldpos);
            diffuse *= 1.0 - smoothstep(light.range - light.attenuation, light.range, d);

        #ifdef WORLDEFFECT_SPECULAR
            float3 reflection = reflect(direction, viewdir);
            reflection = normalize(reflection);
            float spec = dot(reflection, input.normal);
            spec = max(spec, 0.0);
            spec = pow(spec, cps_specular_exponent);
            lspec += diffuse * spec * cps_specular_color.rgb * cps_specular_color.a;
        #endif

            lcol += diffuse * light.color.rgb * light.color.a;
        }

        color.rgb *= lcol;
    #ifdef WORLDEFFECT_SPECULAR
        color.rgb += lspec;
    #endif
    }    
#endif
    return color;
}

#ifdef WORLDEFFECT_LIGHTING

[maxvertexcount(3*2)]
void GSdebug(triangle PI_PSINPUT input[3], inout LineStream<PI_PSINPUT> output) {
    PI_PSINPUT psinput;
    for (uint i = 0; i < 3; i++) {
        psinput = input[i];
        psinput.dif_color = float3(1.0, 0.0, 1.0);
        output.Append(psinput);

        psinput.pos.xyz += psinput.normal * 0.5;
        output.Append(psinput);
        output.RestartStrip();
    }
}

#endif

//
