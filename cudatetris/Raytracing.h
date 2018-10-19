#pragma once
#include <Windows.h>
class Hitable;
class HitableList;
class Camera;

class RaytraceEnviroment {
private:
	Hitable** list;
	HitableList* world;
	Camera* cam;
	int width;
	int height;
public:
	RaytraceEnviroment(int, int);
	int ray_tracing_main(uint32_t* back_buffer, int width, int height, int count);
};


