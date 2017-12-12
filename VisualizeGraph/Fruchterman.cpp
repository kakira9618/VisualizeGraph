#include "Fruchterman.h"

struct Diff {
	float dx, dy;
};

struct NodeData {
	D3DXVECTOR2 pos;
};

struct EdgeData {
	int s;
	int t;
};

struct CBUFFER {
	float W;
	float L;
	float t;
	float k;
};


float Fruchterman::f_r(float d, float k) {
	return (k * k) / d;
}
float Fruchterman::f_a(float d, float k) {
	return (d * d) / k;
}

// Fruchterman-Reingold: http://d.hatena.ne.jp/mFumi/20140213/1392287682
void Fruchterman::calc(std::vector<Node> &nodes, std::vector<Edge> &edges, int iteration, int itrmax, int nowitr) {
	float W = 1.0, L = 1.0;
	float area = W * L;
	float k = sqrt(area / nodes.size()) * SPRING_DESIABLE_LENGTH_C;
	float t = W / 1000, dt = t / (iteration + 1);
	if (itrmax != 0) {
		t -= nowitr * (t / (itrmax + 1));
	}
	if (t < 0) return;
	std::vector<Diff> node_v(nodes.size());
	
	for (int i = 0; i < nodes.size(); i++) {
		nodes[i].x /= WINDOW_WIDTH;
		nodes[i].y /= WINDOW_HEIGHT;
	}

	for (int itr = 0; itr < iteration; itr++) {

		for (int i = 0; i < nodes.size(); i++) {
			Node &v = nodes[i];
			node_v[i].dx = 0;
			node_v[i].dy = 0;
			for (int j = 0; j < nodes.size(); j++) {
				Node &u = nodes[j];
				if (i != j) {
					float dx = v.x - u.x;
					float dy = v.y - u.y;
					float delta = sqrt(dx * dx + dy * dy);
					if (delta != 0.0f) {
						float d = f_r(delta, k) / delta;
						node_v[i].dx += dx * d;
						node_v[i].dy += dy * d;
					}
				}
			}
		}


		for (int i = 0; i < edges.size(); i++) {
			int vi = edges[i].s;
			int ui = edges[i].t;
			Node &v = nodes[vi];
			Node &u = nodes[ui];
			float dx = v.x - u.x;
			float dy = v.y - u.y;
			float delta = sqrt(dx * dx + dy * dy);
			if (delta != 0.0f) {
				float d = f_a(delta, k) / delta;
				float ddx = dx * d;
				float ddy = dy * d;
				node_v[vi].dx += -ddx;
				node_v[ui].dx += +ddx;
				node_v[vi].dy += -ddy;
				node_v[ui].dy += +ddy;
			}
		}
#undef min
#undef max
		for (int i = 0; i < nodes.size(); i++) {
			float dx = node_v[i].dx;
			float dy = node_v[i].dy;
			Node &v = nodes[i];
			float disp = sqrt(dx * dx + dy * dy);
			if (disp != 0.0f) {
				float d = std::min(disp, t) / disp;
				float x = v.x + dx * d;
				float y = v.y + dy * d;
				x = std::min(W, std::max(0.0f, x));
				y = std::min(L, std::max(0.0f, y));
				//v.x = std::min(sqrt(W * W / 4 - y * y), std::max(-sqrt(W * W / 4 - y * y), x)) + W / 2;
				//v.y = std::min(sqrt(L * L / 4 - x * x), std::max(-sqrt(L * L / 4 - x * x), y)) + L / 2;
				v.x = x;
				v.y = y;
			}
		}

		t -= dt;
	}

	
	for (int i = 0; i < nodes.size(); i++) {
		nodes[i].x *= WINDOW_WIDTH;
		nodes[i].y *= WINDOW_HEIGHT;
	}
	
}

