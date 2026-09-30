#define _CRT_SECURE_NO_WARNINGS

#include <d3dcompiler.h>
#include <cstdio>
#include <cassert>

#define COMCHECKHR_EX(hr, call)
#include "../../../include/common/comhelpers.h"

#pragma comment(lib, "d3dcompiler.lib")

const WCHAR DEFAULT_INPUT_SHADER[] = L"shaders.hlsl";
const UINT COMPILER_FLAGS = D3DCOMPILE_WARNINGS_ARE_ERRORS |
                            0;

/* This source file runs during compilation */
/*  It compiles all of the required D3D10 backend shaders. */
struct SHADER_INFO
{
    const char* profile;
    const char* entry;
    const char* varname;
};

constexpr SHADER_INFO shaders[] = {
    { "vs_4_0", "VSmain", "vs_basic" },
    { "ps_4_0", "PSmain", "ps_basic" }
};

bool compile_shader(const SHADER_INFO& info, FILE* output_h, LPCWSTR input_hlsl = DEFAULT_INPUT_SHADER)
{
    if (!output_h) {
        output_h = stdout;
    }

    std::printf("Compiling \"%s\" at \"%ls\"...\n", info.entry, input_hlsl);
    HRESULT hr;
    ID3DBlob* shaderblob = NULL;
    ID3DBlob* errorblob = NULL;
    COMCHECKCALL(D3DCompileFromFile(
            input_hlsl,
            NULL,
            NULL,

            info.entry,
            info.profile,
            COMPILER_FLAGS,
            0,
            
            &shaderblob,
            &errorblob
        ));
    if (FAILED(hr)) {
        std::fflush(stdout);
        if (hr == MAKE_HRESULT(1, FACILITY_WIN32, ERROR_FILE_NOT_FOUND)) {
            std::fprintf(stderr, "Compilation Error: File \"%ws\" not found.", input_hlsl);
            return false;
        }

        assert(errorblob != NULL);
        std::fputs("Compilation Error(s):\n", stderr);
        std::fwrite(errorblob->GetBufferPointer(), errorblob->GetBufferSize(), 1, stderr);
        COMRELEASE(errorblob);
        return false;
    }

    {
        UINT DISASSEMBLY_FLAGS = D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING |
                                 D3D_DISASM_ENABLE_INSTRUCTION_OFFSET |
                                 D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS |
                                 0;
        ID3DBlob* disassembly = NULL;
        COMCHECKCALL(D3DDisassemble(
                    shaderblob->GetBufferPointer(),
                    shaderblob->GetBufferSize(),
                    DISASSEMBLY_FLAGS,
                    NULL,
                    &disassembly
                ));
        std::fputs("\n/*\n", output_h);
            std::fwrite(disassembly->GetBufferPointer(), disassembly->GetBufferSize(), 1, output_h);
        std::fputs("*/\n", output_h);
        COMRELEASE(disassembly);
    }

    const char* VAR_FMT = "const unsigned char %s[] = {";
    const char* VAR_END = "\n};\n";
    std::fprintf(output_h, VAR_FMT, info.varname);

    auto shaderptr = static_cast<const BYTE*>(shaderblob->GetBufferPointer());
    auto shadersize = shaderblob->GetBufferSize();
    for (SIZE_T i = 0;; i++) {
        const char* FMT = "0x%02x";
        if ((i % 8) == 0) {
            std::fputs("\n\t", output_h);
        }

        std::fprintf(output_h, FMT, shaderptr[i]);
        if (i >= shadersize-1) {
            break;
        }
        
        std::fputs(", ", output_h);
    }

    std::fputs(VAR_END, output_h);

    COMRELEASE(shaderblob);
    COMRELEASESAFE(errorblob);
    return true;
}

int main()
{
    HRESULT hr;
    (void)hr;
    
    std::printf("D3D10 Shader Compiler!\n");

    const char* OUTPUT_HEADER = "shaders.h";
    FILE* output_h = NULL;
    errno_t err = fopen_s(&output_h, OUTPUT_HEADER, "w");
    if (err) {
        std::printf("Failed opening output header \"%s\": \"%s\"", OUTPUT_HEADER, strerror(err));
        return -1;
    }

    for (const auto& shader : shaders) {
        if (!compile_shader(shader, output_h)) {
            fclose(output_h);
            remove(OUTPUT_HEADER);
            return -1;
        }
    }

    fclose(output_h);
    return 0;
}
