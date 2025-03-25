#include "Camera.h"

Camera::Camera() : fov(XM_PIDIV4), position(XMFLOAT3(0, 0, -5)), target(XMFLOAT3(0, 0, 0)) {}

void Camera::Update(float deltaTime) {}

XMMATRIX Camera::GetViewMatrix() const {
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}

XMMATRIX Camera::GetProjectionMatrix() const {
    return XMMatrixPerspectiveFovLH(fov, 1300.0f / 1000.0f, 0.1f, 1000.0f);
}

FPSCamera::FPSCamera() {
    position = XMFLOAT3(0, 0, -5);
    target = XMFLOAT3(0, 0, 0);
    yaw = 0.0f;
    pitch = 0.0f;
}

void FPSCamera::Update(float deltaTime) {
    float speed = 500.0f * deltaTime;
    float rotationSpeed = 10.0f * deltaTime;

    if (GetAsyncKeyState('W') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        forward = XMVector3Normalize(forward);
        position.x += XMVectorGetX(forward) * speed;
        position.y += XMVectorGetY(forward) * speed;
        position.z += XMVectorGetZ(forward) * speed;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        forward = XMVector3Normalize(forward);
        position.x -= XMVectorGetX(forward) * speed;
        position.y -= XMVectorGetY(forward) * speed;
        position.z -= XMVectorGetZ(forward) * speed;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        XMVECTOR right = XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward);
        right = XMVector3Normalize(right);
        position.x -= XMVectorGetX(right) * speed;
        position.z -= XMVectorGetZ(right) * speed;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        XMVECTOR right = XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward);
        right = XMVector3Normalize(right);
        position.x += XMVectorGetX(right) * speed;
        position.z += XMVectorGetZ(right) * speed;
    }

    if (GetAsyncKeyState('Q') & 0x8000) yaw += rotationSpeed;
    if (GetAsyncKeyState('E') & 0x8000) yaw -= rotationSpeed;
    if (GetAsyncKeyState('R') & 0x8000) {
        pitch -= rotationSpeed;
        if (pitch < -XM_PIDIV2 + 0.1f) pitch = -XM_PIDIV2 + 0.1f;
    }
    if (GetAsyncKeyState('F') & 0x8000) {
        pitch += rotationSpeed;
        if (pitch > XM_PIDIV2 - 0.1f) pitch = XM_PIDIV2 - 0.1f;
    }

    XMVECTOR direction = XMVectorSet(cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw), 0);
    direction = XMVector3Normalize(direction);
    XMVECTOR newTarget = XMVectorAdd(XMLoadFloat3(&position), direction);
    XMStoreFloat3(&target, newTarget);
}

XMMATRIX FPSCamera::GetViewMatrix() const {
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}

OrbitalCamera::OrbitalCamera() : angle(0), distance(5.0f) {
    position = XMFLOAT3(0, 0, -distance);
    target = XMFLOAT3(0, 0, 0);
}

void OrbitalCamera::Update(float deltaTime) {
    angle += deltaTime;
    position.x = distance * sin(angle);
    position.z = distance * cos(angle);
    if (GetAsyncKeyState('W') & 0x8000) distance -= 1.0f * deltaTime;
    if (GetAsyncKeyState('S') & 0x8000) distance += 1.0f * deltaTime;
    if (distance < 1.0f) distance = 1.0f;
}

XMMATRIX OrbitalCamera::GetViewMatrix() const {
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}