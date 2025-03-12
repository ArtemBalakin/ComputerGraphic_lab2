#include "Grid.h"
#include <iostream>

struct Vertex {
    XMFLOAT3 pos;
    XMFLOAT3 color;
};

Grid::Grid(ID3D11Device* device, float size, int divisions) {
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    float halfSize = size / 2.0f;
    float step = size / divisions;

    // Создаём линии сетки
    for (int i = 0; i <= divisions; ++i) {
        float x = -halfSize + i * step;
        vertices.push_back({ XMFLOAT3(x, 0, -halfSize), XMFLOAT3(0.5f, 0.5f, 0.5f) }); // Серый цвет
        vertices.push_back({ XMFLOAT3(x, 0, halfSize), XMFLOAT3(0.5f, 0.5f, 0.5f) });

        float z = -halfSize + i * step;
        vertices.push_back({ XMFLOAT3(-halfSize, 0, z), XMFLOAT3(0.5f, 0.5f, 0.5f) });
        vertices.push_back({ XMFLOAT3(halfSize, 0, z), XMFLOAT3(0.5f, 0.5f, 0.5f) });
    }

    for (UINT i = 0; i < vertices.size(); i += 2) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();
    device->CreateBuffer(&bd, &initData, &vertexBuffer);

    bd.ByteWidth = sizeof(UINT) * indices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices.data();
    device->CreateBuffer(&bd, &initData, &indexBuffer);

    indexCount = indices.size();
}

Grid::~Grid() {
    if (vertexBuffer) vertexBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
}

void Grid::Draw(ID3D11DeviceContext* context) {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context->DrawIndexed(indexCount, 0, 0);
}