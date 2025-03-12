#ifndef GRID_H
#define GRID_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class Grid {
public:
    Grid(ID3D11Device* device, float size, int divisions);
    ~Grid();
    void Draw(ID3D11DeviceContext* context);

private:
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    UINT indexCount;
};

#endif