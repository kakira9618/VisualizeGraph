#include "Font.h"
#include "Main.h"

Font::Font(ID3D11Device* pDev, ID3D11DeviceContext* pDeviceContext, TCHAR* text, int fontsize, WCHAR* fontfamily) : m_iFontsize(fontsize), m_pDeviceContext(pDeviceContext), m_pText(text), m_pFontfamily(fontfamily), m_pDev(pDev) {
	CreateFontTexture(m_pDev, m_pText, m_pFontfamily, m_iFontsize, &m_pTexture, &m_wh);
}

bool Font::UpdateFontTexture(TCHAR* text, int fontsize, WCHAR* fontfamily) {
	if (fontsize == 0) fontsize = m_iFontsize;
	if (!fontfamily) fontfamily = m_pFontfamily;
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pTexResView);
	SAFE_RELEASE(m_pFontBuffer);
	return CreateFontTexture(m_pDev, text, fontfamily, fontsize, &m_pTexture, &m_wh);
}

// 指定文字のフォントテクスチャを取得
// ※DirectX9技術編から流用しています
// http://marupeke296.com/DX10_No5_FontTexture.html
bool Font::CreateFontTexture(ID3D11Device* pDev, TCHAR* c, WCHAR* fontfamily, int fontsize, ID3D11Texture2D** ppTexture, D3DXVECTOR2 *wh) {

	// フォントの生成
	LOGFONT lf = { fontsize, 0, 0, 0, 0, 0, 0, 0, SHIFTJIS_CHARSET, OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("") };
	
	wcscpy_s(lf.lfFaceName, fontfamily);
	HFONT hFont;
	if (!(hFont = CreateFontIndirect(&lf))) {
		return false;
	}

	// デバイスコンテキスト取得
	// デバイスにフォントを持たせないとGetGlyphOutline関数はエラーとなる
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// 文字コード取得
	UINT code = 0;
#if _UNICODE
	// unicodeの場合、文字コードは単純にワイド文字のUINT変換です
	code = (UINT)*c;
#else
	// マルチバイト文字の場合、
	// 1バイト文字のコードは1バイト目のUINT変換、
	// 2バイト文字のコードは[先導コード]*256 + [文字コード]です
	if (IsDBCSLeadByte(*c))
		code = (BYTE)c[0] << 8 | (BYTE)c[1];
	else
		code = c[0];
#endif

	// フォントビットマップ取得
	TEXTMETRIC TM;

	GetTextMetrics(hdc, &TM);
	GLYPHMETRICS GM;
	CONST MAT2 Mat = { { 0,1 },{ 0,0 },{ 0,0 },{ 0,1 } };
	DWORD size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);
	BYTE* ptr = new BYTE[size];
	GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, size, ptr, &Mat);

	// デバイスコンテキストとフォントハンドルの開放
	SelectObject(hdc, oldFont);
	DeleteObject(hFont);
	ReleaseDC(NULL, hdc);


	//--------------------------------
	// 書き込み可能テクスチャ作成
	//--------------------------------

	// CPUで書き込みができるテクスチャを作成します。
	// 自前でテクスチャに色をつけたい場合に使えます。

	// テクスチャ作成
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = GM.gmCellIncX;
	desc.Height = TM.tmHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// RGBA(255,255,255,255)タイプ
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;			// 動的（書き込みするための必須条件）
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// シェーダリソースとして使う
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// CPUからアクセスして書き込みOK

	pDev->CreateTexture2D(&desc, 0, ppTexture);

	wh->x = (float)desc.Width;
	wh->y = (float)desc.Height;

	// テクスチャに書き込み
	// テクスチャをマップ（＝ロック）すると、
	// メモリにアクセスするための情報がD3D10_MAPPED_TEXTURE2Dに格納されます。
	D3D11_MAPPED_SUBRESOURCE mapped;
	m_pDeviceContext->Map(*ppTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	/*
	(*ppTexture)->Map(
	D3D11CalcSubresource(0, 0, 1),
	D3D11_MAP_WRITE_DISCARD,
	0,
	&mapped);
	*/
	BYTE* pBits = (BYTE*)mapped.pData;

	// フォント情報の書き込み
	// iOfs_x, iOfs_y : 書き出し位置(左上)
	// iBmp_w, iBmp_h : フォントビットマップの幅高
	// Level : α値の段階 (GGO_GRAY4_BITMAPなので17段階)
	int iOfs_x = GM.gmptGlyphOrigin.x;
	int iOfs_y = TM.tmAscent - GM.gmptGlyphOrigin.y;
	int iBmp_w = GM.gmBlackBoxX + (4 - (GM.gmBlackBoxX % 4)) % 4;
	int iBmp_h = GM.gmBlackBoxY;
	int Level = 17;
	int x, y;
	DWORD Alpha, Color;
	memset(pBits, 0, mapped.RowPitch * TM.tmHeight);
	for (y = iOfs_y; y < iOfs_y + iBmp_h; y++)
		for (x = iOfs_x; x < iOfs_x + iBmp_w; x++) {
			Alpha = (255 * ptr[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
			Color = 0x00ffffff | (Alpha << 24);
			memcpy((BYTE*)pBits + mapped.RowPitch * y + 4 * x, &Color, sizeof(DWORD));
		}

	m_pDeviceContext->Unmap(*ppTexture, 0);

	delete[] ptr;

	pDev->CreateShaderResourceView(*ppTexture, 0, &m_pTexResView);

	//バーテックスバッファー作成
	SimpleVertex fontVertices[] =
	{
		PIXELPOS(0,m_wh.y), D3DXVECTOR2(0,1),//頂点1,
		PIXELPOS(0,0), D3DXVECTOR2(0,0),//頂点2
		PIXELPOS(m_wh.x, m_wh.y), D3DXVECTOR2(1,1), //頂点3
		PIXELPOS(m_wh.x,0), D3DXVECTOR2(1,0), //頂点4
	};

	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(fontVertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = fontVertices;
	if (FAILED(pDev->CreateBuffer(&bd, &InitData, &m_pFontBuffer)))
	{
		return E_FAIL;
	}

	return true;
}

Font::~Font() {
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pTexResView);
	SAFE_RELEASE(m_pFontBuffer);
}