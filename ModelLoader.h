#pragma once
#include <string>
#include <vector>

class ModelLoader {
public:
    ModelLoader();
    bool LoadModel(const std::string& filePath);
    const std::vector<float>& GetVertices() const;
    const std::vector<unsigned int>& GetIndices() const;
    const std::string& GetTexturePath() const;

private:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::string texturePath;
};