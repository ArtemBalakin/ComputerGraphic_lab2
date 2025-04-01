cbuffer ConstantBuffer : register(b0) {
    float4x4 worldViewProj;
    float4 color;
    int useTexture;
    float3 lightDir; // Направление света
    float padding;   // Выравнивание
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.normal = input.normal; // Передаем нормали в пиксельный шейдер
    output.texCoord = input.texCoord;
    return output;
}

float4 PSMainTextured(PS_INPUT input) : SV_TARGET {
    float3 lightDirection = normalize(lightDir);
    float3 normal = normalize(input.normal);
    float lightIntensity = max(dot(normal, -lightDirection), 0.1f); // Минимум 0.1 для базового освещения
    float4 texColor = tex.Sample(samp, input.texCoord);
    return texColor * lightIntensity;
}

float4 PSMainColored(PS_INPUT input) : SV_TARGET {
    float3 lightDirection = normalize(lightDir);
    float3 normal = normalize(input.normal);
    float lightIntensity = max(dot(normal, -lightDirection), 0.1f); // Минимум 0.1 для базового освещения
    return color * lightIntensity;
}