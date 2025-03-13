#ifndef FOLLOW_CAMERA_H
#define FOLLOW_CAMERA_H

#include "Camera.h"
#include "CelestialBody.h"
#include <DirectXMath.h>

using namespace DirectX;

class FollowCamera : public Camera {
public:
    FollowCamera(CelestialBody *followedObject);

    void Update(float deltaTime) override;

    XMMATRIX GetViewMatrix() const override;

private:
    CelestialBody *followedObject; // Указатель на объект, за которым следим
    XMFLOAT3 offset; // Смещение камеры относительно объекта
};

#endif
