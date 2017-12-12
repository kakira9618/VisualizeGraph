#include "MAIN.h"

//グローバル変数
MAIN* g_pMain = NULL;
CFPSCounter FPS(100);

void createConsoleWindow() {
	AllocConsole();    
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

//関数プロトタイプの宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
//
//アプリケーションのエントリー関数 
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT)
{
	g_pMain = new MAIN;
	if (g_pMain != NULL)
	{
		if (DEBUG) createConsoleWindow();
		if (SUCCEEDED(g_pMain->InitWindow(hInstance, 0, 0, WINDOW_WIDTH,
			WINDOW_HEIGHT, APP_NAME)))
		{
			g_pMain->InitApp();
			if (SUCCEEDED(g_pMain->InitD3D()))
			{
				g_pMain->Loop();
			}
		}
		//アプリ終了
		g_pMain->DestroyD3D();
		delete g_pMain;
	}
	return 0;
}
//
//
//OSから見たウィンドウプロシージャー（実際の処理はMAINクラスのプロシージャーで処理）
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return g_pMain->MsgProc(hWnd, uMsg, wParam, lParam);
}



//
//
//
MAIN::MAIN()
{
	ZeroMemory(this, sizeof(MAIN));
}
//
//
//
MAIN::~MAIN()
{
}


