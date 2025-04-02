#include "CelestialBody.h"
#include <memory>
#include "ModelLoader.h"
#include "Logger.h"
#include <DirectXTex.h>

CelestialBody::CelestialBody(ID3D11Device* device, const std::string& modelPath, DirectX::XMFLOAT3 pos,
                             DirectX::XMFLOAT4 col, float rad, bool useTex, DirectX::XMFLOAT3 emissiveCol)
    : position(pos), color(col), radius(rad), useTexture(useTex), emissiveColor(emissiveCol),
      vertexBuffer(nullptr), indexBuffer(nullptr), textureSRV(nullptr), parent(nullptr) {
    logger << "[CelestialBody] Начало создания объекта" << std::endl;
    logger << "[CelestialBody] Проверка пути к модели: " << modelPath << std::endl;

    rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    relativeTransform = DirectX::XMMatrixIdentity();

    modelLoader = std::make_unique<ModelLoader>();
    if (!modelPath.empty() && modelLoader->LoadModel(modelPath)) {
        logger << "[CelestialBody] Модель успешно загружена: " << modelPath << std::endl;
        vertices = modelLoader->GetVertices();
        indices = modelLoader->GetIndices();
        std::string texPath = modelLoader->GetTexturePath();
        logger << "[CelestialBody] Путь к текстуре из модели: " << texPath << std::endl;

        if (!texPath.empty() && useTexture) {
            std::string fullTexPath = "Textures/" + texPath;
            logger << "[CelestialBody] Попытка загрузить текстуру: " << fullTexPath << std::endl;
            LoadTexture(device, fullTexPath);
            if (textureSRV) {
                logger << "[CelestialBody] Текстура успешно загружена: " << fullTexPath << std::endl;
            } else {
                logger << "[CelestialBody] Ошибка: текстура не загружена: " << fullTexPath << std::endl;
            }
        }
    } else {
        logger << "[CelestialBody] Ошибка: не удалось загрузить модель" << std::endl;
    }

    logger << "[CelestialBody] Инициализация буферов" << std::endl;
    InitializeBuffers(device);
    logger << "[CelestialBody] Объект успешно создан" << std::endl;
}

CelestialBody::~CelestialBody() {
    if (vertexBuffer) vertexBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
    if (textureSRV) textureSRV->Release();
    logger << "[CelestialBody] Объект уничтожен" << std::endl;
}

void CelestialBody::InitializeBuffers(ID3D11Device* device) {
    logger << "[CelestialBody] Начало инициализации буферов" << std::endl;

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(float));
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
    if (FAILED(hr)) {
        logger << "[CelestialBody] Ошибка W не удалось создать вершинный буфер" << std::endl;
    } else {
        logger << "[CelestialBody] Вершинный буфер успешно создан" << std::endl;
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
        logger << "[CelestialBody] Ошибка: не удалось создать индексный буфер" << std::endl;
    } else {
        logger << "[CelestialBody] Индексный буфер успешно создан" << std::endl;
    }

    logger << "[CelestialBody] Буферы успешно инициализированы" << std::endl;
}

void CelestialBody::LoadTexture(ID3D11Device* device, const std::string& texturePath) {
    logger << "[CelestialBody] Начало загрузки текстуры: " << texturePath << std::endl;
    std::wstring wTexPath(texturePath.begin(), texturePath.end());
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromWICFile(wTexPath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);

    if (FAILED(hr)) {
        logger << "[CelestialBody] Ошибка: не удалось загрузить текстуру из файла: " << texturePath << std::endl;
        return;
    }

    logger << "[CelestialBody] Текстура из файла загружена, создание ресурса" << std::endl;
    ID3D11Texture2D* texture;
    hr = DirectX::CreateTexture(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), (ID3D11Resource**)&texture);

    if (SUCCEEDED(hr)) {
        logger << "[CelestialBody] Ресурс текстуры создан, создание SRV" << std::endl;
        hr = device->CreateShaderResourceView(texture, nullptr, &textureSRV);
        texture->Release();
        if (SUCCEEDED(hr)) {
            logger << "[CelestialBody] SRV для текстуры успешно создан: " << texturePath << std::endl;
        } else {
            logger << "[CelestialBody] Ошибка: не удалось создать SRV для текстуры: " << texturePath << std::endl;
        }
    } else {
        logger << "[CelestialBody] Ошибка: не удалось создать ресурс текстуры: " << texturePath << std::endl;
    }
}

