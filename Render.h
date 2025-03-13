#pragma once
#include <d3d11.h>
#include <string>

class Render {
public:
    Render();

    ~Render();

    bool Initialize(HWND hwnd, int width, int height);

    void BeginFrame();

    void EndFrame();

    void DrawIndexed(UINT indexCount);

    void Cleanup();

    ID3D11Device *GetDevice() const { return device; }
    ID3D11DeviceContext *GetContext() const { return context; }

private:
    bool CompileShader(const std::wstring &fileName, const std::string &entryPoint, const std::string &target,
                       ID3DBlob **blob);

    ID3D11Device *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain *swapChain;
    ID3D11RenderTargetView *renderTargetView;
    ID3D11DepthStencilView *depthStencilView;
    ID3D11VertexShader *vertexShader;
    ID3D11PixelShader *pixelShader;
    ID3D11InputLayout *inputLayout;
};
