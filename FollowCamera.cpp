#include "FollowCamera.h"
using namespace DirectX;

FollowCamera::FollowCamera(XMFLOAT3 offset, XMFLOAT3 targetOffset) : offset(offset) {
    position = XMFLOAT3(0, 0, 0);
    target = XMFLOAT3(0, 0, 0);
    viewMatrix = XMMatrixIdentity();
    projMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, 800.0f / 600.0f, 0.1f, 1000.0f);

    // Вычисляем базовое расстояние как длину вектора offset
    baseDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&offset)));
}

void FollowCamera::Update(const XMFLOAT3& targetPosition, float attachedCount) {
    target = targetPosition;
    XMVECTOR targetVec = XMLoadFloat3(&target);

    // Вычисляем новое расстояние на основе количества прикреплённых объектов
    float distance = baseDistance + attachedCount * 0.5f; // Коэффициент 0.5 можно настроить

    // Нормализуем offset и масштабируем его на новое расстояние
    XMVECTOR offsetVec = XMLoadFloat3(&offset);
    offsetVec = XMVector3Normalize(offsetVec); // Единичный вектор направления
    offsetVec = XMVectorScale(offsetVec, distance); // Увеличиваем длину

    // Применяем поворот к смещению камеры
    XMMATRIX rotation = XMMatrixRotationY(yaw);
    offsetVec = XMVector3Transform(offsetVec, rotation);

    // Вычисляем новую позицию камеры
    XMVECTOR newPos = targetVec + offsetVec;
    XMStoreFloat3(&position, newPos);

    // Обновляем матрицу вида
    viewMatrix = XMMatrixLookAtLH(newPos, targetVec, XMVectorSet(0, 1, 0, 0));
}

void FollowCamera::Rotate(float angle) {
    yaw += angle;
    if (yaw > XM_2PI) yaw -= XM_2PI;
    if (yaw < -XM_2PI) yaw += XM_2PI;
}

XMVECTOR FollowCamera::GetForward() const {
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR targ = XMLoadFloat3(&target);
    XMVECTOR forward = XMVectorSubtract(targ, pos);
    forward = XMVector3Normalize(forward);
    forward = XMVectorSetY(forward, 0); // Проекция на плоскость XZ
    return XMVector3Normalize(forward);
}

XMVECTOR FollowCamera::GetRight() const {
    XMVECTOR forward = GetForward();
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR right = XMVector3Cross(up, forward);
    return XMVector3Normalize(right);
}