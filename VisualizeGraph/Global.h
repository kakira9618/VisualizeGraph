#pragma once

//必要なライブラリファイルのロード
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11.lib")
#pragma comment(lib,"d3dCompiler.lib")
//警告非表示
#pragma warning(disable : 4305)
#include "CC.h"

//マクロ
#define SAFE_RELEASE(x) if(x){(x)->Release(); (x);}
#define PIXELPOS(x,y) D3DXVECTOR3( (x)/(WINDOW_WIDTH/2.0f)-1.0f, -(y)/(WINDOW_HEIGHT/2.0f)+1.0f, 0.0f )