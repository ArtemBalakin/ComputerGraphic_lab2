cbuffer ConstantBuffer : register(b0) {
    float4x4 worldViewProj;
    float4 color;
    int useTexture;
    float3 lightDir;
    float padding1;
    float4x4 world;
    float3 cameraPos;
    float fogStart;
    float fogEnd;
    float4 fogColor;
    float3 lightPos;
    float3 lightColor;
    float3 materialDiffuse;
    float3 materialSpecular;
    float shininess;
    float3 emissiveColor;
    float padding2;
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
    float2 texCoord : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.normal = mul(input.normal, (float3x3)world);
    output.texCoord = input.texCoord;
    output.worldPos = mul(float4(input.pos, 1.0f), world).xyz;
    return output;
}

float4 PSMainTextured(PS_INPUT input) : SV_TARGET {
    float3 lightDirection = normalize(lightPos - input.worldPos);
    float3 viewDirection = normalize(cameraPos - input.worldPos);
    float3 normal = normalize(input.normal);

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    float3 diffuse = diff * materialDiffuse * lightColor;

    // Specular
    float3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);
    float3 specular = spec * materialSpecular * lightColor;

    float4 texColor = tex.Sample(samp, input.texCoord);
    float4 objectColor = float4(diffuse + specular + emissiveColor, 1.0) * texColor;

    // Fog
    float distance = length(input.worldPos - cameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    return lerp(objectColor, fogColor, fogFactor);
}

float4 PSMainColored(PS_INPUT input) : SV_TARGET {
    float3 lightDirection = normalize(lightPos - input.worldPos);
    float3 viewDirection = normalize(cameraPos - input.worldPos);
    float3 normal = normalize(input.normal);

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    float3 diffuse = diff * materialDiffuse * lightColor;

    // Specular
    float3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);
    float3 specular = spec * materialSpecular * lightColor;

    float4 objectColor = float4(diffuse + specular + emissiveColor, 1.0) * color;

    // Fog
    float distance = length(input.worldPos - cameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    return lerp(objectColor, fogColor, fogFactor);
}