#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include "DataStructure.h"


class Fruchterman {
	static float f_r(float d, float k);
	static float f_a(float d, float k);
public:
	static void calc(std::vector<Node> &nodes, std::vector<Edge> &edges, int iteration = 50, int itrmax = 0, int nowitr = 0);
	static void calcWithGPU(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, std::vector<Node> &nodes, std::vector<Edge> &edges, int iteration = 50, int itrmax = 0, int nowitr = 0);
};