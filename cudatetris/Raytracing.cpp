// Raytracing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <time.h>
#include <stdio.h>
#include <iostream>
#include "Sphere.h"
#include "HitableList.h"
#include "Camera.h"
#include "Material.h"
#include <float.h>
#include "Raytracing.h"
#include <Windows.h>
#include <future>


RaytraceEnviroment::RaytraceEnviroment(int nx, int ny) {
	this->list = (Hitable**)malloc(sizeof(Hitable*) * 5);
	this->list[0] = new Sphere(Vec3(0, 0, -1), 0.5, new Lambertian(Vec3(0.8, 0.3, 0.3)));
	this->list[1] = new Sphere(Vec3(0, -100.5, -1), 100, new Lambertian(Vec3(0.8, 0.8, 0.0)));
	this->list[2] = new Sphere(Vec3(1, 0, -1), 0.5, new Metal(Vec3(0.8, 0.6, 0.2), 0.3));
	this->list[3] = new Sphere(Vec3(-1, 0, -1), 0.5, new Dielectric(1.5));
	this->list[4] = new Sphere(Vec3(-1, 0, -1), -0.45, new Dielectric(1.5));
	this->cam = new Camera(90.0, float(nx) / float(ny));
	this->world = new HitableList(list, 5);
	this->width = nx;
	this->height = ny;
	
}

Vec3 color(const Ray& ray, Hitable* world, int depth) {
	hit_record rec;

	if (depth > 5) {
		return Vec3(0.0, 0.0, 0.0);
	}

	if (world->hit(ray, 0.001, FLT_MAX, rec)) {
		Ray scattered;
		Vec3 attenuation;

		if (depth < 50 && rec.mat_ptr->scatter(ray, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return Vec3(0.0, 0.0, 0.0);
		}
	}
	else {
		Vec3 unit_direction = unit_vector(ray.direction());
		float t = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
	}
}

struct RayTraceTreadConfig {
	HitableList* world;
	Camera* cam;
	uint32_t* back_buffer;
	int width;
	int height;
	int x;
	int y;
	int number_of_samples;
	int threadIndex;
};

struct SetPixelPacket {
	int x;
	int y;
	int index;
	float r;
	float g;
	float b;
	int width;
	int height;
};

void set_pixel(uint32_t* back_buffer, SetPixelPacket* spp) {
	if (spp->x >= 0 && spp->x < spp->width) {
		if (spp->y >= 0 && spp->y < spp->height) {
			int ir = int(255.99 * spp->r);
			int ig = int(255.99 * spp->g);
			int ib = int(255.99 * spp->b);
			back_buffer[spp->index] =  ir << 16 | ig << 8 | ib;
		}
	}
}

void single_pixel(RayTraceTreadConfig* rttc) {
	
	Vec3 col(0, 0, 0);

	for (int i = 0; i < rttc->number_of_samples; i++) {
		float u = float(rttc->x + drand48()) / (float(rttc->width));
		float v = float(rttc->y + drand48()) / float(rttc->height);
		Ray r = rttc->cam->get_ray(u, v);
		Vec3 p = r.point_at_parameter(2.0);

		col += color(r, rttc->world, 0);
	}

	col /= float(rttc->number_of_samples);
	SetPixelPacket spp;
	spp.x = rttc->x;
	spp.y = rttc->height - rttc->y;
	spp.height = rttc->height;
	spp.width = rttc->width;
	spp.r = col[0];
	spp.g = col[1];
	spp.b = col[2];
	spp.index = rttc->threadIndex;
	set_pixel(rttc->back_buffer, &spp);
}

void single_thread(RayTraceTreadConfig* configs, int number_of_configs){
	for (int i = 0; i < number_of_configs; i++)
	{
		single_pixel(&configs[i]);
//		configs += sizeof(RayTraceTreadConfig);
	}
}


int dispatch_thread_configs(RayTraceTreadConfig* thread_configs, int number_of_configs, int number_of_threads) {
	int returned_threads = 0;
	int dispatch_threads = 0;
	std::vector<std::thread> threads;

	for (int i = 0; i < number_of_threads; i++) {
		threads.push_back(std::thread(single_pixel, &thread_configs[dispatch_threads]));//THIS UGLY CHANGE HAS TO CHANGE
		dispatch_threads++;
	}
	
	//basic idea right, needs some clean up
	while (returned_threads != dispatch_threads) {
		for (int i = 0; i < threads.size(); i++) {
			if (threads[i].joinable()) {
				//this is bad lock logic
				threads[i].join();
				std::thread* dead_thread = &threads[i];
				if (dispatch_threads != number_of_configs) {
					threads[i].swap(std::thread(single_pixel, &thread_configs[dispatch_threads]));
					dispatch_threads++;
				}
				//delete dead_thread;
				returned_threads++;
			}
		}
	}

	return returned_threads;
}

int RaytraceEnviroment::ray_tracing_main(uint32_t* back_buffer, int width, int height, int count)
{
	int nx = width;
	int ny = height;
	int ns = 4;
	
	srand(time(NULL));

	unsigned concurentThreadsSupported = std::thread::hardware_concurrency() - 2;

	RayTraceTreadConfig* rayTraceThreadConfig = (RayTraceTreadConfig*)malloc(sizeof(RayTraceTreadConfig) * height * width);
	int index = 0;

	for (int y = height - 1; y >= 0; y--)
	{
		for (int x = 0; x < width; x++)
		{
			RayTraceTreadConfig* rttc = &rayTraceThreadConfig[index];
			rttc->back_buffer = back_buffer;
			rttc->cam = cam;
			rttc->world = world;
			rttc->width = width;
			rttc->height = height;
			rttc->number_of_samples = ns;
			rttc->x = x;
			rttc->y = y;
			rttc->threadIndex = index;
			index += 1;
		}
	}

	std::vector<std::thread> threads;
	int config_stride = height * width / 1;

	for (int i = 0; i < 1; i++) {
		threads.push_back(std::thread(single_thread, &rayTraceThreadConfig[i * config_stride], config_stride));
	}

	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}

	free(rayTraceThreadConfig);
	return 0;
}
