#include "Render.h"
#include "CelestialBody.h"
#include "Ground.h"
#include "FollowCamera.h"
#include <windows.h>
#include <memory>
#include <vector>
#include "Logger.h"
#include <DirectXMath.h>

int main() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"WindowClass";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, L"WindowClass", L"DirectX Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    Render render(hwnd);
    if (!render.Initialize()) {
        logger << "[main] Ошибка инициализации рендера" << std::endl;
        return -1;
    }

    std::unique_ptr<Ground> ground = std::make_unique<Ground>(render.GetDevice(), "Textures/ground.obj");

    std::vector<std::unique_ptr<CelestialBody>> bodies;
    // Katamari (основной объект)
    bodies.push_back(std::make_unique<CelestialBody>(
        render.GetDevice(),
        "Textures/soccer_ball.obj",
        DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
        DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.0f,
        true,
        DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f) // Легкое белое свечение
    ));

    // Дополнительные мячи для налипания
    bodies.push_back(std::make_unique<CelestialBody>(
        render.GetDevice(),
        "Textures/soccer_ball.obj",
        DirectX::XMFLOAT3(5.0f, 1.0f, 5.0f),
        DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
        0.5f,
        true,
        DirectX::XMFLOAT3(0.3f, 0.0f, 0.0f) // Красное свечение
    ));
    bodies.push_back(std::make_unique<CelestialBody>(
        render.GetDevice(),
        "Textures/soccer_ball.obj",
        DirectX::XMFLOAT3(-5.0f, 1.0f, -5.0f),
        DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
        0.5f,
        true,
        DirectX::XMFLOAT3(0.0f, 0.3f, 0.0f) // Зеленое свечение
    ));
    bodies.push_back(std::make_unique<CelestialBody>(
        render.GetDevice(),
        "Textures/soccer_ball.obj",
        DirectX::XMFLOAT3(3.0f, 1.0f, -3.0f),
        DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
        0.5f,
        true,
        DirectX::XMFLOAT3(0.0f, 0.0f, 0.3f) // Синее свечение
    ));
    bodies.push_back(std::make_unique<CelestialBody>(
        render.GetDevice(),
        "Textures/soccer_ball.obj",
        DirectX::XMFLOAT3(-3.0f, 1.0f, 4.0f),
        DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
        0.5f,
        true,
        DirectX::XMFLOAT3(0.2f, 0.2f, 0.0f) // Желтое свечение
    ));

    DirectX::XMFLOAT3 camPos(0.0f, 5.0f, -10.0f);
    DirectX::XMFLOAT3 target(0.0f, 0.0f, 0.0f);
    FollowCamera camera(camPos, target);

    CelestialBody* katamari = bodies[0].get();
    float deltaTime = 1.0f / 60.0f;
    DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            if (GetAsyncKeyState('W') & 0x8000) {
                velocity = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), deltaTime * 2.0f);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionMultiply(currentRotation, deltaRotation));
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                velocity = DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), deltaTime * 2.0f);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionMultiply(currentRotation, deltaRotation));
            }
            if (GetAsyncKeyState('A') & 0x8000) {
                velocity = DirectX::XMVectorSet(-5.0f, 0.0f, 0.0f, 0.0f);
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), deltaTime * 2.0f);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionMultiply(currentRotation, deltaRotation));
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                velocity = DirectX::XMVectorSet(5.0f, 0.0f, 0.0f, 0.0f);
                DirectX::XMVECTOR deltaRotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), deltaTime * 2.0f);
                DirectX::XMVECTOR currentRotation = DirectX::XMLoadFloat4(&katamari->rotation);
                DirectX::XMStoreFloat4(&katamari->rotation, DirectX::XMQuaternionMultiply(currentRotation, deltaRotation));
            }

            katamari->UpdatePosition(velocity, deltaTime);

            for (auto& obj : bodies) {
                if (!obj->parent) {
                    DirectX::XMVECTOR attachmentPoint;
                    const CelestialBody* collidedBody = katamari->CheckCollision(obj.get(), attachmentPoint);
                    if (collidedBody) {
                        logger << "[main] Столкновение обнаружено, прикрепляем объект" << std::endl;
                        const_cast<CelestialBody*>(katamari)->AttachChild(obj.get());
                    }
                }
            }

            for (const auto& body : bodies) {
                body->Update();
            }

            camera.Update(katamari->GetPosition(), static_cast<float>(katamari->GetChildren().size()));
            DirectX::XMMATRIX viewProj = camera.GetViewProjMatrix();
            DirectX::XMFLOAT3 cameraPos = camera.GetPosition(); // Теперь метод GetPosition доступен

            render.RenderScene(bodies, ground.get(), viewProj, cameraPos);

            if (!(GetAsyncKeyState('W') & 0x8000) && !(GetAsyncKeyState('S') & 0x8000) &&
                !(GetAsyncKeyState('A') & 0x8000) && !(GetAsyncKeyState('D') & 0x8000)) {
                velocity = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            }
        }
    }

    logger << "[main] Программа завершена" << std::endl;
    return 0;
}