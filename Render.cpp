#include "Render.h"

#include <codecvt>
#include <d3dcompiler.h>
#include <iostream>
#include <locale>

Render::Render() : device(nullptr), context(nullptr), swapChain(nullptr), renderTargetView(nullptr),
                   depthStencilView(nullptr), vertexShader(nullptr), pixelShader(nullptr), inputLayout(nullptr) {
    std::cout << "=== Render object constructed ===" << std::endl;
}

Render::~Render() {
    Cleanup();
    std::cout << "=== Render object destroyed ===" << std::endl;
}

bool Render::Initialize(HWND hwnd, int width, int height) {
    std::cout << "=== Initializing Render System ===" << std::endl;
    std::cout << "Window handle: " << hwnd << ", Width: " << width << ", Height: " << height << std::endl;

    // Настройка описания цепочки обмена (Swap Chain)
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    std::cout << "Configured swap chain description: BufferCount=" << scd.BufferCount << ", Format=R8G8B8A8_UNORM, Windowed=" << scd.Windowed << std::endl;

    // Создание устройства DirectX и цепочки обмена
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                               D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create device and swap chain. Error code: " << hr << std::endl;
        return false;
    }
    std::cout << "Device created: " << device << ", SwapChain created: " << swapChain << ", Context created: " << context << std::endl;

    // Получение заднего буфера и создание Render Target View
    ID3D11Texture2D* backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to get back buffer. Error code: " << hr << std::endl;
        return false;
    }
    std::cout << "Back buffer retrieved: " << backBuffer << std::endl;

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create render target view. Error code: " << hr << std::endl;
        return false;
    }
    std::cout << "Render target view created: " << renderTargetView << std::endl;

    // Создание текстуры глубины и трафарета (Depth Stencil)
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    std::cout << "Configured depth stencil description: Width=" << depthDesc.Width << ", Height=" << depthDesc.Height << ", Format=D24_UNORM_S8_UINT" << std::endl;

    ID3D11Texture2D* depthStencil;
    hr = device->CreateTexture2D(&depthDesc, nullptr, &depthStencil);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create depth stencil texture. Error code: " << hr << std::endl;
        return false;
    }
    std::cout << "Depth stencil texture created: " << depthStencil << std::endl;

    hr = device->CreateDepthStencilView(depthStencil, nullptr, &depthStencilView);
    depthStencil->Release();
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create depth stencil view. Error code: " << hr << std::endl;
        return false;
    }
    std::cout << "Depth stencil view created: " << depthStencilView << std::endl;

    // Установка целей рендеринга
    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    std::cout << "Render targets set: RenderTargetView=" << renderTargetView << ", DepthStencilView=" << depthStencilView << std::endl;

    // Настройка области отображения (Viewport)
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
    std::cout << "Viewport set to " << viewport.Width << "x" << viewport.Height << ", MinDepth: " << viewport.MinDepth << ", MaxDepth: " << viewport.MaxDepth << std::endl;

    // Компиляция шейдеров из папки Shaders
    ID3DBlob* vsBlob, * psBlob;
    std::cout << "Compiling vertex shader from ../Shaders/Shader.hlsl..." << std::endl;
    if (!CompileShader(L"../Shaders/Shader.hlsl", "VS", "vs_5_0", &vsBlob)) {
        std::cout << "ERROR: Failed to compile vertex shader." << std::endl;
        return false;
    }
    std::cout << "Vertex shader compiled successfully." << std::endl;

    std::cout << "Compiling pixel shader from ../Shaders/Shader.hlsl..." << std::endl;
    if (!CompileShader(L"../Shaders/Shader.hlsl", "PS", "ps_5_0", &psBlob)) {
        std::cout << "ERROR: Failed to compile pixel shader." << std::endl;
        vsBlob->Release();
        return false;
    }
    std::cout << "Pixel shader compiled successfully." << std::endl;

    // Создание вершинного шейдера
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create vertex shader. Error code: " << hr << std::endl;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }
    std::cout << "Vertex shader created: " << vertexShader << std::endl;

    // Создание пиксельного шейдера
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create pixel shader. Error code: " << hr << std::endl;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }
    std::cout << "Pixel shader created: " << pixelShader << std::endl;

    // Определение входного макета (Input Layout)
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    std::cout << "Creating input layout with 2 elements (POSITION, COLOR)..." << std::endl;
    hr = device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create input layout. Error code: " << hr << std::endl;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }
    std::cout << "Input layout created: " << inputLayout << std::endl;

    // Установка входного макета и шейдеров
    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);
    std::cout << "Input layout and shaders set for context: InputLayout=" << inputLayout << ", VertexShader=" << vertexShader << ", PixelShader=" << pixelShader << std::endl;

    // Освобождение блобов шейдеров
    vsBlob->Release();
    psBlob->Release();
    std::cout << "Shader blobs released." << std::endl;

    std::cout << "=== Render System Initialized Successfully ===" << std::endl;
    return true;
}

void Render::BeginFrame() {
    std::cout << "=== Beginning frame ===" << std::endl;
    float clearColor[] = { 0.0f, 0.0f, 0.5f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, clearColor);
    std::cout << "Render target cleared with color (0, 0, 0.5, 1)." << std::endl;
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    std::cout << "Depth stencil cleared with depth 1.0." << std::endl;
}

void Render::EndFrame() {
    std::cout << "Presenting frame..." << std::endl;
    swapChain->Present(1, 0);
    std::cout << "Frame presented successfully." << std::endl;
}

void Render::DrawIndexed(UINT indexCount) {
    std::cout << "Drawing " << indexCount << " indices..." << std::endl;
    context->DrawIndexed(indexCount, 0, 0);
    std::cout << "DrawIndexed call completed." << std::endl;
}

void Render::Cleanup() {
    std::cout << "=== Cleaning up render resources ===" << std::endl;
    if (inputLayout) {
        inputLayout->Release();
        std::cout << "Input layout released." << std::endl;
    }
    if (vertexShader) {
        vertexShader->Release();
        std::cout << "Vertex shader released." << std::endl;
    }
    if (pixelShader) {
        pixelShader->Release();
        std::cout << "Pixel shader released." << std::endl;
    }
    if (depthStencilView) {
        depthStencilView->Release();
        std::cout << "Depth stencil view released." << std::endl;
    }
    if (renderTargetView) {
        renderTargetView->Release();
        std::cout << "Render target view released." << std::endl;
    }
    if (swapChain) {
        swapChain->Release();
        std::cout << "Swap chain released." << std::endl;
    }
    if (context) {
        context->Release();
        std::cout << "Device context released." << std::endl;
    }
    if (device) {
        device->Release();
        std::cout << "Device released." << std::endl;
    }
    std::cout << "Cleanup completed." << std::endl;
}

bool Render::CompileShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& target, ID3DBlob** blob) {
    std::cout << "=== Compiling shader ===" << std::endl;
    std::wcout << L"Checking file: " << fileName << std::endl;
    std::cout << "File: " << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fileName) << ", Entry point: " << entryPoint << ", Target: " << target << std::endl;
    ID3DBlob* errorBlob;
    HRESULT hr = D3DCompileFromFile(fileName.c_str(), nullptr, nullptr, entryPoint.c_str(), target.c_str(), 0, 0, blob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cout << "ERROR: Shader compilation failed: " << (char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        } else {
            std::cout << "ERROR: Shader compilation failed with unknown error. HRESULT: " << hr << std::endl;
        }
        return false;
    }
    if (errorBlob) errorBlob->Release();
    std::cout << "Shader compiled successfully. Blob size: " << (*blob)->GetBufferSize() << " bytes." << std::endl;
    return true;
}