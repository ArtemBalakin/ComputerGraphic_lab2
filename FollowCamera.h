#pragma once
#include <DirectXMath.h>

class FollowCamera {
public:
    FollowCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target);
    DirectX::XMMATRIX GetViewMatrix();
    DirectX::XMMATRIX GetProjMatrix();
    DirectX::XMMATRIX GetViewProjMatrix();
    void SetAspectRatio(float aspect);
    void Update(DirectX::XMFLOAT3 target, float size);

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_target;
    float m_aspectRatio = 1.333f; // 800/600 по умолчанию
};