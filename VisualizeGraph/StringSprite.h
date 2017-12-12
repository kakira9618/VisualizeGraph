#pragma once
#include <TCHAR.h>
#include "Font.h"

class StringSprite {
	ID3D11Device *m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	ID3D11Buffer* m_pConstantBuffer;
public:
	Font *m_pCodes[256];
	StringSprite(ID3D11Device *pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pConstantBuffer, int fontsize = 32, WCHAR* fontfamily = TEXT("‚l‚r –¾’©"));
	void SetDrawOption(ID3D11VertexShader* pVertexShader, ID3D11PixelShader* pPixelShader, ID3D11InputLayout* pVertexLayout, ID3D11SamplerState* pSampleLinear);
	void DrawText(int x, int y, TCHAR *pText);
	~StringSprite();
};