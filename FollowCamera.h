#ifndef FOLLOWCAMERA_H
#define FOLLOWCAMERA_H

#include <DirectXMath.h>

class FollowCamera {
public:
    FollowCamera(DirectX::XMFLOAT3 offset, DirectX::XMFLOAT3 targetOffset);
    void Update(const DirectX::XMFLOAT3& targetPosition, float attachedCount);
    void Rotate(float angle);
    DirectX::XMVECTOR GetForward() const;
    DirectX::XMVECTOR GetRight() const;

    DirectX::XMMATRIX GetViewMatrix() const { return viewMatrix; }
    DirectX::XMMATRIX GetProjectionMatrix() const { return projMatrix; }
    DirectX::XMMATRIX GetViewProjection() const { return viewMatrix * projMatrix; }

private:
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 target;
    DirectX::XMFLOAT3 offset;
    float yaw = 0.0f;
    float baseDistance;
    DirectX::XMMATRIX viewMatrix;
    DirectX::XMMATRIX projMatrix;
};

#endif