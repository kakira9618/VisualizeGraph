#include "CC.h"

cbuffer global : register(b0) 
{
	float W;
	float L;
	float t;
	float k;
};

struct Vec2 {
	float2 val;
};

struct Edge {
	int s;
	int t;
};

StructuredBuffer<Vec2> Pos : register(t0);
StructuredBuffer<Edge> Edges : register(t1);
RWStructuredBuffer<Vec2> NewPos : register(u0);
RWStructuredBuffer<Vec2> Vel : register(u1);

groupshared float2 g_vel[NODE_NUMV_DEFAULT];

float f_r(float d, float k) {
	return (k * k) / d;
}

float f_a(float d, float k) {
	return (d * d) / k;
}

[numthreads(NODE_NUMV_DEFAULT, 1, 1)]
void CalcR(uint3 GTid :SV_GroupThreadID) {
	
	int i = GTid.x;
	Vel[i].val = float2(0.0, 0.0);
	NewPos[i].val = float2(0.0, 0.0);
	for (int j = 0; j < NODE_NUMV_DEFAULT; j++) {
		if (i != j) {
			float2 d = Pos[i].val - Pos[j].val;
			float delta = length(d);
			if (abs(delta) < 0.001) delta = 0.001;
		
			float f = f_r(delta, k) / delta;
			Vel[i].val += f * d;
			
		}
	}
}


[numthreads(NODE_NUME_DEFAULT, 1, 1)]
void CalcA(uint3 GTid :SV_GroupThreadID) {
	int i = GTid.x;

	if (i == 0) {
		for (int j = 0; j < NODE_NUMV_DEFAULT; j++) {
			g_vel[j] = float2(0.0, 0.0);
		}
	}
	GroupMemoryBarrierWithGroupSync();

	int vi = Edges[i].s;
	int ui = Edges[i].t;
	float2 d = Pos[vi].val - Pos[ui].val;

	float delta = length(d);
	if (abs(delta) < 0.001) delta = 0.001;

	float f = f_a(delta, k) / delta;

	float2 dd = f * d;
	g_vel[vi] -= dd;
	g_vel[ui] += dd;
	GroupMemoryBarrierWithGroupSync();
	
	if (i == 0) {
		for (int j = 0; j < NODE_NUMV_DEFAULT; j++) {
			Vel[j].val += g_vel[j];
		}
	}
	GroupMemoryBarrierWithGroupSync();
	
}

[numthreads(NODE_NUMV_DEFAULT, 1, 1)]
void CalcPos(uint3 GTid :SV_GroupThreadID) {
	int i = GTid.x;
	
	float2 d = Vel[i].val;
	float disp = length(d);
	if (abs(disp) < 0.001) disp = 0.001;
	
	float f = min(disp, t) / disp;
	float2 newpos = Pos[i].val + f * d;
		
	newpos.x = min(W, max(0.0f, newpos.x)) - W / 2;
	newpos.y = min(L, max(0.0f, newpos.y)) - L / 2;
	//newpos.x = min(sqrt(max(W * W / 4 - newpos.y * newpos.y, 0.0f)), max(-sqrt(max(W * W / 4 - newpos.y * newpos.y, 0.0f)), newpos.x)) + W / 2;
	//newpos.y = min(sqrt(max(L * L / 4 - newpos.x * newpos.x, 0.0f)), max(-sqrt(max(L * L / 4 - newpos.x * newpos.x, 0.0f)), newpos.y)) + L / 2;
	//newpos.x = min(sqrt(W * W / 4 - newpos.y * newpos.y), max(-sqrt(W * W / 4 - newpos.y * newpos.y), newpos.x)) + W / 2;
	//newpos.y = min(sqrt(L * L / 4 - newpos.x * newpos.x), max(-sqrt(L * L / 4 - newpos.x * newpos.x), newpos.y)) + L / 2;
	newpos.x += W / 2;
	newpos.y += L / 2;
	NewPos[i].val = newpos;
	
	
}