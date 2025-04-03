cbuffer ConstantBuffer : register(b0) {
    float4x4 worldViewProj;
    float4x4 world;
    float4 color;
    int useTexture;
    float3 lightDir; // Направление света
    float3 lightColor;
    float3 materialDiffuse;
    float3 materialSpecular;
    float shininess;
    float3 emissiveColor;
    float3 cameraPos;
    float padding;
};

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.normal = normalize(mul(input.normal, (float3x3)world));
    output.texCoord = input.texCoord;
    output.worldPos = mul(float4(input.pos, 1.0f), world).xyz;
    return output;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 PSMainTextured(PS_INPUT input) : SV_TARGET {
    float4 texColor = tex.Sample(samp, input.texCoord);
    float3 normal = normalize(input.normal);
    // Добавляем небольшую пертурбацию нормалей для земли
    if (abs(input.worldPos.y) < 0.1f) { // Предполагаем, что земля находится на y ≈ 0
        float2 noise = frac(sin(input.texCoord * 123.45f) * 43758.5453f);
        normal += float3(noise.x * 0.1f, 0.0f, noise.y * 0.1f);
        normal = normalize(normal);
    }
    float3 lightDirection = normalize(lightDir);
    float3 viewDir = normalize(cameraPos - input.worldPos);
    float3 ambient = lightColor * 0.3f;
    float diff = max(dot(normal, lightDirection), 0.0);
    float3 diffuse = lightColor * (diff * materialDiffuse);
    float3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float3 specular = lightColor * (spec * materialSpecular) * 2.0f; // Увеличиваем вклад зеркального света
    float3 lighting = ambient + diffuse + specular;
    float3 finalLighting = lighting + emissiveColor;
    finalLighting = saturate(finalLighting);
    float4 finalColor = texColor * float4(finalLighting, 1.0);
    return finalColor;
}

float4 PSMainColored(PS_INPUT input) : SV_TARGET {
    float3 normal = normalize(input.normal);
    if (abs(input.worldPos.y) < 0.1f) {
        float2 noise = frac(sin(input.texCoord * 123.45f) * 43758.5453f);
        normal += float3(noise.x * 0.1f, 0.0f, noise.y * 0.1f);
        normal = normalize(normal);
    }
    float3 lightDirection = normalize(lightDir);
    float3 viewDir = normalize(cameraPos - input.worldPos);
    float3 ambient = lightColor * 0.3f;
    float diff = max(dot(normal, lightDirection), 0.0);
    float3 diffuse = lightColor * (diff * materialDiffuse);
    float3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float3 specular = lightColor * (spec * materialSpecular) * 2.0f; // Увеличиваем вклад зеркального света
    float3 lighting = ambient + diffuse + specular;
    float3 finalLighting = lighting + emissiveColor;
    finalLighting = saturate(finalLighting);
    float4 finalColor = color * float4(finalLighting, 1.0);
    return finalColor;
}