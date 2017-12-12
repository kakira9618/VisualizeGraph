#include "StringSprite.h"
#include "Main.h"

StringSprite::StringSprite(ID3D11Device *pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pConstantBuffer, int fontsize, WCHAR* fontfamily): m_pDevice(pDevice), m_pDeviceContext(pDeviceContext), m_pConstantBuffer(pConstantBuffer) {
	for (TCHAR c = ' '; c <= '~'; c++) {
		m_pCodes[c] = new Font(m_pDevice, m_pDeviceContext, &c, fontsize, fontfamily);
	}
}

void StringSprite::SetDrawOption(ID3D11VertexShader* pVertexShader, ID3D11PixelShader* pPixelShader, ID3D11InputLayout* pVertexLayout, ID3D11SamplerState* pSampleLinear) {

	//使用するシェーダーの登録	
	m_pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	m_pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
	//頂点インプットレイアウトをセット
	m_pDeviceContext->IASetInputLayout(pVertexLayout);
	//サンプラーを登録
	m_pDeviceContext->PSSetSamplers(0, 1, &pSampleLinear);

	//ブレンディングステート
	ID3D11BlendState* hpBlendState = NULL;
	D3D11_BLEND_DESC BlendStateDesc;
	BlendStateDesc.AlphaToCoverageEnable = FALSE;
	BlendStateDesc.IndependentBlendEnable = FALSE;
	for (int i = 0; i < 8; i++) {
		BlendStateDesc.RenderTarget[i].BlendEnable = TRUE;
		BlendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		BlendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		BlendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		BlendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		BlendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		BlendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	m_pDevice->CreateBlendState(&BlendStateDesc, &hpBlendState);

	float blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	m_pDeviceContext->OMSetBlendState(hpBlendState, blendFactor, 0xffffffff);

}

void StringSprite::DrawText(int x, int y, TCHAR *pText) {


	//コンスタントバッファーを使うシェーダーの登録
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//プリミティブ・トポロジーをセット
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	//バーテックスバッファーをセット
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	int n = _tcslen(pText);

	for (int i = 0; i < n; i++) {
		TCHAR c = pText[i];
		m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pCodes[c]->m_pFontBuffer, &stride, &offset);
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_pCodes[c]->m_pTexResView);

		//シェーダーのコンスタントバッファーに各種データを渡す	
		D3D11_MAPPED_SUBRESOURCE pData;
		SIMPLESHADER_CONSTANT_BUFFER cb;
		if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			D3DXVECTOR3 transPos = PIXELPOS(WINDOW_WIDTH / 2 + x, WINDOW_HEIGHT / 2 + y);
			D3DXMATRIX m;
			D3DXMatrixTranslation(&m, transPos.x, transPos.y, 0);
			D3DXMatrixTranspose(&m, &m);
			cb.mWVP = m;
			memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
			m_pDeviceContext->Unmap(m_pConstantBuffer, 0);
		}

		m_pDeviceContext->Draw(4, 0);

		x += m_pCodes[c]->m_wh.x;
	}
}

StringSprite::~StringSprite() {
	for (TCHAR c = 0; c < 256; c++) {
		delete m_pCodes[c];
	}
}
