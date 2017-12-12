//ヘッダーファイルのインクルード

#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <d3dCompiler.h>
#include <vector>
#include <random>
#include <utility>
#include <set>
#include <algorithm>
#include <io.h>
#include <Fcntl.h>
#include <iostream>
#include <sstream>
#include "FPSCounter.h"
#include "Font.h"
#include "StringSprite.h"
#include "DataStructure.h"
#include "Fruchterman.h"
//
//
//
class MAIN
{
public:
	MAIN();
	~MAIN();
	HRESULT InitWindow(HINSTANCE, INT, INT, INT, INT, LPCWSTR);
	LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM);
	void InitApp();
	HRESULT InitD3D();
	HRESULT InitPolygon();
	HRESULT InitShader();


	void Loop();
	void App();
	void Render();
	void DestroyD3D();
	void updateLineBuffer();

	std::vector<Node> m_nodes; // ノード情報
	std::vector<Edge> m_edges; // エッジ情報
	std::vector<Point> m_lines; // エッジの描画情報
	
	int m_node_w; // 表示する幅、高さ/2[px]
	int m_node_numv; // ノード数
	int m_node_nume; // エッジ数

	//↓アプリにひとつ
	HWND m_hWnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D* m_pDepthStencil;
	//↓モデルの種類ごと(モデルの構造が全て同一ならアプリにひとつ）
	ID3D11InputLayout* m_pVertexLayout;
	ID3D11InputLayout* m_pLineLayout;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11VertexShader* m_pLineShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11PixelShader* m_pGradPixelShader;
	ID3D11PixelShader* m_pLinePixelShader;
	ID3D11Buffer* m_pConstantBuffer;
	//↓モデルごと	
	ID3D11Buffer* m_pVertexBuffer; 
	ID3D11Buffer* m_pLineBuffer;
	ID3D11SamplerState* m_pSampleLinear;//テクスチャーのサンプラー
	ID3D11ShaderResourceView* m_pTexture;//テクスチャー
	ID3D11Texture2D* m_pTextTexture;
	StringSprite* m_pInfoText;
};
