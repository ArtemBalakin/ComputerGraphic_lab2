#include "CelestialBody.h"
#include <cmath>
#include <iostream>

CelestialBody::CelestialBody(ID3D11Device *device, bool isSphere, CelestialBody *parent, float scale, XMFLOAT3 color,
                             float orbitInclination, float mass)
    : parent(parent), orbitSpeed(0), rotationSpeed(0), orbitRadius(0), scale(scale), color(color),
      vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0), totalOrbitAngle(0.0f),
      orbitInclination(orbitInclination), mass(mass) {
    //std::cout << "Creating CelestialBody with mass: " << mass << std::endl;
    position = XMFLOAT3(0, 0, 0);
    velocity = XMFLOAT3(0, 0, 0);
    rotationQuaternion = XMFLOAT4(0, 0, 0, 1);

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;
    if (isSphere) CreateSphere(1.0f, 20, 20, vertices, indices);
    else CreateCube(1.0f, vertices, indices);

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

void CelestialBody::Update(float deltaTime, const std::vector<CelestialBody*>& allBodies) {
    const float G = 5.0f;

    XMVECTOR accel = XMVectorZero(); // Ускорение

    for (const auto* other : allBodies) {
        if (other == this) continue; // Пропускаем само тело

        XMVECTOR r = XMVectorSubtract(XMLoadFloat3(&other->position), XMLoadFloat3(&position));
        float distance = XMVectorGetX(XMVector3Length(r));
        if (distance < 0.1f) distance = 0.1f; // Избегаем деления на ноль

        // F = G * m1 * m2 / r^2
        float forceMagnitude = G * mass * other->mass / (distance * distance);
        XMVECTOR forceDir = XMVector3Normalize(r);
        XMVECTOR force = XMVectorScale(forceDir, forceMagnitude);

        // a = F / m
        accel = XMVectorAdd(accel, XMVectorScale(force, 1.0f / mass));
    }

    // Обновляем скорость: v = v0 + a * dt
    XMVECTOR vel = XMLoadFloat3(&velocity);
    vel = XMVectorAdd(vel, XMVectorScale(accel, deltaTime));
    XMStoreFloat3(&velocity, vel);

    // Обновляем позицию: p = p0 + v * dt
    XMVECTOR pos = XMLoadFloat3(&position);
    pos = XMVectorAdd(pos, XMVectorScale(vel, deltaTime));
    XMStoreFloat3(&position, pos);

    // Вращение вокруг своей оси
    float rotationAngle = rotationSpeed * deltaTime;
    XMVECTOR rotation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), rotationAngle);
    XMVECTOR currentQuat = XMLoadFloat4(&rotationQuaternion);
    XMVECTOR newQuat = XMQuaternionMultiply(currentQuat, rotation);
    newQuat = XMQuaternionNormalize(newQuat);
    XMStoreFloat4(&rotationQuaternion, newQuat);

    //std::cout << "Body at (" << position.x << ", " << position.y << ", " << position.z
   //           << "), velocity (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
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
            v.color =XMFLOAT3(sin(phi), cos(theta), sin(phi));
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