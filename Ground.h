#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>
#include "ModelLoader.h"
#include "Logger.h"

class Ground {
public:
    Ground(ID3D11Device* device, const std::string& modelPath);
    ~Ground();

    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj,
              DirectX::XMFLOAT3 cameraPos, float fogStart, float fogEnd, DirectX::XMFLOAT4 fogColor) const;
    bool HasTexture() const;

private:
    void InitializeBuffers(ID3D11Device* device);
    void LoadTexture(ID3D11Device* device, const std::string& texturePath);

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    std::unique_ptr<ModelLoader> modelLoader;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11ShaderResourceView* textureSRV;
};