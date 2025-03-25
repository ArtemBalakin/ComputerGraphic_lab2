#include <Windows.h>
#include <iostream>
#include <vector>
#include <memory>
#include "CelestialBody.h"
#include "FollowCamera.h"
#include "Render.h"
#include "Window.h"
#include "Grid.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    Window window;
    if (!window.Initialize(hInstance, 800, 600, L"Katamari Simulation")) {
        return -1;
    }

    Render render;
    if (!render.Initialize(window.GetHWND(), 800, 600)) {
        return -1;
    }

    Grid grid(render.device, 100.0f, 50);

    std::vector<std::unique_ptr<CelestialBody>> bodies;
    bodies.push_back(std::make_unique<CelestialBody>(0.0f, 1.0f, 0.0f, 1.0f, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));

    for (int i = 0; i < 200; ++i) {
        float x = (rand() % 1000 - 500) * 0.1f;
        float z = (rand() % 1000 - 500) * 0.1f;
        float r = (rand() % 100) / 100.0f;
        float g = (rand() % 100) / 100.0f;
        float b = (rand() % 100) / 100.0f;
        float scale = 0.2f + (rand() % 50) / 100.0f;
        bodies.push_back(std::make_unique<CelestialBody>(x, scale, z, scale, DirectX::XMFLOAT4(r, g, b, 1.0f)));
    }

    FollowCamera camera(DirectX::XMFLOAT3(0, 5, -10), DirectX::XMFLOAT3(0, 0, 0));

    MSG msg = {};
    DirectX::XMVECTOR velocity = DirectX::XMVectorZero();
    const float maxSpeed = 5.0f;
    const float acceleration = 0.8f;
    const float friction = 0.02f;
    const float rotationSpeed = 1.0f;

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            float deltaTime = 0.016f;

            CelestialBody* katamari = bodies[0].get();
            DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&katamari->position);
            DirectX::XMVECTOR forward = camera.GetForward();
            DirectX::XMVECTOR right = camera.GetRight();
            DirectX::XMVECTOR targetVelocity = DirectX::XMVectorZero();

            if (GetAsyncKeyState('W') & 0x8000) {
                targetVelocity = DirectX::XMVectorAdd(targetVelocity, DirectX::XMVectorScale(forward, maxSpeed));
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                targetVelocity = DirectX::XMVectorSubtract(targetVelocity, DirectX::XMVectorScale(forward, maxSpeed));
            }
            if (GetAsyncKeyState('A') & 0x8000) {
                targetVelocity = DirectX::XMVectorSubtract(targetVelocity, DirectX::XMVectorScale(right, maxSpeed));
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                targetVelocity = DirectX::XMVectorAdd(targetVelocity, DirectX::XMVectorScale(right, maxSpeed));
            }

            if (GetAsyncKeyState('Q') & 0x8000) {
                camera.Rotate(rotationSpeed * deltaTime);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), rotationSpeed * deltaTime);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionMultiply(currentRotation, deltaRotation));
            }
            if (GetAsyncKeyState('E') & 0x8000) {
                camera.Rotate(-rotationSpeed * deltaTime);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), -rotationSpeed * deltaTime);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionMultiply(currentRotation, deltaRotation));
            }

            velocity = DirectX::XMVectorLerp(velocity, targetVelocity, acceleration);
            velocity = DirectX::XMVectorScale(velocity, 1.0f - friction);
            DirectX::XMVECTOR movement = DirectX::XMVectorScale(velocity, deltaTime);
            pos = DirectX::XMVectorAdd(pos, movement);

            float newY = DirectX::XMVectorGetY(pos);
            if (newY < katamari->scale) {
                newY = katamari->scale;
                velocity = DirectX::XMVectorSetY(velocity, 0);
            }
            DirectX::XMStoreFloat3(&katamari->position, DirectX::XMVectorSetY(pos, newY));

            if (DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity)) > 0.001f) {
                DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(velocity);
                DirectX::XMVECTOR axis = DirectX::XMVector3Cross(DirectX::XMVectorSet(0, 1, 0, 0), direction);
                float speed = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity));
                float angle = speed * deltaTime * 2.0f;
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(axis, angle);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMVECTOR newRotation = DirectX::XMQuaternionMultiply(currentRotation, deltaRotation);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionNormalize(newRotation));
            }

            // Обновленная проверка столкновений с иерархическим прикреплением
            for (auto it = bodies.begin() + 1; it != bodies.end();) {
                CelestialBody* obj = it->get();
                if (!obj->parent) {
                    DirectX::XMVECTOR attachmentPoint;
                    const CelestialBody* collidedBody = katamari->CheckCollision(obj, attachmentPoint);
                    if (collidedBody) {
                        // Прикрепляем объект к телу, с которым произошло столкновение
                        obj->parent = const_cast<CelestialBody*>(collidedBody);
                        const_cast<CelestialBody*>(collidedBody)->attachedObjects.push_back(obj);

                        // Вычисляем мировую матрицу объекта и родителя
                        DirectX::XMMATRIX parentWorld = collidedBody->GetWorldMatrix();
                        DirectX::XMMATRIX invParentWorld = DirectX::XMMatrixInverse(nullptr, parentWorld);
                        DirectX::XMMATRIX objWorld = DirectX::XMMatrixScaling(obj->scale, obj->scale, obj->scale) *
                                                    DirectX::XMMatrixTranslation(obj->position.x, obj->position.y, obj->position.z);

                        // Вычисляем относительную трансформацию
                        obj->relativeTransform = objWorld * invParentWorld;
                        ++it;
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }

            // Обновляем все тела
            for (const auto& body : bodies) {
                body->Update();
            }

            camera.Update(bodies[0]->position, static_cast<float>(bodies[0]->attachedObjects.size()));
            render.RenderScene(&camera, bodies, &grid);
        }
    }

    render.Cleanup();
    window.Cleanup();
    return static_cast<int>(msg.wParam);
}