#ifndef FOLLOW_CAMERA_H
#define FOLLOW_CAMERA_H

#include "Camera.h"
#include "CelestialBody.h"
#include <DirectXMath.h>
#include <iostream>

using namespace DirectX;

class FollowCamera : public Camera {
public:
    FollowCamera(CelestialBody* targetBody) : targetBody(targetBody) {
        position = targetBody->position;
        fov = XM_PIDIV4; // Угол обзора по умолчанию — 45 градусов
        up = XMFLOAT3(0, 1, 0);
        std::cout << "=== FollowCamera constructed, following target at (" << position.x << ", " << position.y << ", " << position.z << ") ===" << std::endl;
    }

    void Update(float deltaTime) override {
        if (targetBody) {
            // Позиция камеры совпадает с позицией объекта
            position = targetBody->position;

            // Получаем матрицу вращения объекта из кватерниона
            XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&targetBody->rotationQuaternion));

            // Направление взгляда — вдоль оси Z объекта
            XMVECTOR forward = XMVector3Transform(XMVectorSet(0, 0, 1, 0), rotationMatrix);
            XMVECTOR targetPos =XMVectorAdd( XMLoadFloat3(&position) , forward);
            XMStoreFloat3(&this->target, targetPos); // Сохраняем позицию цели

            // Вектор "вверх" также зависит от вращения объекта
            XMVECTOR upVector = XMVector3Transform(XMVectorSet(0, 1, 0, 0), rotationMatrix);
            XMStoreFloat3(&this->up, upVector);

            // Выводим значения для отладки
            std::cout << "FollowCamera updated. Position: (" << position.x << ", " << position.y << ", " << position.z
                      << "), Target: (" << this->target.x << ", " << this->target.y << ", " << this->target.z << ")" << std::endl;
        }
    }

    XMMATRIX GetViewMatrix() const override {
        std::cout << "Getting FollowCamera view matrix..." << std::endl;
        return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&this->target), XMLoadFloat3(&up));
    }

private:
    CelestialBody* targetBody; // Указатель на объект, за которым следует камера
    XMFLOAT3 up;              // Вектор "вверх" для ориентации камеры
};

#endif