#include "Render.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 color;
};

void GenerateSphere(float radius, int slices, int stacks, std::vector<Vertex>& vertices, std::vector<UINT>& indices, bool colored) {
    for (int i = 0; i <= stacks; ++i) {
        float theta = i * DirectX::XM_PI / stacks;
        for (int j = 0; j <= slices; ++j) {
            float phi = j * 2 * DirectX::XM_PI / slices;
            float x = sin(theta) * cos(phi);
            float y = sin(theta) * sin(phi);
            float z = cos(theta);
            Vertex v;
            v.pos = DirectX::XMFLOAT3(x * radius, y * radius, z * radius);
            if (colored) {
                v.color = DirectX::XMFLOAT3(theta / DirectX::XM_PI, phi / (2 * DirectX::XM_PI), 0.5f);
            } else {
                v.color = DirectX::XMFLOAT3(1, 1, 1);
            }
            vertices.push_back(v);
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int a = i * (slices + 1) + j;
            int b = a + 1;
            int c = (i + 1) * (slices + 1) + j;
            int d = c + 1;
            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }
}

bool Render::Initialize(HWND hwnd, int width, int height) {
    if (!CreateDeviceAndSwapChain(hwnd, width, height)) return false;
    if (!CreateRenderTargetAndDepthStencil(width, height)) return false;
    if (!CreateShadersAndInputLayout()) return false;
    if (!CreateBuffers()) return false;
    return true;
}

bool Render::CreateDeviceAndSwapChain(HWND hwnd, int width, int height) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &deviceContext);
    return SUCCEEDED(hr);
}

bool Render::CreateRenderTargetAndDepthStencil(int width, int height) {
    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthStencil;
    device->CreateTexture2D(&depthDesc, nullptr, &depthStencil);
    device->CreateDepthStencilView(depthStencil, nullptr, &depthStencilView);
    depthStencil->Release();

    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
    D3D11_VIEWPORT viewport = { 0, 0, (float)width, (float)height, 0.0f, 1.0f };
    deviceContext->RSSetViewports(1, &viewport);
    return true;
}

bool Render::CreateShadersAndInputLayout() {
    const char* vsCode = R"(
        cbuffer ConstantBuffer : register(b0) {
            float4x4 world;
            float4x4 viewProj;
            float3 objectColor;
        }
        struct VS_INPUT {
            float3 pos : POSITION;
            float3 color : COLOR;
        };
        struct PS_INPUT {
            float4 pos : SV_POSITION;
            float3 color : COLOR;
        };
        PS_INPUT main(VS_INPUT input) {
            PS_INPUT output;
            float4 pos = float4(input.pos, 1.0);
            pos = mul(pos, world);
            pos = mul(pos, viewProj);
            output.pos = pos;
            output.color = input.color * objectColor;
            return output;
        }
    )";

    const char* psCode = R"(
        struct PS_INPUT {
            float4 pos : SV_POSITION;
            float3 color : COLOR;
        };
        float4 main(PS_INPUT input) : SV_TARGET {
            return float4(input.color, 1.0);
        }
    )";

    ID3DBlob* vsBlob, * psBlob, * errorBlob;
    HRESULT hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) errorBlob->Release();
        return false;
    }

    hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) errorBlob->Release();
        vsBlob->Release();
        return false;
    }

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    device->CreateInputLayout(ied, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

    vsBlob->Release();
    psBlob->Release();
    return true;
}

bool Render::CreateBuffers() {
    std::vector<Vertex> verticesKatamari;
    std::vector<UINT> indicesKatamari;
    GenerateSphere(1.0f, 30, 30, verticesKatamari, indicesKatamari, true); // Увеличены slices и stacks для сглаживания

    std::vector<Vertex> verticesObject;
    std::vector<UINT> indicesObject;
    GenerateSphere(1.0f, 30, 30, verticesObject, indicesObject, false);

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * verticesKatamari.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = verticesKatamari.data();
    device->CreateBuffer(&bd, &initData, &vertexBufferKatamari);

    bd.ByteWidth = sizeof(UINT) * indicesKatamari.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indicesKatamari.data();
    device->CreateBuffer(&bd, &initData, &indexBufferKatamari);
    indexCountKatamari = indicesKatamari.size();

    bd.ByteWidth = sizeof(Vertex) * verticesObject.size();
    initData.pSysMem = verticesObject.data();
    device->CreateBuffer(&bd, &initData, &vertexBufferObject);

    bd.ByteWidth = sizeof(UINT) * indicesObject.size();
    initData.pSysMem = indicesObject.data();
    device->CreateBuffer(&bd, &initData, &indexBufferObject);
    indexCountObject = indicesObject.size();

    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&bd, nullptr, &constantBuffer);
    return true;
}

void Render::RenderScene(FollowCamera* camera, const std::vector<std::unique_ptr<CelestialBody>>& bodies, Grid* grid) {
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);
    deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    deviceContext->VSSetShader(vertexShader, nullptr, 0);
    deviceContext->PSSetShader(pixelShader, nullptr, 0);
    deviceContext->IASetInputLayout(inputLayout);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    grid->Draw(deviceContext);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetVertexBuffers(0, 1, &vertexBufferKatamari, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBufferKatamari, DXGI_FORMAT_R32_UINT, 0);

    ConstantBuffer cb;
    XMStoreFloat4x4(&cb.viewProj, XMMatrixTranspose(camera->GetViewProjection()));

    // Отрисовка Katamari
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(bodies[0]->GetWorldMatrix()));
    cb.objectColor = DirectX::XMFLOAT3(bodies[0]->color.x, bodies[0]->color.y, bodies[0]->color.z);
    deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
    deviceContext->DrawIndexed(indexCountKatamari, 0, 0);

    // Отрисовка всех объектов
    deviceContext->IASetVertexBuffers(0, 1, &vertexBufferObject, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBufferObject, DXGI_FORMAT_R32_UINT, 0);
    for (size_t i = 1; i < bodies.size(); ++i) {
        XMStoreFloat4x4(&cb.world, XMMatrixTranspose(bodies[i]->GetWorldMatrix()));
        cb.objectColor = DirectX::XMFLOAT3(bodies[i]->color.x, bodies[i]->color.y, bodies[i]->color.z);
        deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
        deviceContext->DrawIndexed(indexCountObject, 0, 0);
    }

    swapChain->Present(0, 0);
}

void Render::Cleanup() {
    if (constantBuffer) constantBuffer->Release();
    if (indexBufferKatamari) indexBufferKatamari->Release();
    if (vertexBufferKatamari) vertexBufferKatamari->Release();
    if (indexBufferObject) indexBufferObject->Release();
    if (vertexBufferObject) vertexBufferObject->Release();
    if (inputLayout) inputLayout->Release();
    if (pixelShader) pixelShader->Release();
    if (vertexShader) vertexShader->Release();
    if (depthStencilView) depthStencilView->Release();
    if (renderTargetView) renderTargetView->Release();
    if (swapChain) swapChain->Release();
    if (deviceContext) deviceContext->Release();
    if (device) device->Release();
}