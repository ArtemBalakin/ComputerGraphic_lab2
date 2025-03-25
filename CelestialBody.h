#ifndef CELESTIALBODY_H
#define CELESTIALBODY_H

#include <DirectXMath.h>
#include <vector>
#include <memory>

class CelestialBody {
public:
    CelestialBody(float x, float y, float z, float s, DirectX::XMFLOAT4 c);
    ~CelestialBody() = default;

    // Проверяет столкновение и возвращает тело, с которым произошло столкновение
    const CelestialBody* CheckCollision(const CelestialBody* other, DirectX::XMVECTOR& attachmentPoint) const;

    // Обновляет позицию на основе родителя
    void Update();

    // Возвращает мировую матрицу для рендеринга
    DirectX::XMMATRIX GetWorldMatrix() const;

    // Публичные члены
    DirectX::XMFLOAT3 position;         // Мировая позиция объекта
    DirectX::XMFLOAT4 rotation;         // Кватернион вращения (для катамари)
    DirectX::XMMATRIX relativeTransform; // Относительная матрица трансформации (для прикрепленных объектов)
    DirectX::XMFLOAT4 color;            // Цвет объекта
    float scale;                        // Масштаб (радиус) объекта
    CelestialBody* parent;               // Указатель на родительский объект
    std::vector<CelestialBody*> attachedObjects; // Список прикрепленных объектов
};

#endif // CELESTIALBODY_H