#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

class Grid {
public:
    Grid(ID3D11Device* device, float size, int divisions);
    ~Grid();
    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj) const;

private:
    float size;
    int divisions;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;

    void InitializeBuffers(ID3D11Device* device);
};