#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Render.h"

using namespace DirectX;

struct Vertex {
    XMFLOAT3 pos;
    XMFLOAT3 color;
};

class CelestialBody {
public:
    CelestialBody(ID3D11Device *device, bool isSphere, CelestialBody *parent, float scale, XMFLOAT3 color,
                  float orbitInclination, float mass = 1.0f);
    ~CelestialBody();

    void Update(float deltaTime, const std::vector<CelestialBody*>& allBodies);
    void Draw(ID3D11DeviceContext *context, Render &render, const XMMATRIX &view, const XMMATRIX &projection,
              ID3D11Buffer *constantBuffer);

    XMFLOAT3 position;  // Позиция тела
    XMFLOAT3 velocity;  // Вектор скорости
    float mass;         // Масса тела

public:
    CelestialBody *parent;
    float orbitSpeed, rotationSpeed, orbitRadius, scale;
    XMFLOAT3 color;
    ID3D11Buffer *vertexBuffer, *indexBuffer;
    UINT indexCount;
    float totalOrbitAngle;
    float orbitInclination;
    XMFLOAT4 rotationQuaternion;

    void CreateSphere(float radius, int slices, int stacks, std::vector<Vertex> &vertices, std::vector<UINT> &indices);
    void CreateCube(float size, std::vector<Vertex> &vertices, std::vector<UINT> &indices);
};