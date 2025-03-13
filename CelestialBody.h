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
                  float orbitInclination = 0.0f);

    ~CelestialBody();

    void Update(float deltaTime);

    void Draw(ID3D11DeviceContext *context, Render &render, const XMMATRIX &view, const XMMATRIX &projection,
              ID3D11Buffer *constantBuffer);

    XMFLOAT3 position;

public:
    float orbitSpeed;
    float rotationSpeed;
    float orbitRadius;

private:
    CelestialBody *parent;
    float scale;
    XMFLOAT3 color;
    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;
    UINT indexCount;
    XMFLOAT4 rotationQuaternion;
    float totalOrbitAngle;
    float orbitInclination;

    void CreateSphere(float radius, int slices, int stacks, std::vector<Vertex> &vertices, std::vector<UINT> &indices);

    void CreateCube(float size, std::vector<Vertex> &vertices, std::vector<UINT> &indices);
};
