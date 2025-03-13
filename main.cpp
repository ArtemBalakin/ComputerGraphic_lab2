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
#include <random>

using namespace DirectX;

struct ConstantBuffer {
    XMMATRIX worldViewProj;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    //std::cout << "=== Starting DirectX Gravitational Simulation ===" << std::endl;

    Window window(1300, 1000, L"DirectX Gravitational Simulation");
    if (!window.Initialize()) {
        //std::cout << "ERROR: Failed to initialize window." << std::endl;
        return -1;
    }

    Render render;
    if (!render.Initialize(window.GetHandle(), 1300, 1000)) {
        //std::cout << "ERROR: Failed to initialize render system." << std::endl;
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
        //std::cout << "ERROR: Failed to create constant buffer." << std::endl;
        return -1;
    }

    // Вектор для хранения 100 планет
    std::vector<std::unique_ptr<CelestialBody>> bodies;

    // Генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-25.0f, 25.0f); // Позиции в кубе 50x50x50
    std::uniform_real_distribution<float> velDist(0.0f, 0.5f);   // Сниженные скорости
    std::uniform_real_distribution<float> sizeDist(1.0f, 5.0f);   // Размеры чуть больше для видимости
    std::uniform_real_distribution<float> massDist(1.0f, 50.0f);  // Массы меньше
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);  // Цвета

    // Создаём 100 планет
    for (int i = 0; i < 100 ; ++i) {
        XMFLOAT3 position(posDist(gen), posDist(gen), posDist(gen)); // Случайная позиция в кубе 50x50x50
        XMFLOAT3 color(colorDist(gen), colorDist(gen), colorDist(gen)); // Случайный цвет
        float size = sizeDist(gen); // Случайный размер
        float mass = massDist(gen); // Случайная масса

        // Скорость направлена к центру с небольшим случайным разбросом, ослаблена
        XMFLOAT3 velocity(-position.x + velDist(gen),
                         -position.y  + velDist(gen),
                         -position.z  + velDist(gen));

        bodies.push_back(std::make_unique<CelestialBody>(device, true, nullptr, size, color, 0.0f, mass));
        bodies[i]->position = position;
        bodies[i]->velocity = velocity;
        bodies[i]->rotationSpeed = mass * 0.0f; // Вращение пропорционально массе, но слабее
        //std::cout << "Planet " << i << ": Mass " << mass << ", Position (" << position.x << ", " << position.y << ", " << position.z
        //          << "), Velocity (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
    }

    // Настройка камер
    std::vector<std::unique_ptr<Camera>> cameras;
    cameras.emplace_back(std::make_unique<FPSCamera>());
    cameras.emplace_back(std::make_unique<OrbitalCamera>());
    cameras.emplace_back(std::make_unique<FollowCamera>(bodies[0].get()));
    int currentCamera = 0;

    // Настройка камер для видимости куба 50x50x50
    cameras[0]->position = XMFLOAT3(0, 30.0f, -80.0f); // FPS-камера: ближе к кубу
    cameras[0]->target = XMFLOAT3(0, 0, 0);
    cameras[1]->position = XMFLOAT3(0, 30.0f, -80.0f); // Орбитальная камера: ближе
    cameras[1]->target = XMFLOAT3(0, 0, 0);
    cameras[2]->position = bodies[0]->position;        // Камера следования
    cameras[2]->fov = XM_PIDIV2;                       // Широкий угол обзора

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
            float deltaTime = (currentTime.QuadPart - lastTime.QuadPart) / (float)frequency.QuadPart; // Ещё больше замедляем
            lastTime = currentTime;

            if (GetAsyncKeyState('T') & 0x8000) {
                currentCamera = (currentCamera + 1) % cameras.size();
                Sleep(200);
            }
            if (GetAsyncKeyState('3') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV4;
            if (GetAsyncKeyState('4') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV2;
            if (GetAsyncKeyState('5') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV4 / 2;

            // Обновляем все тела с учётом гравитации
            std::vector<CelestialBody *> bodyPtrs;
            for (auto &body : bodies) bodyPtrs.push_back(body.get());
            for (auto &body : bodies) body->Update(deltaTime, bodyPtrs);

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