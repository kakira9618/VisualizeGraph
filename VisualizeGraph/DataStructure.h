#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <d3dCompiler.h>
#include "Global.h"

//頂点の構造体
struct SimpleVertex
{
	D3DXVECTOR3 Pos; //位置
	D3DXVECTOR2 vTex; //テクスチャー座標
};

struct Point {
	D3DXVECTOR3 s;
	Point(float x, float y) : s(PIXELPOS(x, y)) {}
};
//Simpleシェーダー用のコンスタントバッファーのアプリ側構造体 もちろんシェーダー内のコンスタントバッファーと一致している必要あり
struct SIMPLESHADER_CONSTANT_BUFFER
{
	D3DXMATRIX mWVP;
};

struct Node {
	float x, y; // 位置
	Node(float x, float y) : x(x), y(y) {}
};

struct Edge {
	int s, t; // 繋いでいる頂点インデックス
	float c; // コスト（重み）
	Edge(int s, int t, float c) : s(s), t(t), c(c) {}
};