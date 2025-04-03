#pragma once
#include <d3d11.h>
#include <vector>
#include <memory>
#include "CelestialBody.h"
#include "Ground.h"

class Render {
public:
    Render(HWND hwnd);
    ~Render();

    bool Initialize();
    void RenderScene(const std::vector<std::unique_ptr<CelestialBody>>& bodies, const Ground* ground,
                     DirectX::XMMATRIX viewProj, DirectX::XMFLOAT3 cameraPos);
    ID3D11Device* GetDevice() { return device; }

private:
    HWND hwnd;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11Texture2D* depthStencilBuffer;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShaderTextured;
    ID3D11PixelShader* pixelShaderColored;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* constantBuffer;
    ID3D11SamplerState* samplerState;
};