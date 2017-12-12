//グローバル

Texture2D g_texDecal: register(t0);//テクスチャーは レジスターt(n)
SamplerState g_samLinear : register(s0);//サンプラーはレジスターs(n)

cbuffer global
{
	matrix g_mWVP;
};

//構造体
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

//バーテックスシェーダー
VS_OUTPUT VS(float4 Pos : POSITION, float2 Tex : TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(Pos, g_mWVP);
	output.Tex = Tex;
	return output;
} 

//バーテックスシェーダー(Line)
float4 VS_LINE(float4 Pos : POSITION) : SV_POSITION 
{
	return Pos;
}

//ピクセルシェーダー
float4 PS(VS_OUTPUT input) : SV_Target
{
	return g_texDecal.Sample(g_samLinear, input.Tex);
}


//ピクセルシェーダー(Line)
float4 PS_LINE(float4 Pos : SV_POSITION) : SV_Target
{
	return float4(1, 0, 0, 1);
}


// グラデーション文字用
float4 PS_GRAD(VS_OUTPUT input) : SV_Target
{
	float4 c = g_texDecal.Sample(g_samLinear, input.Tex);
	c.x = 1;
	c.y = c.z = lerp(0.8, 0.3, input.Tex.y);
	return c;
}
