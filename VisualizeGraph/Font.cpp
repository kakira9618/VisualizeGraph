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

// �w�蕶���̃t�H���g�e�N�X�`�����擾
// ��DirectX9�Z�p�҂��痬�p���Ă��܂�
// http://marupeke296.com/DX10_No5_FontTexture.html
bool Font::CreateFontTexture(ID3D11Device* pDev, TCHAR* c, WCHAR* fontfamily, int fontsize, ID3D11Texture2D** ppTexture, D3DXVECTOR2 *wh) {

	// �t�H���g�̐���
	LOGFONT lf = { fontsize, 0, 0, 0, 0, 0, 0, 0, SHIFTJIS_CHARSET, OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("") };
	
	wcscpy_s(lf.lfFaceName, fontfamily);
	HFONT hFont;
	if (!(hFont = CreateFontIndirect(&lf))) {
		return false;
	}

	// �f�o�C�X�R���e�L�X�g�擾
	// �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�
	HDC hdc = GetDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

	// �����R�[�h�擾
	UINT code = 0;
#if _UNICODE
	// unicode�̏ꍇ�A�����R�[�h�͒P���Ƀ��C�h������UINT�ϊ��ł�
	code = (UINT)*c;
#else
	// �}���`�o�C�g�����̏ꍇ�A
	// 1�o�C�g�����̃R�[�h��1�o�C�g�ڂ�UINT�ϊ��A
	// 2�o�C�g�����̃R�[�h��[�擱�R�[�h]*256 + [�����R�[�h]�ł�
	if (IsDBCSLeadByte(*c))
		code = (BYTE)c[0] << 8 | (BYTE)c[1];
	else
		code = c[0];
#endif

	// �t�H���g�r�b�g�}�b�v�擾
	TEXTMETRIC TM;

	GetTextMetrics(hdc, &TM);
	GLYPHMETRICS GM;
	CONST MAT2 Mat = { { 0,1 },{ 0,0 },{ 0,0 },{ 0,1 } };
	DWORD size = GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, 0, NULL, &Mat);
	BYTE* ptr = new BYTE[size];
	GetGlyphOutline(hdc, code, GGO_GRAY4_BITMAP, &GM, size, ptr, &Mat);

	// �f�o�C�X�R���e�L�X�g�ƃt�H���g�n���h���̊J��
	SelectObject(hdc, oldFont);
	DeleteObject(hFont);
	ReleaseDC(NULL, hdc);


	//--------------------------------
	// �������݉\�e�N�X�`���쐬
	//--------------------------------

	// CPU�ŏ������݂��ł���e�N�X�`�����쐬���܂��B
	// ���O�Ńe�N�X�`���ɐF���������ꍇ�Ɏg���܂��B

	// �e�N�X�`���쐬
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = GM.gmCellIncX;
	desc.Height = TM.tmHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// RGBA(255,255,255,255)�^�C�v
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;			// ���I�i�������݂��邽�߂̕K�{�����j
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// �V�F�[�_���\�[�X�Ƃ��Ďg��
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// CPU����A�N�Z�X���ď�������OK

	pDev->CreateTexture2D(&desc, 0, ppTexture);

	wh->x = (float)desc.Width;
	wh->y = (float)desc.Height;

	// �e�N�X�`���ɏ�������
	// �e�N�X�`�����}�b�v�i�����b�N�j����ƁA
	// �������ɃA�N�Z�X���邽�߂̏��D3D10_MAPPED_TEXTURE2D�Ɋi�[����܂��B
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

	// �t�H���g���̏�������
	// iOfs_x, iOfs_y : �����o���ʒu(����)
	// iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���
	// Level : ���l�̒i�K (GGO_GRAY4_BITMAP�Ȃ̂�17�i�K)
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

	//�o�[�e�b�N�X�o�b�t�@�[�쐬
	SimpleVertex fontVertices[] =
	{
		PIXELPOS(0,m_wh.y), D3DXVECTOR2(0,1),//���_1,
		PIXELPOS(0,0), D3DXVECTOR2(0,0),//���_2
		PIXELPOS(m_wh.x, m_wh.y), D3DXVECTOR2(1,1), //���_3
		PIXELPOS(m_wh.x,0), D3DXVECTOR2(1,0), //���_4
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