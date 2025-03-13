cbuffer ConstantBuffer : register(b0) {
    float4x4 worldViewProj;
};

struct VS_INPUT {
    float3 pos : POSITION;
    float3 color : COLOR;
};

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input) {
    VS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.color = input.color;
    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET {
    return float4(input.color, 1.0f);
}