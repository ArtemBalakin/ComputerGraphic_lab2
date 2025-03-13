#include "Camera.h"
#include <windows.h>
#include <iostream>

Camera::Camera() : fov(XM_PIDIV4), position(XMFLOAT3(0, 0, -5)), target(XMFLOAT3(0, 0, 0)) {
    //std::cout << "=== Camera constructed ===" << std::endl;
    //std::cout << "Initial position: (" << position.x << ", " << position.y << ", " << position.z << "), Target: (" <<
  //          target.x << ", " << target.y << ", " << target.z << "), FOV: " << fov << std::endl;
}

void Camera::Update(float deltaTime) {
    //std::cout << "Updating base camera with deltaTime: " << deltaTime << std::endl;
}

XMMATRIX Camera::GetViewMatrix() const {
    //std::cout << "Getting view matrix for position (" << position.x << ", " << position.y << ", " << position.z <<
     //       ") targeting (" << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}

XMMATRIX Camera::GetProjectionMatrix() const {
    //std::cout << "Getting projection matrix with FOV: " << fov << ", Aspect: 800/600, Near: 0.1, Far: 100" << std::endl;
    return XMMatrixPerspectiveFovLH(fov, 800.0f / 600.0f, 0.1f, 100.0f);
}

FPSCamera::FPSCamera() {
    //std::cout << "=== FPSCamera constructed ===" << std::endl;
    position = XMFLOAT3(0, 0, -5);
    target = XMFLOAT3(0, 0, 0);
    yaw = 0.0f; // Начальный угол рыскания
    pitch = 0.0f; // Начальный угол тангажа
    //std::cout << "Initial position set to (0, 0, -5), Target set to (0, 0, 0), Yaw: " << yaw << ", Pitch: " << pitch <<
     //       std::endl;
}

void FPSCamera::Update(float deltaTime) {
    //std::cout << "=== Updating FPSCamera ===" << std::endl;
    //std::cout << "Current position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    float speed = 500.0f * deltaTime;
    float rotationSpeed = 10.0f * deltaTime; // Скорость поворота

    // Движение
    if (GetAsyncKeyState('W') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        forward = XMVector3Normalize(forward);
        position.x += XMVectorGetX(forward) * speed;
        position.y += XMVectorGetY(forward) * speed;
        position.z += XMVectorGetZ(forward) * speed;
        //std::cout << "Moved forward with speed " << speed << std::endl;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        forward = XMVector3Normalize(forward);
        position.x -= XMVectorGetX(forward) * speed;
        position.y -= XMVectorGetY(forward) * speed;
        position.z -= XMVectorGetZ(forward) * speed;
        //std::cout << "Moved backward with speed " << speed << std::endl;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        XMVECTOR right = XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward);
        right = XMVector3Normalize(right);
        position.x -= XMVectorGetX(right) * speed;
        position.z -= XMVectorGetZ(right) * speed;
        //std::cout << "Moved left with speed " << speed << std::endl;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        XMVECTOR forward = XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&position));
        XMVECTOR right = XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward);
        right = XMVector3Normalize(right);
        position.x += XMVectorGetX(right) * speed;
        position.z += XMVectorGetZ(right) * speed;
        //std::cout << "Moved right with speed " << speed << std::endl;
    }

    // Поворот
    if (GetAsyncKeyState('Q') & 0x8000) {
        yaw += rotationSpeed;
        //std::cout << "Rotated left. New yaw: " << yaw << std::endl;
    }
    if (GetAsyncKeyState('E') & 0x8000) {
        yaw -= rotationSpeed;
        //std::cout << "Rotated right. New yaw: " << yaw << std::endl;
    }
    if (GetAsyncKeyState('R') & 0x8000) {
        pitch -= rotationSpeed;
        if (pitch < -XM_PIDIV2 + 0.1f) pitch = -XM_PIDIV2 + 0.1f; // Ограничение вниз
        //std::cout << "Rotated up. New pitch: " << pitch << std::endl;
    }
    if (GetAsyncKeyState('F') & 0x8000) {
        pitch += rotationSpeed;
        if (pitch > XM_PIDIV2 - 0.1f) pitch = XM_PIDIV2 - 0.1f; // Ограничение вверх
        //std::cout << "Rotated down. New pitch: " << pitch << std::endl;
    }

    // Обновление target на основе yaw и pitch
    XMVECTOR direction = XMVectorSet(
        cos(pitch) * sin(yaw),
        sin(pitch),
        cos(pitch) * cos(yaw),
        0
    );
    direction = XMVector3Normalize(direction);
    XMVECTOR newTarget = XMVectorAdd(XMLoadFloat3(&position), direction);
    XMStoreFloat3(&target, newTarget);
    //std::cout << "Target updated to: (" << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    //std::cout << "FPSCamera update completed." << std::endl;
}

XMMATRIX FPSCamera::GetViewMatrix() const {
    //std::cout << "Getting FPS camera view matrix..." << std::endl;
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}

OrbitalCamera::OrbitalCamera() : angle(0), distance(5.0f) {
    //std::cout << "=== OrbitalCamera constructed ===" << std::endl;
    position = XMFLOAT3(0, 0, -distance);
    target = XMFLOAT3(0, 0, 0);
    //std::cout << "Initial position set to (0, 0, -5), Target set to (0, 0, 0), Distance: " << distance << std::endl;
}

void OrbitalCamera::Update(float deltaTime) {
    //std::cout << "=== Updating OrbitalCamera ===" << std::endl;
    //std::cout << "Current position: (" << position.x << ", " << position.y << ", " << position.z << "), Angle: " <<
       //     angle << ", Distance: " << distance << std::endl;
    angle += deltaTime;
    position.x = distance * sin(angle);
    position.z = distance * cos(angle);
    //std::cout << "Orbit calculated. New position: (" << position.x << ", " << position.y << ", " << position.z << ")" <<
      //      std::endl;
    if (GetAsyncKeyState('W') & 0x8000) {
        distance -= 1.0f * deltaTime;
        //std::cout << "Zooming in. New distance: " << distance << std::endl;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        distance += 1.0f * deltaTime;
        //std::cout << "Zooming out. New distance: " << distance << std::endl;
    }
    if (distance < 1.0f) distance = 1.0f;
    //std::cout << "Distance clamped to: " << distance << std::endl;
    //std::cout << "OrbitalCamera update completed." << std::endl;
}

XMMATRIX OrbitalCamera::GetViewMatrix() const {
    //std::cout << "Getting Orbital camera view matrix..." << std::endl;
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}
