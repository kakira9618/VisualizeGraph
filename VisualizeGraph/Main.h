//�w�b�_�[�t�@�C���̃C���N���[�h

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

	std::vector<Node> m_nodes; // �m�[�h���
	std::vector<Edge> m_edges; // �G�b�W���
	std::vector<Point> m_lines; // �G�b�W�̕`����
	
	int m_node_w; // �\�����镝�A����/2[px]
	int m_node_numv; // �m�[�h��
	int m_node_nume; // �G�b�W��

	//���A�v���ɂЂƂ�
	HWND m_hWnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D* m_pDepthStencil;
	//�����f���̎�ނ���(���f���̍\�����S�ē���Ȃ�A�v���ɂЂƂj
	ID3D11InputLayout* m_pVertexLayout;
	ID3D11InputLayout* m_pLineLayout;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11VertexShader* m_pLineShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11PixelShader* m_pGradPixelShader;
	ID3D11PixelShader* m_pLinePixelShader;
	ID3D11Buffer* m_pConstantBuffer;
	//�����f������	
	ID3D11Buffer* m_pVertexBuffer; 
	ID3D11Buffer* m_pLineBuffer;
	ID3D11SamplerState* m_pSampleLinear;//�e�N�X�`���[�̃T���v���[
	ID3D11ShaderResourceView* m_pTexture;//�e�N�X�`���[
	ID3D11Texture2D* m_pTextTexture;
	StringSprite* m_pInfoText;
};
