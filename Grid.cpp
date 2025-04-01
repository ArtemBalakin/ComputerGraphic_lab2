#include "Grid.h"

#include "Logger.h"



struct Vertex {

    DirectX::XMFLOAT3 position;

    DirectX::XMFLOAT3 normal;

};



Grid::Grid(ID3D11Device* device, float size, int divisions)

    : size(size), divisions(divisions), vertexBuffer(nullptr), indexBuffer(nullptr) {

    logger << "[Grid] Constructor called: size=" << size << ", divisions=" << divisions << std::endl;



    // Generate grid vertices

    float halfSize = size / 2.0f;

    float step = size / divisions;



    for (int i = 0; i <= divisions; ++i) {

        float x = -halfSize + i * step;

        // Line along Z-axis

        vertices.push_back(x);          // X

        vertices.push_back(0.0f);       // Y

        vertices.push_back(-halfSize);  // Z

        vertices.push_back(0.0f);       // Normal X

        vertices.push_back(1.0f);       // Normal Y

        vertices.push_back(0.0f);       // Normal Z



        vertices.push_back(x);

        vertices.push_back(0.0f);

        vertices.push_back(halfSize);

        vertices.push_back(0.0f);

        vertices.push_back(1.0f);

        vertices.push_back(0.0f);



        // Line along X-axis

        vertices.push_back(-halfSize);

        vertices.push_back(0.0f);

        vertices.push_back(x);

        vertices.push_back(0.0f);

        vertices.push_back(1.0f);

        vertices.push_back(0.0f);



        vertices.push_back(halfSize);

        vertices.push_back(0.0f);

        vertices.push_back(x);

        vertices.push_back(0.0f);

        vertices.push_back(1.0f);

        vertices.push_back(0.0f);

    }



    // Generate indices for lines

    for (unsigned int i = 0; i < (divisions + 1) * 2; ++i) {

        indices.push_back(i * 2);

        indices.push_back(i * 2 + 1);

    }



    InitializeBuffers(device);

}



Grid::~Grid() {

    if (vertexBuffer) vertexBuffer->Release();

    if (indexBuffer) indexBuffer->Release();

}



void Grid::InitializeBuffers(ID3D11Device* device) {

    // Create vertex buffer

    D3D11_BUFFER_DESC vbDesc = {};

    vbDesc.Usage = D3D11_USAGE_DEFAULT;

    vbDesc.ByteWidth = vertices.size() * sizeof(float);

    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    vbDesc.CPUAccessFlags = 0;



    D3D11_SUBRESOURCE_DATA vbData = {};

    vbData.pSysMem = vertices.data();



    HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);

    if (FAILED(hr)) {

        logger << "[Grid] Failed to create vertex buffer" << std::endl;

    }



    // Create index buffer

    D3D11_BUFFER_DESC ibDesc = {};

    ibDesc.Usage = D3D11_USAGE_DEFAULT;

    ibDesc.ByteWidth = indices.size() * sizeof(unsigned int);

    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    ibDesc.CPUAccessFlags = 0;



    D3D11_SUBRESOURCE_DATA ibData = {};

    ibData.pSysMem = indices.data();



    hr = device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);

    if (FAILED(hr)) {

        logger << "[Grid] Failed to create index buffer" << std::endl;

    }

}



void Grid::Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj) const {

    logger << "[Grid] Drawing grid: size=" << size << ", divisions=" << divisions << std::endl;



    // Set up world matrix (grid is at Y=0)

    DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

    DirectX::XMMATRIX worldViewProj = world * viewProj;



    // Update constant buffer

    struct ConstantBufferData {

        DirectX::XMMATRIX worldViewProj;

        DirectX::XMFLOAT4 color;

        BOOL useTexture;

    } cbData;

    cbData.worldViewProj = DirectX::XMMatrixTranspose(worldViewProj);

    cbData.color = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);  // Gray color for grid

    cbData.useTexture = FALSE;

    context->UpdateSubresource(constantBuffer, 0, nullptr, &cbData, 0, 0);



    // Set vertex buffer

    UINT stride = 6 * sizeof(float);  // Position + Normal

    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);



    // Draw

    context->DrawIndexed(indices.size(), 0, 0);

}