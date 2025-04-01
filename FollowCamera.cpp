#include "FollowCamera.h"

FollowCamera::FollowCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target)
    : m_position(position), m_target(target) {
}

DirectX::XMMATRIX FollowCamera::GetViewMatrix() {
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(m_position.x, m_position.y, m_position.z, 0.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorSet(m_target.x, m_target.y, m_target.z, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    return DirectX::XMMatrixLookAtLH(pos, target, up);
}

DirectX::XMMATRIX FollowCamera::GetProjMatrix() {
    return DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, m_aspectRatio, 0.1f, 1000.0f);
}

DirectX::XMMATRIX FollowCamera::GetViewProjMatrix() {
    return GetViewMatrix() * GetProjMatrix();
}

void FollowCamera::SetAspectRatio(float aspect) {
    m_aspectRatio = aspect;
}

void FollowCamera::Update(DirectX::XMFLOAT3 target, float size) {
    m_target = target;
    float distance = 10.0f + size * 2.0f; // Увеличиваем расстояние в зависимости от размера катамари
    m_position = DirectX::XMFLOAT3(target.x, target.y + 5.0f, target.z - distance);
}