#include "CelestialBody.h"
#include <cmath>
#include <iostream>

CelestialBody::CelestialBody(ID3D11Device* device, bool isSphere, CelestialBody* parent, float scale, XMFLOAT3 color)
    : parent(parent), orbitSpeed(0), rotationSpeed(0), orbitRadius(0), scale(scale), color(color), vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0), totalOrbitAngle(0.0f) {
    std::cout << "Creating CelestialBody with parent: " << parent << ", isSphere: " << isSphere << ", scale: " << scale << ", color: (" << color.x << ", " << color.y << ", " << color.z << ")" << std::endl;
    position = XMFLOAT3(0, 0, 0);
    rotationQuaternion = XMFLOAT4(0, 0, 0, 1);
    std::cout << "Initial position set to (0, 0, 0), initial quaternion set to (0, 0, 0, 1)" << std::endl;

    std::vector<Vertex> vertices;
    std::vector<UINT> indices;
    if (isSphere) {
        std::cout << "Generating sphere geometry..." << std::endl;
        CreateSphere(1.0f, 20, 20, vertices, indices);
        std::cout << "Sphere generated with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
    } else {
        std::cout << "Generating cube geometry..." << std::endl;
        CreateCube(1.0f, vertices, indices);
        std::cout << "Cube generated with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
    }

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();
    std::cout << "Creating vertex buffer with size: " << bd.ByteWidth << " bytes..." << std::endl;
    HRESULT hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create vertex buffer. Error code: " << hr << std::endl;
    } else {
        std::cout << "Vertex buffer created successfully. Pointer: " << vertexBuffer << std::endl;
    }

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * indices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices.data();
    std::cout << "Creating index buffer with size: " << bd.ByteWidth << " bytes..." << std::endl;
    hr = device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create index buffer. Error code: " << hr << std::endl;
    } else {
        std::cout << "Index buffer created successfully. Pointer: " << indexBuffer << std::endl;
    }

    indexCount = indices.size();
    std::cout << "Index count set to: " << indexCount << std::endl;
}

CelestialBody::~CelestialBody() {
    std::cout << "Destroying CelestialBody. Releasing buffers..." << std::endl;
    if (vertexBuffer) {
        vertexBuffer->Release();
        std::cout << "Vertex buffer released." << std::endl;
    }
    if (indexBuffer) {
        indexBuffer->Release();
        std::cout << "Index buffer released." << std::endl;
    }
}

