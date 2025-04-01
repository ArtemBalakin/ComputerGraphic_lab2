#include "ModelLoader.h"
#include "Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ModelLoader::ModelLoader() : texturePath("") {
    logger << "[ModelLoader] Создан объект ModelLoader" << std::endl;
}

bool ModelLoader::LoadModel(const std::string& filePath) {
    logger << "[ModelLoader] Начало загрузки модели: " << filePath << std::endl;

    Assimp::Importer importer;
    logger << "[ModelLoader] Попытка загрузить файл: " << filePath << std::endl;

    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene) {
        logger << "[ModelLoader] Ошибка: не удалось загрузить сцену: " << importer.GetErrorString() << std::endl;
        return false;
    }

    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        logger << "[ModelLoader] Ошибка: сцена неполная: " << importer.GetErrorString() << std::endl;
        return false;
    }

    if (!scene->mRootNode) {
        logger << "[ModelLoader] Ошибка: отсутствует корневой узел" << std::endl;
        return false;
    }

    if (!scene->HasMeshes()) {
        logger << "[ModelLoader] Ошибка: меши отсутствуют в модели" << std::endl;
        return false;
    }

    logger << "[ModelLoader] Модель успешно разобрана, количество меш: " << scene->mNumMeshes << std::endl;

    aiMesh* mesh = scene->mMeshes[0];
    logger << "[ModelLoader] Обработка меша: вершины=" << mesh->mNumVertices << ", грани=" << mesh->mNumFaces << std::endl;

    vertices.clear();
    indices.clear();

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D pos = mesh->mVertices[i];
        aiVector3D norm = mesh->mNormals[i];
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
        vertices.push_back(norm.x);
        vertices.push_back(norm.y);
        vertices.push_back(norm.z);
        if (mesh->HasTextureCoords(0)) {
            aiVector3D texCoord = mesh->mTextureCoords[0][i];
            vertices.push_back(texCoord.x);
            vertices.push_back(texCoord.y);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            logger << "[ModelLoader] Текстурные координаты отсутствуют для вершины " << i << ", установлены в (0, 0)" << std::endl;
        }
    }
    logger << "[ModelLoader] Вершины обработаны, общее количество float: " << vertices.size() << std::endl;

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }
    logger << "[ModelLoader] Индексы обработаны, общее количество: " << indices.size() << std::endl;

    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString path;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
            texturePath = path.C_Str();
            logger << "[ModelLoader] Текстура найдена в модели: " << texturePath << std::endl;
        } else {
            logger << "[ModelLoader] Текстура в материале не найдена" << std::endl;
        }
    } else {
        logger << "[ModelLoader] Материалы отсутствуют в модели" << std::endl;
    }

    logger << "[ModelLoader] Загрузка модели завершена успешно" << std::endl;
    return true;
}

const std::vector<float>& ModelLoader::GetVertices() const {
    logger << "[ModelLoader] Запрос вершин, размер: " << vertices.size() << std::endl;
    return vertices;
}

const std::vector<unsigned int>& ModelLoader::GetIndices() const {
    logger << "[ModelLoader] Запрос индексов, размер: " << indices.size() << std::endl;
    return indices;
}

const std::string& ModelLoader::GetTexturePath() const {
    logger << "[ModelLoader] Запрос пути к текстуре: " << texturePath << std::endl;
    return texturePath;
}