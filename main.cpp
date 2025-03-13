#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <windows.h>
#include "Render.h"
#include "Window.h"
#include "CelestialBody.h"
#include "Camera.h"
#include "FollowCamera.h"
#include <iostream>
#include "Grid.h"

using namespace DirectX;

struct ConstantBuffer {
    XMMATRIX worldViewProj;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    std::cout << "=== Starting DirectX Gravitational Simulation ===" << std::endl;

    Window window(800, 600, L"DirectX Gravitational Simulation");
    if (!window.Initialize()) {
        std::cout << "ERROR: Failed to initialize window." << std::endl;
        return -1;
    }

    Render render;
    if (!render.Initialize(window.GetHandle(), 800, 600)) {
        std::cout << "ERROR: Failed to initialize render system." << std::endl;
        return -1;
    }

    ID3D11Device *device = render.GetDevice();
    ID3D11DeviceContext *context = render.GetContext();

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ID3D11Buffer *constantBuffer = nullptr;
    HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create constant buffer." << std::endl;
        return -1;
    }

    // Создаём три планеты с расстояниями 20 и умеренными массами
    std::vector<std::unique_ptr<CelestialBody>> bodies;


    bodies.push_back(std::make_unique<CelestialBody>(device, true, nullptr, 1.5f, XMFLOAT3(1.0f, 0.0f, 0.0f), 0.0f, 100.0f));
    bodies[0]->position = XMFLOAT3(0, 0, 0);
    bodies[0]->velocity = XMFLOAT3(0, 10, 0);
    bodies[0]->rotationSpeed = 100.5f;
    std::cout << "Planet 1: Mass 100, Position (0, 0, 0), Velocity (0, 0, 0), Color Red" << std::endl;

    bodies.push_back(std::make_unique<CelestialBody>(device, true, nullptr, 1.0f, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f, 500.0f));
    bodies[1]->position = XMFLOAT3(20, 0, 0);
    bodies[1]->velocity = XMFLOAT3(0, -2.0f, 0);
    bodies[1]->rotationSpeed = 100.7f;
    std::cout << "Planet 2: Mass 50, Position (20, 0, 0), Velocity (0, 2.0, 0), Color Green" << std::endl;

    bodies.push_back(std::make_unique<CelestialBody>(device, true, nullptr, 0.8f, XMFLOAT3(0.0f, 0.0f, 1.0f), 0.0f, 30.0f));
    bodies[2]->position = XMFLOAT3(10, 10, 0); // Расстояние 20
    bodies[2]->velocity = XMFLOAT3(-2.0f, -10, 0);
    bodies[2]->rotationSpeed = 0.9f;
    std::cout << "Planet 3: Mass 30, Position (0, 20, 0), Velocity (-2.0, 0, 0), Color Blue" << std::endl;

    Grid grid(device, 200.0f, 100);

    std::vector<std::unique_ptr<Camera>> cameras;
    cameras.emplace_back(std::make_unique<FPSCamera>());
    cameras.emplace_back(std::make_unique<OrbitalCamera>());
    cameras.emplace_back(std::make_unique<FollowCamera>(bodies[1].get()));
    int currentCamera = 0;
    cameras[0]->position = XMFLOAT3(0, 2.0f, -40.0f);
    cameras[1]->position = XMFLOAT3(0, 2.0f, -40.0f);
    cameras[1]->target = XMFLOAT3(0, 0, 0);
    cameras[2]->position = bodies[1]->position;

    LARGE_INTEGER frequency, lastTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);

    window.Show();

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float deltaTime = (currentTime.QuadPart - lastTime.QuadPart) / (float)frequency.QuadPart;
            lastTime = currentTime;

            if (GetAsyncKeyState('T') & 0x8000) {
                currentCamera = (currentCamera + 1) % cameras.size();
                Sleep(200);
            }
            if (GetAsyncKeyState('3') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV4;
            if (GetAsyncKeyState('4') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV2;
            if (GetAsyncKeyState('5') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV4 / 2;

            // Обновляем все тела с учётом гравитации
            std::vector<CelestialBody*> bodyPtrs;
            for (auto& body : bodies) bodyPtrs.push_back(body.get());
            for (auto& body : bodies) body->Update(deltaTime, bodyPtrs);

            cameras[currentCamera]->Update(deltaTime);

            render.BeginFrame();

            for (size_t i = 0; i < bodies.size(); ++i) {
                bodies[i]->Draw(context, render, cameras[currentCamera]->GetViewMatrix(),
                                cameras[currentCamera]->GetProjectionMatrix(), constantBuffer);
            }
            render.EndFrame();
        }
    }

    if (constantBuffer) constantBuffer->Release();
    render.Cleanup();
    window.Cleanup();
    return static_cast<int>(msg.wParam);
}