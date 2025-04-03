#include "Render.h"
#include "Logger.h"
#include <d3dcompiler.h>
#include "CelestialBody.h"
#include "Ground.h"

Render::Render(HWND hwnd) : hwnd(hwnd), device(nullptr), context(nullptr), swapChain(nullptr),
    renderTargetView(nullptr), depthStencilView(nullptr), depthStencilState(nullptr), depthStencilBuffer(nullptr),
    vertexShader(nullptr), pixelShaderTextured(nullptr), pixelShaderColored(nullptr), inputLayout(nullptr),
    constantBuffer(nullptr), samplerState(nullptr) {
    logger << "[Render] Создан объект Render" << std::endl;
}

Render::~Render() {
    if (constantBuffer) constantBuffer->Release();
    if (inputLayout) inputLayout->Release();
    if (pixelShaderTextured) pixelShaderTextured->Release();
    if (pixelShaderColored) pixelShaderColored->Release();
    if (vertexShader) vertexShader->Release();
    if (depthStencilState) depthStencilState->Release();
    if (depthStencilView) depthStencilView->Release();
    if (depthStencilBuffer) depthStencilBuffer->Release();
    if (renderTargetView) renderTargetView->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();
    logger << "[Render] Объект Render уничтожен" << std::endl;
}

bool Render::Initialize() {
    logger << "[Render] Начало инициализации рендера" << std::endl;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = 800;
    scd.BufferDesc.Height = 600;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать устройство и цепочку обмена" << std::endl;
        return false;
    }
    logger << "[Render] Устройство и цепочка обмена созданы" << std::endl;

    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать RenderTargetView" << std::endl;
        return false;
    }
    logger << "[Render] RenderTargetView создан" << std::endl;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 800;
    depthDesc.Height = 600;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = device->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать буфер глубины" << std::endl;
        return false;
    }
    logger << "[Render] Буфер глубины создан" << std::endl;

    hr = device->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать DepthStencilView" << std::endl;
        return false;
    }
    logger << "[Render] DepthStencilView создан" << std::endl;

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = FALSE;

    hr = device->CreateDepthStencilState(&dsDesc, &depthStencilState);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать DepthStencilState" << std::endl;
        return false;
    }
    logger << "[Render] DepthStencilState создан" << std::endl;

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    context->OMSetDepthStencilState(depthStencilState, 1);
    logger << "[Render] RenderTargets и DepthStencilState установлены" << std::endl;

    D3D11_VIEWPORT viewport = {};
    viewport.Width = 800.0f;
    viewport.Height = 600.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
    logger << "[Render] Viewport установлен" << std::endl;

    ID3DBlob* vsBlob, *psTexturedBlob, *psColoredBlob, *errorBlob;
    hr = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            logger << "[Render] Ошибка компиляции вершинного шейдера: " << (const char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        return false;
    }
    logger << "[Render] Вершинный шейдер скомпилирован" << std::endl;

    hr = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "PSMainTextured", "ps_5_0", 0, 0, &psTexturedBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            logger << "[Render] Ошибка компиляции пиксельного шейдера (Textured): " << (const char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        vsBlob->Release();
        return false;
    }
    logger << "[Render] Пиксельный шейдер (Textured) скомпилирован" << std::endl;

    hr = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "PSMainColored", "ps_5_0", 0, 0, &psColoredBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            logger << "[Render] Ошибка компиляции пиксельного шейдера (Colored): " << (const char*)errorBlob->GetBufferPointer() << std::endl;
            errorBlob->Release();
        }
        vsBlob->Release();
        psTexturedBlob->Release();
        return false;
    }
    logger << "[Render] Пиксельный шейдер (Colored) скомпилирован" << std::endl;

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать вершинный шейдер" << std::endl;
        return false;
    }
    logger << "[Render] Вершинный шейдер создан" << std::endl;

    hr = device->CreatePixelShader(psTexturedBlob->GetBufferPointer(), psTexturedBlob->GetBufferSize(), nullptr, &pixelShaderTextured);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать пиксельный шейдер (Textured)" << std::endl;
        return false;
    }
    logger << "[Render] Пиксельный шейдер (Textured) создан" << std::endl;

    hr = device->CreatePixelShader(psColoredBlob->GetBufferPointer(), psColoredBlob->GetBufferSize(), nullptr, &pixelShaderColored);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать пиксельный шейдер (Colored)" << std::endl;
        return false;
    }
    logger << "[Render] Пиксельный шейдер (Colored) создан" << std::endl;

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    hr = device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать InputLayout" << std::endl;
        vsBlob->Release();
        psTexturedBlob->Release();
        psColoredBlob->Release();
        return false;
    }
    logger << "[Render] InputLayout создан" << std::endl;

    vsBlob->Release();
    psTexturedBlob->Release();
    psColoredBlob->Release();

    struct ConstantBufferData {
        DirectX::XMMATRIX worldViewProj;
        DirectX::XMMATRIX world;
        DirectX::XMFLOAT4 color;
        BOOL useTexture;
        DirectX::XMFLOAT3 lightDir; // Заменяем lightPos на lightDir
        DirectX::XMFLOAT3 lightColor;
        DirectX::XMFLOAT3 materialDiffuse;
        DirectX::XMFLOAT3 materialSpecular;
        float shininess;
        DirectX::XMFLOAT3 emissiveColor;
        DirectX::XMFLOAT3 cameraPos;
        float padding;
    };
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBufferData);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать константный буфер" << std::endl;
        return false;
    }
    logger << "[Render] Константный буфер создан" << std::endl;

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&sampDesc, &samplerState);
    if (FAILED(hr)) {
        logger << "[Render] Ошибка: не удалось создать SamplerState" << std::endl;
        return false;
    }
    logger << "[Render] SamplerState создан" << std::endl;

    context->VSSetShader(vertexShader, nullptr, 0);
    context->IASetInputLayout(inputLayout);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetSamplers(0, 1, &samplerState);
    logger << "[Render] Шейдеры и состояния установлены" << std::endl;

    logger << "[Render] Инициализация рендера завершена успешно" << std::endl;
    return true;
}
void Render::RenderScene(const std::vector<std::unique_ptr<CelestialBody>>& bodies, const Ground* ground,
                        DirectX::XMMATRIX viewProj, DirectX::XMFLOAT3 cameraPos) {
    logger << "[Render] Начало рендеринга сцены" << std::endl;

    if (!context || !ground || !constantBuffer) {
        logger << "[Render] Ошибка: недействительный контекст, ground или constantBuffer" << std::endl;
        return;
    }

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(renderTargetView, clearColor);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    logger << "[Render] Буферы очищены" << std::endl;

    if (ground->HasTexture()) {
        logger << "[Render] Используется текстурный шейдер для ground" << std::endl;
        context->PSSetShader(pixelShaderTextured, nullptr, 0);
    } else {
        logger << "[Render] Используется цветной шейдер для ground" << std::endl;
        context->PSSetShader(pixelShaderColored, nullptr, 0);
    }
    logger << "[Render] Вызов Draw для ground" << std::endl;
    ground->Draw(context, constantBuffer, viewProj, cameraPos);

    context->PSSetShader(pixelShaderTextured, nullptr, 0);
    logger << "[Render] Установлен текстурный шейдер для тел" << std::endl;
    for (const auto& body : bodies) {
        logger << "[Render] Рендеринг тела" << std::endl;
        body->Draw(context, constantBuffer, viewProj, cameraPos);
    }

    swapChain->Present(1, 0);
    logger << "[Render] Сцена представлена на экран" << std::endl;
    logger << "[Render] Рендеринг сцены завершен" << std::endl;
}