void CelestialBody::Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX viewProj,
                        DirectX::XMFLOAT3 cameraPos, float fogStart, float fogEnd, DirectX::XMFLOAT4 fogColor) const {
    logger << "[CelestialBody] Начало рендеринга" << std::endl;

    DirectX::XMMATRIX world = GetWorldMatrix();
    DirectX::XMMATRIX worldViewProj = world * viewProj;

    struct ConstantBufferData {
        DirectX::XMMATRIX worldViewProj;
        DirectX::XMFLOAT4 color;
        BOOL useTexture;
        DirectX::XMFLOAT3 lightDir;
        float padding1;
        DirectX::XMMATRIX world;
        DirectX::XMFLOAT3 cameraPos;
        float fogStart;
        float fogEnd;
        DirectX::XMFLOAT4 fogColor;
        DirectX::XMFLOAT3 lightPos;
        DirectX::XMFLOAT3 lightColor;
        DirectX::XMFLOAT3 materialDiffuse;
        DirectX::XMFLOAT3 materialSpecular;
        float shininess;
        DirectX::XMFLOAT3 emissiveColor; // Подсветка
        float padding2;
    } cbData;

    cbData.worldViewProj = DirectX::XMMatrixTranspose(worldViewProj);
    cbData.color = color;
    cbData.useTexture = textureSRV != nullptr && useTexture;
    cbData.lightDir = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f); // Старое направление света
    cbData.padding1 = 0.0f;
    cbData.world = DirectX::XMMatrixTranspose(world);
    cbData.cameraPos = cameraPos;
    cbData.fogStart = fogStart;
    cbData.fogEnd = fogEnd;
    cbData.fogColor = fogColor;
    cbData.lightPos = DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f);       // Свет сверху
    cbData.lightColor = DirectX::XMFLOAT3(2.0f, 2.0f, 2.0f);      // Яркий белый свет
    cbData.materialDiffuse = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); // Полное отражение
    cbData.materialSpecular = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f); // Блики
    cbData.shininess = 64.0f;                                      // Глянцевость
    cbData.emissiveColor = emissiveColor;                          // Подсветка объекта
    cbData.padding2 = 0.0f;

    context->UpdateSubresource(constantBuffer, 0, nullptr, &cbData, 0, 0);
    logger << "[CelestialBody] Константный буфер обновлен" << std::endl;

    if (cbData.useTexture && textureSRV) {
        logger << "[CelestialBody] Рендеринг с текстурой" << std::endl;
        context->PSSetShaderResources(0, 1, &textureSRV);
    } else {
        logger << "[CelestialBody] Рендеринг с цветом" << std::endl;
    }

    UINT stride = 8 * sizeof(float);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    logger << "[CelestialBody] Вершинный буфер установлен" << std::endl;

    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    logger << "[CelestialBody] Индексный буфер установлен" << std::endl;

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
    logger << "[CelestialBody] Выполнен вызов DrawIndexed, индексов: " << indices.size() << std::endl;

    for (const auto* child : children) {
        child->Draw(context, constantBuffer, viewProj, cameraPos, fogStart, fogEnd, fogColor);
    }

    logger << "[CelestialBody] Рендеринг завершен" << std::endl;
}

void CelestialBody::UpdatePosition(DirectX::XMVECTOR velocity, float deltaTime) {
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(velocity, deltaTime));
    DirectX::XMStoreFloat3(&position, pos);
}

void CelestialBody::Update() {
    if (parent) {
        DirectX::XMMATRIX parentWorld = parent->GetWorldMatrix();
        DirectX::XMMATRIX world = relativeTransform * parentWorld;
        DirectX::XMVECTOR pos, rot, scale;
        DirectX::XMMatrixDecompose(&scale, &rot, &pos, world);
        DirectX::XMStoreFloat3(&position, pos);
        DirectX::XMStoreFloat4(&rotation, rot);
    }
    for (auto* child : children) {
        child->Update();
    }
}

DirectX::XMMATRIX CelestialBody::GetWorldMatrix() const {
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(radius, radius, radius);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    return scale * rotationMatrix * translation;
}

const CelestialBody* CelestialBody::CheckCollision(const CelestialBody* other, DirectX::XMVECTOR& attachmentPoint) const {
    if (other == this || other->parent) return nullptr;

    DirectX::XMVECTOR thisPos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR otherPos = DirectX::XMLoadFloat3(&other->position);
    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(thisPos, otherPos)));
    float collisionDistance = radius + other->radius;

    if (distance < collisionDistance) {
        DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(otherPos, thisPos));
        attachmentPoint = DirectX::XMVectorAdd(thisPos, DirectX::XMVectorScale(direction, radius));
        return other;
    }
    return nullptr;
}

void CelestialBody::AttachChild(CelestialBody* child) {
    if (child == this || child->parent) return;

    child->parent = this;
    DirectX::XMVECTOR childPos = DirectX::XMLoadFloat3(&child->position);
    DirectX::XMVECTOR thisPos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR relativePos = DirectX::XMVectorSubtract(childPos, thisPos);
    child->relativeTransform = DirectX::XMMatrixTranslationFromVector(relativePos);
    children.push_back(child);
}