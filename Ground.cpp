#include "Ground.h"
#include <memory>
#include "Logger.h"
#include <DirectXTex.h>

Ground::Ground(ID3D11Device* device, const std::string& modelPath)
    : position(0.0f, 0.0f, 0.0f), color(0.0f, 0.392f, 0.0f, 1.0f),
      vertexBuffer(nullptr), indexBuffer(nullptr), textureSRV(nullptr) {
    logger << "[Ground] Начало создания объекта Ground" << std::endl;
    logger << "[Ground] Проверка пути к модели: " << modelPath << std::endl;

    modelLoader = std::make_unique<ModelLoader>();
    if (!modelPath.empty() && modelLoader->LoadModel(modelPath)) {
        logger << "[Ground] Модель успешно загружена: " << modelPath << std::endl;
        vertices = modelLoader->GetVertices();
        indices = modelLoader->GetIndices();
        std::string texPath = modelLoader->GetTexturePath();
        logger << "[Ground] Путь к текстуре из модели: " << texPath << std::endl;

        if (!texPath.empty()) {
            std::string fullTexPath = "Textures/" + texPath;
            logger << "[Ground] Попытка загрузить текстуру: " << fullTexPath << std::endl;
            LoadTexture(device, fullTexPath);
            if (textureSRV) {
                logger << "[Ground] Текстура успешно загружена: " << fullTexPath << std::endl;
            } else {
                logger << "[Ground] Ошибка: текстура не загружена: " << fullTexPath << std::endl;
            }
        } else {
            logger << "[Ground] Текстура не указана в модели" << std::endl;
        }
    } else {
        logger << "[Ground] Ошибка: не удалось загрузить модель, используется запасная плоскость" << std::endl;
        float planeVertices[] = {
            -50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
             50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
             50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
            -50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
        };
        unsigned int planeIndices[] = {
            0, 1, 2,
            2, 3, 0
        };
        vertices.assign(planeVertices, planeVertices + sizeof(planeVertices) / sizeof(float));
        indices.assign(planeIndices, planeIndices + sizeof(planeIndices) / sizeof(unsigned int));
    }

    logger << "[Ground] Инициализация буферов" << std::endl;
    InitializeBuffers(device);
    logger << "[Ground] Объект Ground успешно создан" << std::endl;
}

Ground::~Ground() {
    if (vertexBuffer) vertexBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
    if (textureSRV) textureSRV->Release();
    logger << "[Ground] Объект Ground уничтожен" << std::endl;
}

void Ground::InitializeBuffers(ID3D11Device* device) {
    logger << "[Ground] Начало инициализации буферов" << std::endl;

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(float));
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
    if (FAILED(hr)) {
        logger << "[Ground] Ошибка: не удалось создать вершинный буфер" << std::endl;
    } else {
        logger << "[Ground] Вершинный буфер успешно создан" << std::endl;
    }

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(unsigned int));
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    hr = device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);
    if (FAILED(hr)) {
        logger << "[Ground] Ошибка: не удалось создать индексный буфер" << std::endl;
    } else {
        logger << "[Ground] Индексный буфер успешно создан" << std::endl;
    }

    logger << "[Ground] Буферы успешно инициализированы" << std::endl;
}

void Ground::LoadTexture(ID3D11Device* device, const std::string& texturePath) {
    logger << "[Ground] Начало загрузки текстуры: " << texturePath << std::endl;
    std::wstring wTexPath(texturePath.begin(), texturePath.end());
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromWICFile(wTexPath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);

    if (FAILED(hr)) {
        logger << "[Ground] Ошибка: не удалось загрузить текстуру из файла: " << texturePath << std::endl;
        return;
    }

    logger << "[Ground] Текстура из файла загружена, создание ресурса" << std::endl;
    ID3D11Texture2D* texture;
    hr = DirectX::CreateTexture(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), (ID3D11Resource**)&texture);

    if (SUCCEEDED(hr)) {
        logger << "[Ground] Ресурс текстуры создан, создание SRV" << std::endl;
        hr = device->CreateShaderResourceView(texture, nullptr, &textureSRV);
        texture->Release();
        if (SUCCEEDED(hr)) {
            logger << "[Ground] SRV для текстуры успешно создан: " << texturePath << std::endl;
        } else {
            logger << "[Ground] Ошибка: не удалось создать SRV для текстуры: " << texturePath << std::endl;
        }
    } else {
        logger << "[Ground] Ошибка: не удалось создать ресурс текстуры: " << texturePath << std::endl;
    }
}

void Ground::Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj) const {
    logger << "[Ground] Начало рендеринга пола" << std::endl;

    DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    DirectX::XMMATRIX worldViewProj = world * viewProj;

    struct ConstantBufferData {
        DirectX::XMMATRIX worldViewProj;
        DirectX::XMFLOAT4 color;
        BOOL useTexture;
    } cbData;
    cbData.worldViewProj = DirectX::XMMatrixTranspose(worldViewProj);
    cbData.color = color;
    cbData.useTexture = textureSRV != nullptr;

    context->UpdateSubresource(constantBuffer, 0, nullptr, &cbData, 0, 0);
    logger << "[Ground] Константный буфер обновлен" << std::endl;

    if (cbData.useTexture && textureSRV) {
        logger << "[Ground] Рендеринг с текстурой" << std::endl;
        context->PSSetShaderResources(0, 1, &textureSRV);
    } else {
        logger << "[Ground] Рендеринг с цветом (зеленая плоскость)" << std::endl;
    }

    UINT stride = 8 * sizeof(float);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    logger << "[Ground] Вершинный буфер установлен" << std::endl;

    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    logger << "[Ground] Индексный буфер установлен" << std::endl;

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
    logger << "[Ground] Выполнен вызов DrawIndexed, индексов: " << indices.size() << std::endl;

    logger << "[Ground] Рендеринг пола завершен" << std::endl;
}

bool Ground::HasTexture() const {
    bool hasTex = textureSRV != nullptr;
    logger << "[Ground] Проверка наличия текстуры: " << (hasTex ? "да" : "нет") << std::endl;
    return hasTex;
}