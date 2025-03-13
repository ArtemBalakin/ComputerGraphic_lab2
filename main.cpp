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

    ID3D11Device *device = render.GetDevice();
    ID3D11DeviceContext *context = render.GetContext();
    std::cout << "Retrieved device: " << device << ", context: " << context << std::endl;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ID3D11Buffer *constantBuffer = nullptr;
    std::cout << "Creating constant buffer with size: " << sizeof(ConstantBuffer) << " bytes..." << std::endl;
    HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    if (FAILED(hr)) {
        std::cout << "ERROR: Failed to create constant buffer. Error code: " << hr << std::endl;
        return -1;
    }
    std::cout << "Constant buffer created successfully. Pointer: " << constantBuffer << std::endl;

    std::vector<std::unique_ptr<CelestialBody> > bodies;
    std::cout << "Creating celestial bodies..." << std::endl;

    // Звезда (без орбиты)
    bodies.push_back(std::make_unique<CelestialBody>(device, true, nullptr, 2.0f, XMFLOAT3(1.0f, 1.0f, 0.2f), 0.0f));
    bodies[0]->position = XMFLOAT3(0, 2.0f, 0);
    bodies[0]->rotationSpeed = 0.5f;
    std::cout << "Star created at (0, 2.0, 0) with scale 2.0 and rotation speed 0.5, color yellow." << std::endl;

    // Планета 1 (голубая, наклон 0.2 радиана ~ 11.5°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.0f, XMFLOAT3(0.2f, 0.6f, 1.0f), 0.2f));
    bodies[1]->orbitRadius = 7.0f;
    bodies[1]->orbitSpeed = 1.0f;
    bodies[1]->rotationSpeed = 1.0f;
    std::cout <<
            "Planet 1 created with orbit radius 7.0, orbit speed 1.0, rotation speed 1.0, scale 1.0, inclination 0.2 rad, color blue."
            << std::endl;

    // Луна 1 для Планеты 1 (серая, наклон 0.4 радиана ~ 23°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, false, bodies[1].get(), 0.5f, XMFLOAT3(0.7f, 0.7f, 0.7f), 0.4f));
    bodies[2]->orbitRadius = 5.0f;
    bodies[2]->orbitSpeed = 2.0f;
    bodies[2]->rotationSpeed = 2.0f;
    std::cout <<
            "Moon 1 for Planet 1 created with orbit radius 1.0, orbit speed 2.0, rotation speed 2.0, scale 0.5, inclination 0.4 rad, color gray."
            << std::endl;

    // Планета 2 (зелёная, наклон -0.3 радиана ~ -17°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.2f, XMFLOAT3(0.1f, 0.9f, 0.3f), -0.3f));
    bodies[3]->orbitRadius = 10.0f;
    bodies[3]->orbitSpeed = 0.8f;
    bodies[3]->rotationSpeed = 0.9f;
    std::cout <<
            "Planet 2 created with orbit radius 10.0, orbit speed 0.8, rotation speed 0.9, scale 1.2, inclination -0.3 rad, color green."
            << std::endl;

    // Луна 1 для Планеты 2 (фиолетовая, наклон 0.5 радиана ~ 28.6°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, false, bodies[3].get(), 0.4f, XMFLOAT3(0.6f, 0.2f, 0.8f), 0.5f));
    bodies[4]->orbitRadius = 5.2f;
    bodies[4]->orbitSpeed = 1.8f;
    bodies[4]->rotationSpeed = 1.5f;
    std::cout <<
            "Moon 1 for Planet 2 created with orbit radius 1.2, orbit speed 1.8, rotation speed 1.5, scale 0.4, inclination 0.5 rad, color purple."
            << std::endl;

    // Планета 3 (красная, наклон 0.1 радиана ~ 5.7°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.5f, XMFLOAT3(1.0f, 0.3f, 0.3f), 0.1f));
    bodies[5]->orbitRadius = 13.0f;
    bodies[5]->orbitSpeed = 0.6f;
    bodies[5]->rotationSpeed = 0.7f;
    std::cout <<
            "Planet 3 created with orbit radius 13.0, orbit speed 0.6, rotation speed 0.7, scale 1.5, inclination 0.1 rad, color red."
            << std::endl;

    // Луна 1 для Планеты 3 (белая, наклон -0.6 радиана ~ -34.4°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, false, bodies[5].get(), 0.6f, XMFLOAT3(1.0f, 1.0f, 1.0f), -0.6f));
    bodies[6]->orbitRadius = 5.5f;
    bodies[6]->orbitSpeed = 1.5f;
    bodies[6]->rotationSpeed = 1.2f;
    std::cout <<
            "Moon 1 for Planet 3 created with orbit radius 1.5, orbit speed 1.5, rotation speed 1.2, scale 0.6, inclination -0.6 rad, color white."
            << std::endl;

    // Планета 4 (оранжевая, наклон 0.35 радиана ~ 20°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, true, bodies[0].get(), 1.8f, XMFLOAT3(1.0f, 0.6f, 0.0f), 0.35f));
    bodies[7]->orbitRadius = 16.0f;
    bodies[7]->orbitSpeed = 0.5f;
    bodies[7]->rotationSpeed = 0.6f;
    std::cout <<
            "Planet 4 created with orbit radius 16.0, orbit speed 0.5, rotation speed 0.6, scale 1.8, inclination 0.35 rad, color orange."
            << std::endl;

    // Луна 1 для Планеты 4 (розовая, наклон 0.7 радиана ~ 40°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, false, bodies[7].get(), 0.5f, XMFLOAT3(1.0f, 0.4f, 0.7f), 0.7f));
    bodies[8]->orbitRadius = 5.8f;
    bodies[8]->orbitSpeed = 1.2f;
    bodies[8]->rotationSpeed = 1.0f;
    std::cout <<
            "Moon 1 for Planet 4 created with orbit radius 1.8, orbit speed 1.2, rotation speed 1.0, scale 0.5, inclination 0.7 rad, color pink."
            << std::endl;

    // Луна 2 для Планеты 4 (бирюзовая, наклон -0.2 радиана ~ -11.5°)
    bodies.push_back(
        std::make_unique<CelestialBody>(device, false, bodies[7].get(), 0.4f, XMFLOAT3(0.0f, 0.8f, 0.8f), -0.2f));
    bodies[9]->orbitRadius = 5.0f;
    bodies[9]->orbitSpeed = 1.0f;
    bodies[9]->rotationSpeed = 0.8f;
    std::cout <<
            "Moon 2 for Planet 4 created with orbit radius 2.0, orbit speed 1.0, rotation speed 0.8, scale 0.4, inclination -0.2 rad, color turquoise."
            << std::endl;

    Grid grid(device, 200.0f, 100);

    std::vector<std::unique_ptr<Camera> > cameras;
    cameras.emplace_back(std::make_unique<FPSCamera>());
    cameras.emplace_back(std::make_unique<OrbitalCamera>());
    cameras.emplace_back(std::make_unique<FollowCamera>(bodies[1].get()));
    int currentCamera = 0;
    cameras[0]->position = XMFLOAT3(0, 2.0f, -30.0f);
    cameras[1]->position = XMFLOAT3(0, 2.0f, -30.0f);
    cameras[1]->target = XMFLOAT3(0, 2.0f, 0);
    cameras[2]->position = bodies[1]->position;

    LARGE_INTEGER frequency, lastTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastTime);

    std::cout << "All initialization completed. Showing window..." << std::endl;
    window.Show();

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            float deltaTime = (currentTime.QuadPart - lastTime.QuadPart) / (float) frequency.QuadPart;
            lastTime = currentTime;

            if (GetAsyncKeyState('T') & 0x8000) {
                currentCamera = (currentCamera + 1) % cameras.size();
                Sleep(200);
            }
            if (GetAsyncKeyState('3') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV4;
            if (GetAsyncKeyState('4') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV2;
            if (GetAsyncKeyState('5') & 0x8000) cameras[currentCamera]->fov = XM_PIDIV4 / 2;

            for (size_t i = 0; i < bodies.size(); ++i) {
                bodies[i]->Update(deltaTime);
            }

            cameras[currentCamera]->Update(deltaTime);

            render.BeginFrame();

            if (false) {
                XMMATRIX world = XMMatrixIdentity();
                XMMATRIX worldViewProj = world * cameras[currentCamera]->GetViewMatrix() * cameras[currentCamera]->
                                         GetProjectionMatrix();
                ConstantBuffer cb;
                cb.worldViewProj = XMMatrixTranspose(worldViewProj);
                context->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
                context->VSSetConstantBuffers(0, 1, &constantBuffer);
                grid.Draw(context);
            }

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