void Fruchterman::calcWithGPU(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, std::vector<Node> &nodes, std::vector<Edge> &edges, int iteration, int itrmax, int nowitr) {
#define NUM_DATA nodes.size()
#define NUM_EDGE edges.size()
	static bool firstCall = true;
	// CB: W, L, t
	// SRV: node_x, node_y [n = NODE_NUMV_DEFAULT]
	// SRV: edge_s, edge_t [n = NODE_NUME_DEFAULT]
	// UAV: new_node_x, new_node_y [n = NODE_NUMV_DEFAULT]
	// UAV: new_node_dx, new_node_dy [n = NODE_NUMV_DEFAULT]

	iteration = 1; // for test.
	
	float W = 1.0, L = 1.0;
	float area = W * L;
	float k = sqrt(area / nodes.size()) * SPRING_DESIABLE_LENGTH_C;
	float t = W / 1000, dt = t / (iteration + 1);
	if (itrmax != 0) {
		t -= nowitr * (t / (itrmax + 1));
	}
	if (t < 0) return;

	// データ用意
	NodeData *dataA = new NodeData[NUM_DATA];
	EdgeData *dataB = new EdgeData[NUM_EDGE];

	for (int i = 0; i < NUM_DATA; i++) {
		dataA[i].pos = D3DXVECTOR2(nodes[i].x / WINDOW_WIDTH, nodes[i].y / WINDOW_HEIGHT);
	}

	for (int i = 0; i < NUM_EDGE; i++) {
		dataB[i].s = edges[i].s;
		dataB[i].t = edges[i].t;
	}

	static ID3DBlob* pBlob = NULL;
	static ID3DBlob* pErrorBlob = NULL;
	static ID3D11ComputeShader* pComputeShader1 = NULL;
	static ID3D11ComputeShader* pComputeShader2 = NULL;
	static ID3D11ComputeShader* pComputeShader3 = NULL;
	ID3D11Buffer* pBufferA = NULL;
	ID3D11Buffer* pBufferB = NULL;
	ID3D11Buffer* pBufferResultA = NULL;
	ID3D11Buffer* pBufferResultB = NULL;
	ID3D11ShaderResourceView* pBufferASRV = NULL;
	ID3D11ShaderResourceView* pBufferBSRV = NULL;
	ID3D11UnorderedAccessView* pBufferResultAUAV = NULL;
	ID3D11UnorderedAccessView* pBufferResultBUAV = NULL;

	// シェーダ読み込み
	if (firstCall) {
		HRESULT hr = NULL;
		hr = D3DX11CompileFromFile(L"compute.hlsl", 0, 0, "CalcR", "cs_5_0", 0, 0, 0, &pBlob, &pErrorBlob, 0);
		if (FAILED(hr)) {

			if (pErrorBlob) {
				char* sz = NULL;
				sz = (char *)pErrorBlob->GetBufferPointer();
				std::cout << sz << "シェーダ作成失敗" << std::endl;
			}
			return;
		}
		pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pComputeShader1);

		hr = D3DX11CompileFromFile(L"compute.hlsl", 0, 0, "CalcA", "cs_5_0", 0, 0, 0, &pBlob, &pErrorBlob, 0);
		if (FAILED(hr)) {

			if (pErrorBlob) {
				char* sz = NULL;
				sz = (char *)pErrorBlob->GetBufferPointer();
				std::cout << sz << "シェーダ作成失敗" << std::endl;
			}
			return;
		}
		pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pComputeShader2);

		hr = D3DX11CompileFromFile(L"compute.hlsl", 0, 0, "CalcPos", "cs_5_0", 0, 0, 0, &pBlob, &pErrorBlob, 0);
		if (FAILED(hr)) {

			if (pErrorBlob) {
				char* sz = NULL;
				sz = (char *)pErrorBlob->GetBufferPointer();
				std::cout << sz << "シェーダ作成失敗" << std::endl;
			}
			return;
		}
		pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pComputeShader3);
		firstCall = false;
	}
	
	// バッファオブジェクト作成
	D3D11_BUFFER_DESC dc;
	ZeroMemory(&dc, sizeof(dc));
	dc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	dc.ByteWidth = sizeof(NodeData) * NUM_DATA;
	dc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	dc.StructureByteStride = sizeof(NodeData);

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = dataA;

	pDevice->CreateBuffer(&dc, &InitData, &pBufferA);
	pDevice->CreateBuffer(&dc, NULL, &pBufferResultA);
	pDevice->CreateBuffer(&dc, NULL, &pBufferResultB);

	InitData.pSysMem = dataB;
	dc.ByteWidth = sizeof(EdgeData) * NUM_EDGE;
	dc.StructureByteStride = sizeof(EdgeData);
	pDevice->CreateBuffer(&dc, &InitData, &pBufferB);

	// SRV作成
	D3D11_SHADER_RESOURCE_VIEW_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	sd.BufferEx.FirstElement = 0;
	sd.Format = DXGI_FORMAT_UNKNOWN;
	sd.BufferEx.NumElements = NUM_DATA;
	pDevice->CreateShaderResourceView(pBufferA, &sd, &pBufferASRV);

	sd.BufferEx.NumElements = NUM_EDGE;
	pDevice->CreateShaderResourceView(pBufferB, &sd, &pBufferBSRV);

	// UAV作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC ud;
	ZeroMemory(&ud, sizeof(ud));
	ud.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	ud.Buffer.FirstElement = 0;
	ud.Format = DXGI_FORMAT_UNKNOWN;
	ud.Buffer.NumElements = NUM_DATA;

	pDevice->CreateUnorderedAccessView(pBufferResultA, &ud, &pBufferResultAUAV);
	pDevice->CreateUnorderedAccessView(pBufferResultB, &ud, &pBufferResultBUAV);

	//コンスタントバッファー作成
	ID3D11Buffer* pConstantBuffer;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.ByteWidth = sizeof(SIMPLESHADER_CONSTANT_BUFFER);
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;
	cbd.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(pDevice->CreateBuffer(&cbd, NULL, &pConstantBuffer)))
	{
		SAFE_RELEASE(pBufferA);
		SAFE_RELEASE(pBufferB);
		SAFE_RELEASE(pBufferResultA);
		SAFE_RELEASE(pBufferResultB);
		SAFE_RELEASE(pBufferASRV);
		SAFE_RELEASE(pBufferBSRV);
		SAFE_RELEASE(pBufferResultAUAV);
		SAFE_RELEASE(pBufferResultBUAV);
		delete[] dataA;
		delete[] dataB;
		return;
	}

	for (int itr = 0; itr < iteration; itr++) {
		// 定数を渡す
		D3D11_MAPPED_SUBRESOURCE cData;
		if (SUCCEEDED(pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cData))) {
			CBUFFER cb;
			cb.W = W;
			cb.L = L;
			cb.t = t;
			cb.k = k;
			memcpy_s(cData.pData, cData.RowPitch, (void*)&cb, sizeof(CBUFFER));
			pDeviceContext->Unmap(pConstantBuffer, 0);
		}
		pDeviceContext->CSSetConstantBuffers(0, 1, &pConstantBuffer);

		// CS実行1
		pDeviceContext->CSSetShader(pComputeShader1, 0, 0);
		pDeviceContext->CSSetShaderResources(0, 1, &pBufferASRV);
		pDeviceContext->CSSetShaderResources(1, 1, &pBufferBSRV);
		pDeviceContext->CSSetUnorderedAccessViews(0, 1, &pBufferResultAUAV, 0);
		pDeviceContext->CSSetUnorderedAccessViews(1, 1, &pBufferResultBUAV, 0);
		pDeviceContext->Dispatch(1, 1, 1);

		// CS実行2
		pDeviceContext->CSSetShader(pComputeShader2, 0, 0);
		pDeviceContext->Dispatch(1, 1, 1);

		// CS実行3
		pDeviceContext->CSSetShader(pComputeShader3, 0, 0);
		pDeviceContext->Dispatch(1, 1, 1);

		pDeviceContext->CSSetShader(0, 0, 0);

		SAFE_RELEASE(pConstantBuffer);

		// 値の取り出し
		ID3D11Buffer* pBufferCopy = NULL;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		pBufferResultA->GetDesc(&bd);
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		bd.Usage = D3D11_USAGE_STAGING;
		bd.BindFlags = 0;
		bd.MiscFlags = 0;
		pDevice->CreateBuffer(&bd, NULL, &pBufferCopy);
		pDeviceContext->CopyResource(pBufferCopy, pBufferResultA);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		NodeData* pData = NULL;
		pDeviceContext->Map(pBufferCopy, 0, D3D11_MAP_READ, 0, &MappedResource);
		pData = (NodeData*)MappedResource.pData;

		
		for (int i = 0; i < nodes.size(); i++) {
			nodes[i].x = pData[i].pos.x;
			nodes[i].y = pData[i].pos.y;
		}
		SAFE_RELEASE(pBufferCopy);

		t -= dt;
	}

	SAFE_RELEASE(pBufferA);
	SAFE_RELEASE(pBufferB);
	SAFE_RELEASE(pBufferResultA);
	SAFE_RELEASE(pBufferResultB);
	SAFE_RELEASE(pBufferASRV);
	SAFE_RELEASE(pBufferBSRV);
	SAFE_RELEASE(pBufferResultAUAV);
	SAFE_RELEASE(pBufferResultBUAV);
	delete[] dataA;
	delete[] dataB;
	
	for (int i = 0; i < nodes.size(); i++) {
		nodes[i].x *= WINDOW_WIDTH;
		nodes[i].y *= WINDOW_HEIGHT;
	}
	
	/*
	for (int i = 0; i < NUM_DATA; i++) {
		std::cout << "元データA: " << dataA[i].i << ", 元データB: " << dataB[i].i << ", 計算結果: " << pData[i].i << std::endl;
	}
	*/
	
	/*
	for (int i = 0; i < NUM_DATA; i++) {
		nodes[i].x = pData[i].i;
	}
	*/
}