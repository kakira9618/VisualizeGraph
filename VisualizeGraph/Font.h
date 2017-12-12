#pragma once

#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <d3dCompiler.h>
#include <iostream>
#include <string>

class Font {
	ID3D11Texture2D* m_pTexture;
	ID3D11Device *m_pDev;
	ID3D11DeviceContext* m_pDeviceContext;

public:
	int m_iFontsize;
	TCHAR *m_pText;
	WCHAR *m_pFontfamily;
	D3DXVECTOR2 m_wh;
	ID3D11Buffer* m_pFontBuffer;
	ID3D11ShaderResourceView* m_pTexResView;
	Font(ID3D11Device* pDev, ID3D11DeviceContext* pDeviceContext, TCHAR* text, int fontsize = 32, WCHAR* fontfamily = TEXT("‚l‚r –¾’©"));
	~Font();
	bool UpdateFontTexture(TCHAR* text, int fontsize, WCHAR* fontfamily);
	bool CreateFontTexture(ID3D11Device* pDev, TCHAR* c, WCHAR* fontfamily, int fontsize, ID3D11Texture2D** ppTexture, D3DXVECTOR2 *wh);
};