void MAIN::InitApp() {
	std::random_device rnd;
	std::mt19937 mt(rnd());
	std::uniform_real_distribution<> randfloat(0, 1);

	m_node_w = NODE_WIDTH_DEFAULT / 2;

	m_nodes.clear();
	m_edges.clear();
	m_lines.clear();

	m_node_numv = NODE_NUMV_DEFAULT;
	for (int i = 0; i < m_node_numv; i++) {
		float x = randfloat(mt) * WINDOW_WIDTH;
		float y = randfloat(mt) * WINDOW_HEIGHT;
		m_nodes.emplace_back(x, y);
	}

	m_node_nume = NODE_NUME_DEFAULT;
	std::uniform_int_distribution<> randIndex(0, m_node_numv - 1);
	std::uniform_real_distribution<> randCost(COST_MIN, COST_MAX);
	std::set<std::pair<int, int>> flags;
	for (int i = 0; i < m_node_nume; i++) {
		int s = randIndex(mt);
		int t = randIndex(mt);
		if (s == t) {
			i--; continue;
		}
		float c = randCost(mt);
		std::pair<int, int> fromto = std::make_pair(s, t);
		std::pair<int, int> tofrom = std::make_pair(t, s);
		if (flags.find(fromto) != flags.end()) {
			i--; continue;
		}
		flags.insert(fromto);
		flags.insert(tofrom);
		m_edges.emplace_back(s, t, c);
	}

	for (int i = 0; i < m_node_nume; i++) {
		m_lines.emplace_back(m_nodes[m_edges[i].s].x, m_nodes[m_edges[i].s].y);
		m_lines.emplace_back(m_nodes[m_edges[i].t].x, m_nodes[m_edges[i].t].y);
	}

}
//
//
//ウィンドウ作成
HRESULT MAIN::InitWindow(HINSTANCE hInstance,
	INT iX, INT iY, INT iWidth, INT iHeight, LPCWSTR WindowName)
{
	// ウィンドウの定義
	WNDCLASSEX  wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wc);
	//ウィンドウの作成
	m_hWnd = CreateWindow(WindowName, WindowName, WS_OVERLAPPEDWINDOW,
		0, 0, iWidth, iHeight, 0, 0, hInstance, 0);
	if (!m_hWnd)
	{
		return E_FAIL;
	}
	//ウインドウの表示

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return S_OK;
}
//
//
//ウィンドウプロシージャー
LRESULT MAIN::MsgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_KEYDOWN:
		switch ((char)wParam)
		{
		case VK_ESCAPE://ESCキーで修了
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
//
//
//メッセージループとアプリケーション処理の入り口
void MAIN::Loop()
{
	// メッセージループ
	MSG msg = { 0 };
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//アプリケーションの処理はここから飛ぶ。
			App();
		}
	}
	//アプリケーションの終了
}
//
//
//アプリケーション処理。アプリのメイン関数。
void MAIN::App()
{
	Render();
}
//
//
//
HRESULT MAIN::InitD3D()
{
	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = NULL;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		0, &pFeatureLevels, 1, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice,
		pFeatureLevel, &m_pDeviceContext)))
	{
		return FALSE;
	}
	//レンダーターゲットビューの作成
	ID3D11Texture2D *pBackBuffer;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
	//深度ステンシルビューの作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = WINDOW_WIDTH;
	descDepth.Height = WINDOW_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	m_pDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
	// m_pDevice->CreateDepthStencilView(m_pDepthStencil, NULL, &m_pDepthStencilView);
	// レンダーターゲットビューと深度ステンシルビューをパイプラインにバインド
	// m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	m_pDeviceContext->OMSetRenderTargets(1,&m_pRenderTargetView,NULL());

	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pDeviceContext->RSSetViewports(1, &vp);
	//ラスタライズ設定
	D3D11_RASTERIZER_DESC rdc;
	ZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_NONE;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.AntialiasedLineEnable = TRUE;
	ID3D11RasterizerState* pIr = NULL;
	m_pDevice->CreateRasterizerState(&rdc, &pIr);
	m_pDeviceContext->RSSetState(pIr);
	SAFE_RELEASE(pIr);

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

	//シェーダー初期化
	if (FAILED(InitShader()))
	{
		return E_FAIL;
	}
	//ポリゴン作成
	if (FAILED(InitPolygon()))
	{
		return E_FAIL;
	}

	return S_OK;
}
//
//
//
void MAIN::DestroyD3D()
{
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pDepthStencil);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pSampleLinear);
	SAFE_RELEASE(m_pTexture);
	delete m_pInfoText;
}
//
//
//シェーダーを作成　頂点レイアウトを定義
HRESULT MAIN::InitShader()
{
	//hlslファイル読み込み ブロブ作成　ブロブとはシェーダーの塊みたいなもの。XXシェーダーとして特徴を持たない。後で各種シェーダーに成り得る。
	ID3DBlob *pCompiledShader = NULL;
	ID3DBlob *pErrors = NULL;
	//ブロブからバーテックスシェーダー作成
	if (FAILED(D3DX11CompileFromFile(L"Effect.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"hlsl読み込み失敗1", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	if (FAILED(m_pDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &m_pVertexShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	//頂点インプットレイアウトを定義	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	//頂点インプットレイアウトを作成
	if (FAILED(m_pDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &m_pVertexLayout)))
	{
		return FALSE;
	}
	//ブロブからバーテックスシェーダー作成
	if (FAILED(D3DX11CompileFromFile(L"Effect.hlsl", NULL, NULL, "VS_LINE", "vs_5_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"hlsl読み込み失敗2", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	//ブロブからバーテックスシェーダー作成
	if (FAILED(m_pDevice->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &m_pLineShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"バーテックスシェーダー(Line)作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	D3D11_INPUT_ELEMENT_DESC layout_line[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};	
	numElements = sizeof(layout_line) / sizeof(layout_line[0]);
	//頂点インプットレイアウトを作成
	if (FAILED(m_pDevice->CreateInputLayout(layout_line, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &m_pLineLayout)))
	{
		return FALSE;
	}


	//ブロブからピクセルシェーダー作成
	if (FAILED(D3DX11CompileFromFile(L"Effect.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"hlsl読み込み失敗3", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);
	if (FAILED(m_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &m_pPixelShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//ブロブからピクセルシェーダー作成
	if (FAILED(D3DX11CompileFromFile(L"Effect.hlsl", NULL, NULL, "PS_GRAD", "ps_5_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"hlsl読み込み失敗3", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);
	if (FAILED(m_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &m_pGradPixelShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//ブロブからピクセルシェーダー作成
	if (FAILED(D3DX11CompileFromFile(L"Effect.hlsl", NULL, NULL, "PS_LINE", "ps_5_0", 0, 0, NULL, &pCompiledShader, &pErrors, NULL)))
	{
		MessageBox(0, L"hlsl読み込み失敗4", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);
	if (FAILED(m_pDevice->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &m_pLinePixelShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	SAFE_RELEASE(pCompiledShader);
	//コンスタントバッファー作成　ここでは変換行列渡し用
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SIMPLESHADER_CONSTANT_BUFFER);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(m_pDevice->CreateBuffer(&cb, NULL, &m_pConstantBuffer)))
	{
		return E_FAIL;
	}
	return S_OK;
}

void MAIN::updateLineBuffer() {
	for (int i = 0; i < m_node_nume; i++) {
		D3DXVECTOR3 p1 = PIXELPOS(m_nodes[m_edges[i].s].x, m_nodes[m_edges[i].s].y);
		D3DXVECTOR3 p2 = PIXELPOS(m_nodes[m_edges[i].t].x, m_nodes[m_edges[i].t].y);
		m_lines[2 * i].s.x = p1.x;
		m_lines[2 * i].s.y = p1.y;
		m_lines[2 * i + 1].s.x = p2.x;
		m_lines[2 * i + 1].s.y = p2.y;
	}
	D3D11_MAPPED_SUBRESOURCE msr;
	m_pDeviceContext->Map(m_pLineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &m_lines[0], sizeof(m_lines[0]) * m_lines.size());
	m_pDeviceContext->Unmap(m_pLineBuffer, 0);
}

HRESULT MAIN::InitPolygon()
{
	//バーテックスバッファー作成
	SimpleVertex vertices[] =
	{
		PIXELPOS(-m_node_w,m_node_w), D3DXVECTOR2(0,1),//頂点1,
		PIXELPOS(-m_node_w,-m_node_w), D3DXVECTOR2(0,0),//頂点2
		PIXELPOS(m_node_w,m_node_w), D3DXVECTOR2(1,1), //頂点3
		PIXELPOS(m_node_w,-m_node_w), D3DXVECTOR2(1,0), //頂点4
	};

	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	if (FAILED(m_pDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer)))
	{
		return E_FAIL;
	}

	// StringSprite作成
	m_pInfoText = new StringSprite(m_pDevice, m_pDeviceContext, m_pConstantBuffer, 60, TEXT("メイリオ"));
	

	// 線用のバッファ作成
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(m_lines[0]) * m_lines.size();
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	InitData.pSysMem = &m_lines[0];
	if (FAILED(m_pDevice->CreateBuffer(&bd, &InitData, &m_pLineBuffer)))
	{
		return E_FAIL;
	}

	//テクスチャー用サンプラー作成
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));
	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	m_pDevice->CreateSamplerState(&SamDesc, &m_pSampleLinear);
	//テクスチャー作成
	if (FAILED(D3DX11CreateShaderResourceViewFromFile(m_pDevice, L"node.png", NULL, NULL, &m_pTexture, NULL)))
	{
		return E_FAIL;
	}

	return S_OK;
}
//
//
//シーンを画面にレンダリング
void MAIN::Render()
{
	//画面クリア（実際は単色で画面を塗りつぶす処理）
	float ClearColor[4] = { 135.0f / 255, 206.0f / 255, 235.0f / 255, 1 };// クリア色作成　RGBAの順
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);//画面クリア
//	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);//深度バッファクリア

	static int nowitr = 0;
	if (USE_GPU) {
		Fruchterman::calcWithGPU(m_pDevice, m_pDeviceContext, m_nodes, m_edges, 1, 1000, nowitr++);
	}
	else {
		Fruchterman::calc(m_nodes, m_edges, 1, 1000, nowitr++);
	}
	updateLineBuffer();

	//使用するシェーダーの登録	
	m_pDeviceContext->VSSetShader(m_pLineShader, NULL, 0);
	m_pDeviceContext->PSSetShader(m_pLinePixelShader, NULL, 0);
	//頂点インプットレイアウトをセット
	m_pDeviceContext->IASetInputLayout(m_pLineLayout);
	//プリミティブ・トポロジーをセット
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//バーテックスバッファーをセット
	UINT stride = sizeof(Point);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pLineBuffer, &stride, &offset);
	m_pDeviceContext->Draw(m_node_nume * 2, 0);


	//使用するシェーダーの登録	
	m_pDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
	//コンスタントバッファーを使うシェーダーの登録
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//頂点インプットレイアウトをセット
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout);
	//プリミティブ・トポロジーをセット
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//バーテックスバッファーをセット
	stride = sizeof(SimpleVertex);
	offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//テクスチャーをシェーダーに渡す
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pSampleLinear);
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTexture);

	// ノードを描画
	for (int i = 0; i < m_node_numv; i++) {
		//シェーダーのコンスタントバッファーに各種データを渡す	
		D3D11_MAPPED_SUBRESOURCE pData;
		SIMPLESHADER_CONSTANT_BUFFER cb;
		if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			//ワールド、カメラ、射影行列を渡す
			D3DXVECTOR3 transPos = PIXELPOS(WINDOW_WIDTH / 2 + m_nodes[i].x, WINDOW_HEIGHT / 2 + m_nodes[i].y);
			D3DXMATRIX m;
			D3DXMatrixTranslation(&m, transPos.x, transPos.y, 0);
			D3DXMatrixTranspose(&m, &m);
			cb.mWVP = m;
			memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
			m_pDeviceContext->Unmap(m_pConstantBuffer, 0);
		}
		//プリミティブをレンダリング
		m_pDeviceContext->Draw(4, 0);
	}
	static bool dispText = true;
	static bool lastPushed = false;
	if (!lastPushed && GetKeyState('Z') & 0x80) {
		dispText = dispText == false;
		lastPushed = true;
	}
	if (lastPushed && !(GetKeyState('Z') & 0x80)) {
		lastPushed = false;
	}


	m_pDeviceContext->PSSetShader(m_pGradPixelShader, NULL, 0);
	static int count = 0;
	double fps = FPS.GetFPS();
	if (count++ % 100 == 0) {
		std::cout << "FPS: " << fps << std::endl;
	}
	if (dispText) {
		std::wstring s = TEXT("FPS: ") + std::to_wstring(fps);
		m_pInfoText->DrawTextW(0, 0, (TCHAR *)s.c_str());

		s = TEXT("|V|: ") + std::to_wstring(m_nodes.size()) + _TEXT(", |E|: ") + std::to_wstring(m_edges.size());
		m_pInfoText->DrawTextW(0, 50, (TCHAR *)s.c_str());
	}

	m_pSwapChain->Present(DISP_SYNC, 0);//画面更新（バックバッファをフロントバッファに）	
}