#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>
#include <windows.h>

using namespace DirectX;

class Camera {
public:
    Camera();

    virtual void Update(float deltaTime);

    virtual XMMATRIX GetViewMatrix() const;

    XMMATRIX GetProjectionMatrix() const;

    XMFLOAT3 position;
    XMFLOAT3 target;
    float fov;
};

class FPSCamera : public Camera {
public:
    FPSCamera();

    void Update(float deltaTime) override;

    XMMATRIX GetViewMatrix() const override;

private:
    float yaw; // Угол рыскания (влево-вправо)
    float pitch; // Угол тангажа (вверх-вниз)
};

class OrbitalCamera : public Camera {
public:
    OrbitalCamera();

    void Update(float deltaTime) override;

    XMMATRIX GetViewMatrix() const override;

private:
    float angle;
    float distance;
};

#endif
