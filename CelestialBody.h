#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>
#include <memory>

#include "ModelLoader.h"

class CelestialBody {
public:
    CelestialBody(ID3D11Device* device, const std::string& modelPath, DirectX::XMFLOAT3 pos,
                  DirectX::XMFLOAT4 col, float rad, bool useTex, DirectX::XMFLOAT3 emissiveCol);
    ~CelestialBody();

    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj,
              DirectX::XMFLOAT3 cameraPos) const;
    void UpdatePosition(DirectX::XMVECTOR velocity, float deltaTime);
    void Update();
    DirectX::XMMATRIX GetWorldMatrix() const;
    const CelestialBody* CheckCollision(const CelestialBody* other, DirectX::XMVECTOR& attachmentPoint) const;
    void AttachChild(CelestialBody* child);
    DirectX::XMFLOAT3 GetPosition() const { return position; }
    const std::vector<CelestialBody*>& GetChildren() const { return children; }


    void InitializeBuffers(ID3D11Device* device);
    void LoadTexture(ID3D11Device* device, const std::string& texturePath);

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    float radius;
    bool useTexture;
    DirectX::XMFLOAT3 emissiveColor;
    DirectX::XMFLOAT4 rotation;
    DirectX::XMMATRIX relativeTransform;

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11ShaderResourceView* textureSRV;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::unique_ptr<ModelLoader> modelLoader;

    CelestialBody* parent;
    std::vector<CelestialBody*> children;
};