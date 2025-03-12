#include "FollowCamera.h"
#include <iostream>

FollowCamera::FollowCamera(CelestialBody* followedObject) : followedObject(followedObject) {
    offset = XMFLOAT3(0.0f, 5.0f, -5.0f); // Задаём смещение: над и сзади объекта
    if (followedObject) {
        position = followedObject->position; // Изначально ставим камеру на позицию объекта
        target = followedObject->position;   // Смотрим на объект
    }
    std::cout << "FollowCamera constructed with offset (0, 5, -5)" << std::endl;
}

void FollowCamera::Update(float deltaTime) {
    if (followedObject) {
        // Получаем текущую позицию объекта
        XMFLOAT3 objectPos = followedObject->position;
        
        // Обновляем позицию камеры: позиция объекта + смещение
        position.x = objectPos.x + offset.x;
        position.y = objectPos.y + offset.y;
        position.z = objectPos.z + offset.z;
        
        // Камера смотрит на объект
        target = objectPos;

        std::cout << "FollowCamera updated. Position: (" << position.x << ", " << position.y << ", " << position.z 
                  << "), Target: (" << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    }
}

XMMATRIX FollowCamera::GetViewMatrix() const {
    std::cout << "Getting FollowCamera view matrix..." << std::endl;
    return XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMVectorSet(0, 1, 0, 0));
}