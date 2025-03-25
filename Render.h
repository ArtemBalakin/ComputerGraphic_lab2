#ifndef RENDER_H
#define RENDER_H
#include <d3d11.h>
#include <vector>
#include "CelestialBody.h"
#include "FollowCamera.h"
#include "Grid.h"

struct ConstantBuffer {
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 viewProj;
    DirectX::XMFLOAT3 objectColor;
    float padding; // Для выравнивания
};

class Render {
public:
    bool Initialize(HWND hwnd, int width, int height);
    void RenderScene(FollowCamera* camera, const std::vector<std::unique_ptr<CelestialBody>>& bodies, Grid* grid);
    void Cleanup();

private:
    bool CreateDeviceAndSwapChain(HWND hwnd, int width, int height);
    bool CreateRenderTargetAndDepthStencil(int width, int height);
    bool CreateShadersAndInputLayout();
    bool CreateBuffers();
public:
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;
    ID3D11DepthStencilView* depthStencilView = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    ID3D11Buffer* vertexBufferKatamari = nullptr;
    ID3D11Buffer* indexBufferKatamari = nullptr;
    UINT indexCountKatamari = 0;
    ID3D11Buffer* vertexBufferObject = nullptr;
    ID3D11Buffer* indexBufferObject = nullptr;
    UINT indexCountObject = 0;
    ID3D11Buffer* constantBuffer = nullptr;
};
#endif