void CelestialBody::Update(float deltaTime) {
    std::cout << "Updating CelestialBody with deltaTime: " << deltaTime << std::endl;
    if (parent) {
        std::cout << "Body has parent. Calculating orbit..." << std::endl;
        float orbitAngle = orbitSpeed * deltaTime;
        totalOrbitAngle += orbitAngle;

        XMVECTOR parentPos = XMLoadFloat3(&parent->position);
        XMVECTOR orbitPos = XMVectorSet(orbitRadius * cos(totalOrbitAngle), 0, orbitRadius * sin(totalOrbitAngle), 0);
        XMVECTOR newPos = XMVectorAdd(parentPos, orbitPos);
        XMStoreFloat3(&position, newPos);
        std::cout << "Orbit updated. New position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    } else {
        std::cout << "Body has no parent. Skipping orbit calculation." << std::endl;
    }

    std::cout << "Updating rotation with angle: " << rotationSpeed * deltaTime << std::endl;
    float rotationAngle = rotationSpeed * deltaTime;
    XMVECTOR rotation = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), rotationAngle);
    XMVECTOR currentQuat = XMLoadFloat4(&rotationQuaternion);
    XMVECTOR newQuat = XMQuaternionMultiply(currentQuat, rotation);
    newQuat = XMQuaternionNormalize(newQuat);
    XMStoreFloat4(&rotationQuaternion, newQuat);
    std::cout << "Rotation updated. New quaternion: (" << rotationQuaternion.x << ", " << rotationQuaternion.y << ", " << rotationQuaternion.z << ", " << rotationQuaternion.w << ")" << std::endl;

    XMFLOAT3 pos = position;
    const float maxDistance = 15.0f;
    bool clamped = false;
    if (pos.x > maxDistance) { pos.x = maxDistance; clamped = true; }
    if (pos.x < -maxDistance) { pos.x = -maxDistance; clamped = true; }
    if (pos.y > maxDistance) { pos.y = maxDistance; clamped = true; }
    if (pos.y < -maxDistance) { pos.y = -maxDistance; clamped = true; }
    if (pos.z > maxDistance) { pos.z = maxDistance; clamped = true; }
    if (pos.z < -maxDistance) { pos.z = -maxDistance; clamped = true; }
    if (clamped) {
        std::cout << "Position clamped to: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }
    position = pos;
    std::cout << "Update completed. Final position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void CelestialBody::Draw(ID3D11DeviceContext* context, Render& render, const XMMATRIX& view, const XMMATRIX& projection, ID3D11Buffer* constantBuffer) {
    std::cout << "=== Starting draw for CelestialBody at (" << position.x << ", " << position.y << ", " << position.z << ") with scale " << scale << " ===" << std::endl;

    std::cout << "Creating translation matrix..." << std::endl;
    XMMATRIX translation = XMMatrixTranslation(position.x, position.y, position.z);
    std::cout << "Creating rotation matrix from quaternion (" << rotationQuaternion.x << ", " << rotationQuaternion.y << ", " << rotationQuaternion.z << ", " << rotationQuaternion.w << ")..." << std::endl;
    XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rotationQuaternion));
    std::cout << "Creating scaling matrix with scale " << scale << "..." << std::endl;
    XMMATRIX scaling = XMMatrixScaling(scale, scale, scale);
    std::cout << "Combining world matrix (scaling * rotation * translation)..." << std::endl;
    XMMATRIX world = scaling * rotation * translation;
    std::cout << "Combining worldViewProj matrix (world * view * projection)..." << std::endl;
    XMMATRIX worldViewProj = world * view * projection;

    struct ConstantBuffer { XMMATRIX worldViewProj; };
    ConstantBuffer cb;
    std::cout << "Transposing worldViewProj matrix..." << std::endl;
    cb.worldViewProj = XMMatrixTranspose(worldViewProj);
    std::cout << "Updating constant buffer subresource..." << std::endl;
    context->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    std::cout << "Setting vertex shader constant buffer..." << std::endl;
    context->VSSetConstantBuffers(0, 1, &constantBuffer);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    std::cout << "Setting vertex buffer with stride " << stride << " and offset " << offset << "..." << std::endl;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    std::cout << "Setting index buffer..." << std::endl;
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    std::cout << "Setting primitive topology to TRIANGLELIST..." << std::endl;
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::cout << "Calling render.DrawIndexed with indexCount: " << indexCount << std::endl;
    render.DrawIndexed(indexCount);
    std::cout << "=== Draw completed successfully ===" << std::endl;
}

void CelestialBody::CreateSphere(float radius, int slices, int stacks, std::vector<Vertex>& vertices, std::vector<UINT>& indices) {
    std::cout << "Creating sphere with radius " << radius << ", slices " << slices << ", stacks " << stacks << "..." << std::endl;
    for (int i = 0; i <= stacks; ++i) {
        float phi = XM_PI * static_cast<float>(i) / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * XM_PI * static_cast<float>(j) / slices;
            Vertex v;
            v.pos.x = radius * sin(phi) * cos(theta);
            v.pos.y = radius * cos(phi);
            v.pos.z = radius * sin(phi) * sin(theta);
            v.color = color; // Используем заданный цвет
            vertices.push_back(v);
            std::cout << "Added vertex " << vertices.size() - 1 << " at (" << v.pos.x << ", " << v.pos.y << ", " << v.pos.z << ")" << std::endl;
        }
    }

    std::cout << "Generating sphere indices..." << std::endl;
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
            std::cout << "Added indices: " << first << ", " << second << ", " << (first + 1) << ", " << second << ", " << (second + 1) << ", " << (first + 1) << std::endl;
        }
    }
    std::cout << "Sphere creation completed with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
}

void CelestialBody::CreateCube(float size, std::vector<Vertex>& vertices, std::vector<UINT>& indices) {
    std::cout << "Creating cube with size " << size << "..." << std::endl;
    float s = size / 2.0f;
    vertices = {
        { XMFLOAT3(-s, -s, -s), color },
        { XMFLOAT3(s, -s, -s), color },
        { XMFLOAT3(s, s, -s), color },
        { XMFLOAT3(-s, s, -s), color },
        { XMFLOAT3(-s, -s, s), color },
        { XMFLOAT3(s, -s, s), color },
        { XMFLOAT3(s, s, s), color },
        { XMFLOAT3(-s, s, s), color }
    };
    std::cout << "Added 8 vertices for cube with color (" << color.x << ", " << color.y << ", " << color.z << ")." << std::endl;

    indices = {
        0, 1, 2, 2, 3, 0,
        1, 5, 6, 6, 2, 1,
        5, 4, 7, 7, 6, 5,
        4, 0, 3, 3, 7, 4,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };
    std::cout << "Added 36 indices for cube (6 faces)." << std::endl;
    std::cout << "Cube creation completed with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
}