#include "CelestialBody.h"

CelestialBody::CelestialBody(float x, float y, float z, float s, DirectX::XMFLOAT4 c)
    : position{x, y, z}, scale(s), color(c), parent(nullptr) {
    rotation = {0.0f, 0.0f, 0.0f, 1.0f}; // Единичный кватернион
    relativeTransform = DirectX::XMMatrixIdentity(); // Единичная матрица
}

const CelestialBody* CelestialBody::CheckCollision(const CelestialBody* other, DirectX::XMVECTOR& attachmentPoint) const {
    DirectX::XMVECTOR thisPos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR otherPos = DirectX::XMLoadFloat3(&other->position);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(otherPos, thisPos);
    float distanceSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(diff));
    float collisionDist = scale + other->scale;

    if (distanceSq < collisionDist * collisionDist) {
        DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(diff);
        attachmentPoint = DirectX::XMVectorAdd(thisPos, DirectX::XMVectorScale(direction, scale));
        return this;
    }

    for (const auto* attached : attachedObjects) {
        const CelestialBody* collided = attached->CheckCollision(other, attachmentPoint);
        if (collided) {
            return collided;
        }
    }
    return nullptr;
}

void CelestialBody::Update() {
    if (parent) {
        // Получаем мировую матрицу родителя
        DirectX::XMMATRIX parentWorld = parent->GetWorldMatrix();
        // Вычисляем мировую матрицу объекта
        DirectX::XMMATRIX worldMatrix = relativeTransform * parentWorld;

        // Извлекаем позицию из мировой матрицы
        DirectX::XMVECTOR scaleVec, rotQuat, transVec;
        DirectX::XMMatrixDecompose(&scaleVec, &rotQuat, &transVec, worldMatrix);
        DirectX::XMStoreFloat3(&position, transVec);
    }

    // Рекурсивно обновляем все прикрепленные объекты
    for (auto* attached : attachedObjects) {
        attached->Update();
    }
}

DirectX::XMMATRIX CelestialBody::GetWorldMatrix() const {
    if (parent) {
        // Для прикрепленных объектов: relativeTransform * родительская мировая матрица
        return relativeTransform * parent->GetWorldMatrix();
    } else {
        // Для катамари: масштаб, вращение и трансляция
        DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale, scale, scale);
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
        DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
        return scaleMatrix * rotationMatrix * translationMatrix;
    }
}