#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <d3dCompiler.h>
#include "Global.h"

//���_�̍\����
struct SimpleVertex
{
	D3DXVECTOR3 Pos; //�ʒu
	D3DXVECTOR2 vTex; //�e�N�X�`���[���W
};

struct Point {
	D3DXVECTOR3 s;
	Point(float x, float y) : s(PIXELPOS(x, y)) {}
};
//Simple�V�F�[�_�[�p�̃R���X�^���g�o�b�t�@�[�̃A�v�����\���� �������V�F�[�_�[���̃R���X�^���g�o�b�t�@�[�ƈ�v���Ă���K�v����
struct SIMPLESHADER_CONSTANT_BUFFER
{
	D3DXMATRIX mWVP;
};

struct Node {
	float x, y; // �ʒu
	Node(float x, float y) : x(x), y(y) {}
};

struct Edge {
	int s, t; // �q���ł��钸�_�C���f�b�N�X
	float c; // �R�X�g�i�d�݁j
	Edge(int s, int t, float c) : s(s), t(t), c(c) {}
};