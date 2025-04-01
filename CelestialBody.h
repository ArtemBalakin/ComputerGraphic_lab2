#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "ModelLoader.h"
#include "Logger.h"

class CelestialBody {
public:
    CelestialBody(ID3D11Device* device, const std::string& modelPath, DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 color, float radius, bool useTexture);
    ~CelestialBody();
    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj) const;

    // Новые методы для исправления ошибок
    DirectX::XMFLOAT3 GetPosition() const { return position; }
    void UpdatePosition(DirectX::XMVECTOR velocity, float deltaTime);
    void Update();
    DirectX::XMMATRIX GetWorldMatrix() const;
    const CelestialBody* CheckCollision(const CelestialBody* other, DirectX::XMVECTOR& attachmentPoint) const;
    void AttachChild(CelestialBody* child);
    const std::vector<CelestialBody*>& GetChildren() const { return children; }

    // Публичное поле для вращения
    DirectX::XMFLOAT4 rotation;

    // Указатель на родителя
    CelestialBody* parent;

private:
    void InitializeBuffers(ID3D11Device* device);
    void LoadTexture(ID3D11Device* device, const std::string& texturePath);

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    float radius;
    bool useTexture;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11ShaderResourceView* textureSRV;
    std::unique_ptr<ModelLoader> modelLoader;
    std::vector<CelestialBody*> children;
    DirectX::XMMATRIX relativeTransform;
};