#pragma once


#include "Ray.h"

class Camera {
public:
	Vec3 origin;
	Vec3 lower_left_corner;
	Vec3 vertical;
	Vec3 horizontal;

	Camera(float vfov, float aspect) {
		/*
		float theta = vfov * 3.14 / 180;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;
		*/
		lower_left_corner = Vec3(-2.0, -1.0, -1.0);
		horizontal = Vec3(2 * 2.0, 0.0, 0.0);
		vertical = Vec3(0.0, 2.0 * 1, 0.0);
		origin = Vec3(0.0, 0.0, 0.0f);
	}

	Ray get_ray(float u, float v) {
		return Ray(origin, lower_left_corner + u* horizontal + v*vertical - origin);
	};
};