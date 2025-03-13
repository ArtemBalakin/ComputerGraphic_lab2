#include "CelestialBody.h"
#include <cmath>
#include <iostream>

CelestialBody::CelestialBody(ID3D11Device *device, bool isSphere, CelestialBody *parent, float scale, XMFLOAT3 color,
                             float orbitInclination)
    : parent(parent), orbitSpeed(0), rotationSpeed(0), orbitRadius(0), scale(scale), color(color),
      vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0), totalOrbitAngle(0.0f),
      orbitInclination(orbitInclination) {
    std::cout << "Creating CelestialBody with parent: " << parent << ", isSphere: " << isSphere << ", scale: " << scale
            << ", color: (" << color.x << ", " << color.y << ", " << color.z << "), orbitInclination: " <<
            orbitInclination << std::endl;
    position = XMFLOAT3(0, 0, 0);
    rotationQuaternion = XMFLOAT4(0, 0, 0, 1);

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;
    if (isSphere) {
        CreateSphere(1.0f, 20, 20, vertices, indices);
    } else {
        CreateCube(1.0f, vertices, indices);
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

CelestialBody::~CelestialBody() {
    if (vertexBuffer) vertexBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
}

void CelestialBody::Update(float deltaTime) {
    if (parent) {
        // Обновляем угол орбиты
        totalOrbitAngle += orbitSpeed * deltaTime;

        // Вычисляем базовую позицию в плоскости XZ
        XMVECTOR orbitPos = XMVectorSet(
            orbitRadius * cos(totalOrbitAngle),
            0,
            orbitRadius * sin(totalOrbitAngle),
            0
        );

        // Применяем наклон орбиты (вращение вокруг оси X)
        XMMATRIX inclinationMatrix = XMMatrixRotationX(orbitInclination);
        orbitPos = XMVector3Transform(orbitPos, inclinationMatrix);

        // Позиция родителя
        XMVECTOR parentPos = XMLoadFloat3(&parent->position);

        // Итоговая позиция = позиция родителя + смещение по орбите
        XMVECTOR newPos = XMVectorAdd(parentPos, orbitPos);

        // Сохраняем новую позицию
        XMStoreFloat3(&position, newPos);

        std::cout << "Orbit updated for body with parent at (" << parent->position.x << ", " << parent->position.y <<
                ", " << parent->position.z
                << "). New position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    } else {
        // Для звезды фиксируем позицию
        position = XMFLOAT3(0, 2.0f, 0);
    }

    // Вращение вокруг своей оси
    float rotationAngle = rotationSpeed * deltaTime;
    XMVECTOR rotation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), rotationAngle);
    XMVECTOR currentQuat = XMLoadFloat4(&rotationQuaternion);
    XMVECTOR newQuat = XMQuaternionMultiply(currentQuat, rotation);
    newQuat = XMQuaternionNormalize(newQuat);
    XMStoreFloat4(&rotationQuaternion, newQuat);

    // Ограничение пространства
    XMFLOAT3 pos = position;
    const float maxDistance = 15.0f;
    bool clamped = false;
    if (pos.x > maxDistance) {
        pos.x = maxDistance;
        clamped = true;
    }
    if (pos.x < -maxDistance) {
        pos.x = -maxDistance;
        clamped = true;
    }
    if (pos.y > maxDistance) {
        pos.y = maxDistance;
        clamped = true;
    }
    if (pos.y < -maxDistance) {
        pos.y = -maxDistance;
        clamped = true;
    }
    if (pos.z > maxDistance) {
        pos.z = maxDistance;
        clamped = true;
    }
    if (pos.z < -maxDistance) {
        pos.z = -maxDistance;
        clamped = true;
    }
    if (clamped) {
        std::cout << "Position clamped to: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        position = pos;
    }
}

void CelestialBody::Draw(ID3D11DeviceContext *context, Render &render, const XMMATRIX &view, const XMMATRIX &projection,
                         ID3D11Buffer *constantBuffer) {
    XMMATRIX translation = XMMatrixTranslation(position.x, position.y, position.z);
    XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rotationQuaternion));
    XMMATRIX scaling = XMMatrixScaling(scale, scale, scale);
    XMMATRIX world = scaling * rotation * translation;
    XMMATRIX worldViewProj = world * view * projection;

    struct ConstantBuffer {
        XMMATRIX worldViewProj;
    };
    ConstantBuffer cb;
    cb.worldViewProj = XMMatrixTranspose(worldViewProj);
    context->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    render.DrawIndexed(indexCount);
}

void CelestialBody::CreateSphere(float radius, int slices, int stacks, std::vector<Vertex> &vertices,
                                 std::vector<UINT> &indices) {
    for (int i = 0; i <= stacks; ++i) {
        float phi = XM_PI * static_cast<float>(i) / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * XM_PI * static_cast<float>(j) / slices;
            Vertex v;
            v.pos.x = radius * sin(phi) * cos(theta);
            v.pos.y = radius * cos(phi);
            v.pos.z = radius * sin(phi) * sin(theta);
            v.color = color;
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void CelestialBody::CreateCube(float size, std::vector<Vertex> &vertices, std::vector<UINT> &indices) {
    float s = size / 2.0f;
    vertices = {
        {XMFLOAT3(-s, -s, -s), color},
        {XMFLOAT3(s, -s, -s), color},
        {XMFLOAT3(s, s, -s), color},
        {XMFLOAT3(-s, s, -s), color},
        {XMFLOAT3(-s, -s, s), color},
        {XMFLOAT3(s, -s, s), color},
        {XMFLOAT3(s, s, s), color},
        {XMFLOAT3(-s, s, s), color}
    };

    indices = {
        0, 1, 2, 2, 3, 0,
        1, 5, 6, 6, 2, 1,
        5, 4, 7, 7, 6, 5,
        4, 0, 3, 3, 7, 4,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };
}
