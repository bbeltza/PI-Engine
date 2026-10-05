#define _CRT_SECURE_NO_WARNINGS

#include <d3d10.h>
#include <d3dcompiler.h>
#include <cstdio>
#include <cassert>

#include <memory>

#define COMCHECKHR_EX(hr, call)
#include "../../../include/common/comhelpers.h"

#pragma comment(lib, "D3D10.lib")
#pragma comment(lib, "d3dcompiler.lib") // d3dcompiler.lib gets included, for D3DDisassemble.

constexpr HRESULT ERRNO_TO_HR(errno_t err) {
    switch (err)
    {
        case 0: return S_OK;
        case ENOENT: return ERROR_FILE_NOT_FOUND;
        case ENOMEM: return E_OUTOFMEMORY;
        case EACCES: return E_ACCESSDENIED;
        default: return E_FAIL;
    }
}

struct D3D10IncludeHandler: ID3D10Include
{
    STDMETHOD(Open)(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
    {
        if (IncludeType != D3D_INCLUDE_LOCAL || IncludeType != D3D10_INCLUDE_LOCAL) {
            return E_NOTIMPL;
        }
        
        FILE* file;
        errno_t err;
        err = fopen_s(&file, pFileName, "rb");
        if (err) {
            return ERRNO_TO_HR(err);
        }

        long size;
        void* data;
        if (fseek(file, 0, SEEK_END)) {
            goto ERRCLOSE;
        }

        size = ftell(file);
        if (size == -1) {
            err = errno;
            goto ERRCLOSE;
        }
        if (!size) {
            goto SUCCLOSE;
        }
        
        if (fseek(file, 0, SEEK_SET)) {
            err = errno;
            goto ERRCLOSE;
        }

        data = operator new (size, std::nothrow);
        if (!data) {
            err = errno;
            goto ERRCLOSE;
        }

        if (fread_s(data, size, size, 1, file) != 1) {
            err = errno;
            operator delete(data, std::nothrow);
            goto ERRCLOSE;
        }

        *ppData = data;
        *pBytes = size;
    SUCCLOSE:
        fclose(file);
        return S_OK;

    ERRCLOSE:
        fclose(file);
        return ERRNO_TO_HR(err);
    }

    STDMETHOD(Close)(LPCVOID pData) override
    {
        operator delete(const_cast<LPVOID>(pData), std::nothrow);
        return ERRNO_TO_HR(errno);
    }
};

const UINT COMPILER_FLAGS = D3D10_SHADER_WARNINGS_ARE_ERRORS |
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


bool compile_shader(const SHADER_INFO& info, FILE* output_h, LPCSTR input_data, SIZE_T input_len, LPCSTR input_path)
{
    if (!output_h) {
        output_h = stdout;
    }

    std::printf("Compiling \"%s\" at \"%s\"...\n", info.entry, input_path);
    HRESULT hr;

    D3D10IncludeHandler includehandler;
    ID3D10Blob* shaderblob = NULL;
    ID3D10Blob* errorblob = NULL;
    
    COMCHECKCALL(D3D10CompileShader(
            input_data,
            input_len,
            input_path,
            
            NULL,
            &includehandler,

            info.entry,
            info.profile,
            COMPILER_FLAGS,
            
            &shaderblob,
            &errorblob
        ));
    if (FAILED(hr)) {
        std::fflush(stdout);

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

    const char* INPUT_SHADER = "shaders.hlsl";
    const char* OUTPUT_HEADER = "shaders.h";
    errno_t err;
    FILE* output_tmp = NULL;
    err = tmpfile_s(&output_tmp);
    if (err) {
        std::printf("Failed opening output temporary header file: \"%s\"", strerror(err));
        return -1;
    }

    FILE* input_hlsl = NULL;
    err = fopen_s(&input_hlsl, INPUT_SHADER, "rb");
    if (err) {
        std::printf("Failed opening input shader \"%s\": \"%s\"", INPUT_SHADER, strerror(err));
        fclose(output_tmp);
        return -1;
    }

    std::fseek(input_hlsl, 0, SEEK_END);
    int input_hlslsize = std::ftell(input_hlsl);

    std::unique_ptr<char[]> buf{ new char[input_hlslsize] };
    std::fseek(input_hlsl, 0, SEEK_SET);
    std::fread(buf.get(), input_hlslsize, 1, input_hlsl);
    std::fclose(input_hlsl);
    input_hlsl = NULL;

    for (const auto& shader : shaders) {
        if (!compile_shader(shader, output_tmp, buf.get(), input_hlslsize, INPUT_SHADER)) {
            fclose(output_tmp);
            return -1;
        }
    }

    std::fseek(output_tmp, 0, SEEK_END);
    long output_tmpsize = std::ftell(output_tmp);
    std::fseek(output_tmp, 0, SEEK_SET);

    /* Now, using the win32 API to write to the header */
    
    HANDLE houtput = CreateFile(OUTPUT_HEADER, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!houtput) {
        std::printf("Failed opening output header \"%s\"\n", OUTPUT_HEADER);
        fclose(output_tmp);
        return -1;
    }
    
    HANDLE hmapping = CreateFileMapping(houtput, NULL, PAGE_READWRITE, 0, output_tmpsize, NULL);
    if (!hmapping) {
        std::printf("Failed mapping output header file: CreateFileMappingA failed");
        CloseHandle(houtput);
        fclose(output_tmp);
        return -1;
    }

    LPVOID mapped_data = MapViewOfFile(hmapping, FILE_MAP_WRITE, 0, 0, 0);
    assert(mapped_data);
    
    // TODO: Handle more error checking

    std::fread(mapped_data, output_tmpsize, 1, output_tmp);
    std::fclose(output_tmp);

    UnmapViewOfFile(mapped_data);

    CloseHandle(hmapping);
    CloseHandle(houtput);
    return 0;
}
