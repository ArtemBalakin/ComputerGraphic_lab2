#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <windows.h>
#include "Render.h"
#include "Window.h"
#include "CelestialBody.h"
#include "Camera.h"
#include <iostream>

using namespace DirectX;

struct ConstantBuffer {
    XMMATRIX worldViewProj;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    std::cout << "=== Starting DirectX Solar System Application ===" << std::endl;
    std::cout << "WinMain called with hInstance: " << hInstance << ", nCmdShow: " << nCmdShow << std::endl;

    Window window(800, 600, L"DirectX Solar System");
    std::cout << "Initializing window with size 800x600 and title 'DirectX Solar System'..." << std::endl;
    if (!window.Initialize()) {
        std::cout << "ERROR: Failed to initialize window." << std::endl;
        return -1;
    }
    std::cout << "Window initialized successfully. Handle: " << window.GetHandle() << std::endl;

    Render render;
    std::cout << "Initializing render system..." << std::endl;
    if (!render.Initialize(window.GetHandle(), 800, 600)) {
        std::cout << "ERROR: Failed to initialize render system." << std::endl;
        return -1;
    }
    std::cout << "Render system initialized successfully." << std::endl;

    ID3D11Device* device = render.GetDevice();
    ID3D11DeviceContext* context = render.GetContext();
    std::cout << "Retrieved device: " << device << ", context: " << context << std::endl;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ID3D11Buffer* constantBuffer = nullptr;
    std::cout << "Creating constant buffer with size: " << sizeof(ConstantBuffer) << " bytes..." << std::endl;
    HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create constant buffer. Error code: " << hr << std::endl;
        return -1;
    }
    std::cout << "Constant buffer created successfully. Pointer: " << constantBuffer << std::endl;

    std::vector<std::unique_ptr<CelestialBody>> bodies;
    std::cout << "Creating celestial bodies..." << std::endl;
    // Звезда (ярко-жёлтая)
    bodies.push_back(std::make_unique<CelestialBody>(device, true, nullptr, 2.0f, XMFLOAT3(1.0f, 1.0f, 0.2f)));
    bodies[0]->position = XMFLOAT3(0, 0, 0);
    bodies[0]->rotationSpeed = 0.5f;
    std::cout << "Star created at (0, 0, 0) with scale 2.0 and rotation speed 0.5, color yellow." << std::endl;

    // Планета 1 (голубая)
    bodies.push_back(std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.0f, XMFLOAT3(0.2f, 0.6f, 1.0f)));
    bodies[1]->orbitRadius = 5.0f;
    bodies[1]->orbitSpeed = 1.0f;
    bodies[1]->rotationSpeed = 1.0f;
    bodies[1]->position = XMFLOAT3(5.0f, 0, 0);
    std::cout << "Planet 1 created with orbit radius 5.0, orbit speed 1.0, rotation speed 1.0, scale 1.0, initial position (5, 0, 0), color blue." << std::endl;

    // Луна 1 для Планеты 1 (серая)
    bodies.push_back(std::make_unique<CelestialBody>(device, false, bodies[1].get(), 0.5f, XMFLOAT3(0.7f, 0.7f, 0.7f)));
    bodies[2]->orbitRadius = 1.0f;
    bodies[2]->orbitSpeed = 2.0f;
    bodies[2]->rotationSpeed = 2.0f;
    bodies[2]->position = XMFLOAT3(1.0f, 0, 0);
    std::cout << "Moon 1 for Planet 1 created with orbit radius 1.0, orbit speed 2.0, rotation speed 2.0, scale 0.5, initial position (1, 0, 0), color gray." << std::endl;

    // Планета 2 (зелёная)
    bodies.push_back(std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.2f, XMFLOAT3(0.1f, 0.9f, 0.3f)));
    bodies[3]->orbitRadius = 7.0f;
    bodies[3]->orbitSpeed = 0.8f;
    bodies[3]->rotationSpeed = 0.9f;
    bodies[3]->position = XMFLOAT3(7.0f, 0, 0);
    std::cout << "Planet 2 created with orbit radius 7.0, orbit speed 0.8, rotation speed 0.9, scale 1.2, initial position (7, 0, 0), color green." << std::endl;

    // Луна 1 для Планеты 2 (фиолетовая)
    bodies.push_back(std::make_unique<CelestialBody>(device, false, bodies[3].get(), 0.4f, XMFLOAT3(0.6f, 0.2f, 0.8f)));
    bodies[4]->orbitRadius = 1.2f;
    bodies[4]->orbitSpeed = 1.8f;
    bodies[4]->rotationSpeed = 1.5f;
    bodies[4]->position = XMFLOAT3(1.2f, 0, 0);
    std::cout << "Moon 1 for Planet 2 created with orbit radius 1.2, orbit speed 1.8, rotation speed 1.5, scale 0.4, initial position (1.2, 0, 0), color purple." << std::endl;

    // Планета 3 (красная)
    bodies.push_back(std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.5f, XMFLOAT3(1.0f, 0.3f, 0.3f)));
    bodies[5]->orbitRadius = 9.0f;
    bodies[5]->orbitSpeed = 0.6f;
    bodies[5]->rotationSpeed = 0.7f;
    bodies[5]->position = XMFLOAT3(9.0f, 0, 0);
    std::cout << "Planet 3 created with orbit radius 9.0, orbit speed 0.6, rotation speed 0.7, scale 1.5, initial position (9, 0, 0), color red." << std::endl;

    // Луна 1 для Планеты 3 (белая)
    bodies.push_back(std::make_unique<CelestialBody>(device, false, bodies[5].get(), 0.6f, XMFLOAT3(1.0f, 1.0f, 1.0f)));
    bodies[6]->orbitRadius = 1.5f;
    bodies[6]->orbitSpeed = 1.5f;
    bodies[6]->rotationSpeed = 1.2f;
    bodies[6]->position = XMFLOAT3(1.5f, 0, 0);
    std::cout << "Moon 1 for Planet 3 created with orbit radius 1.5, orbit speed 1.5, rotation speed 1.2, scale 0.6, initial position (1.5, 0, 0), color white." << std::endl;

    // Планета 4 (оранжевая)
    bodies.push_back(std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.8f, XMFLOAT3(1.0f, 0.6f, 0.0f)));
    bodies[7]->orbitRadius = 11.0f;
    bodies[7]->orbitSpeed = 0.5f;
    bodies[7]->rotationSpeed = 0.6f;
    bodies[7]->position = XMFLOAT3(11.0f, 0, 0);
    std::cout << "Planet 4 created with orbit radius 11.0, orbit speed 0.5, rotation speed 0.6, scale 1.8, initial position (11, 0, 0), color orange." << std::endl;

    // Луна 1 для Планеты 4 (розовая)
    bodies.push_back(std::make_unique<CelestialBody>(device, false, bodies[7].get(), 0.5f, XMFLOAT3(1.0f, 0.4f, 0.7f)));
    bodies[8]->orbitRadius = 1.8f;
    bodies[8]->orbitSpeed = 1.2f;
    bodies[8]->rotationSpeed = 1.0f;
    bodies[8]->position = XMFLOAT3(1.8f, 0, 0);
    std::cout << "Moon 1 for Planet 4 created with orbit radius 1.8, orbit speed 1.2, rotation speed 1.0, scale 0.5, initial position (1.8, 0, 0), color pink." << std::endl;

    // Луна 2 для Планеты 4 (бирюзовая)
    bodies.push_back(std::make_unique<CelestialBody>(device, false, bodies[7].get(), 0.4f, XMFLOAT3(0.0f, 0.8f, 0.8f)));
    bodies[9]->orbitRadius = 2.0f;
    bodies[9]->orbitSpeed = 1.0f;
    bodies[9]->rotationSpeed = 0.8f;
    bodies[9]->position = XMFLOAT3(2.0f, 0, 0);
    std::cout << "Moon 2 for Planet 4 created with orbit radius 2.0, orbit speed 1.0, rotation speed 0.8, scale 0.4, initial position (2.0, 0, 0), color turquoise." << std::endl;

    std::vector<std::unique_ptr<Camera>> cameras;
    cameras.emplace_back(std::make_unique<FPSCamera>());
    cameras.emplace_back(std::make_unique<OrbitalCamera>());
    int currentCamera = 0;
    std::cout << "Created " << cameras.size() << " cameras (FPS and Orbital)." << std::endl;

    cameras[0]->position = XMFLOAT3(0, 0, -30.0f);
    cameras[1]->position = XMFLOAT3(0, 0, -30.0f);
    cameras[1]->target = XMFLOAT3(0, 0, 0);
    std::cout << "Initialized cameras. FPS at (0, 0, -30), Orbital at (0, 0, -30) targeting (0, 0, 0)." << std::endl;

    LARGE_INTEGER frequency, lastTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);
    std::cout << "Performance counter initialized. Frequency: " << frequency.QuadPart << std::endl;

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            std::cout << "Processing message: " << msg.message << std::endl;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float deltaTime = (currentTime.QuadPart - lastTime.QuadPart) / (float)frequency.QuadPart;
            lastTime = currentTime;
            std::cout << "New frame started. DeltaTime: " << deltaTime << " seconds." << std::endl;

            if (GetAsyncKeyState('1') & 0x8000) {
                currentCamera = 0;
                std::cout << "Switched to FPS camera. Current camera index: " << currentCamera << std::endl;
            }
            if (GetAsyncKeyState('2') & 0x8000) {
                currentCamera = 1;
                std::cout << "Switched to Orbital camera. Current camera index: " << currentCamera << std::endl;
            }
            if (GetAsyncKeyState('3') & 0x8000) {
                cameras[currentCamera]->fov = XM_PIDIV4;
                std::cout << "Set FOV to 45 degrees for camera " << currentCamera << std::endl;
            }
            if (GetAsyncKeyState('4') & 0x8000) {
                cameras[currentCamera]->fov = XM_PIDIV2;
                std::cout << "Set FOV to 90 degrees for camera " << currentCamera << std::endl;
            }
            if (GetAsyncKeyState('5') & 0x8000) {
                cameras[currentCamera]->fov = XM_PIDIV4 / 2;
                std::cout << "Set FOV to 22.5 degrees for camera " << currentCamera << std::endl;
            }

            for (size_t i = 0; i < bodies.size(); ++i) {
                std::cout << "Updating body " << i << " with deltaTime: " << deltaTime << std::endl;
                bodies[i]->Update(deltaTime);
            }
            std::cout << "All bodies updated." << std::endl;

            std::cout << "Updating camera " << currentCamera << " with deltaTime: " << deltaTime << std::endl;
            cameras[currentCamera]->Update(deltaTime);
            std::cout << "Camera updated." << std::endl;

            render.BeginFrame();
            for (size_t i = 0; i < bodies.size(); ++i) {
                std::cout << "Attempting to draw body " << i << " with camera " << currentCamera << std::endl;
                bodies[i]->Draw(context, render, cameras[currentCamera]->GetViewMatrix(),
                                cameras[currentCamera]->GetProjectionMatrix(), constantBuffer);
            }
            render.EndFrame();
            std::cout << "Frame completed." << std::endl;
        }
    }

    if (constantBuffer) {
        constantBuffer->Release();
        std::cout << "Constant buffer released." << std::endl;
    }
    render.Cleanup();
    window.Cleanup();
    std::cout << "Application cleaned up and exiting with code " << static_cast<int>(msg.wParam) << "." << std::endl;
    return static_cast<int>(msg.wParam);
}