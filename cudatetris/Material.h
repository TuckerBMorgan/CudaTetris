#pragma once

#include "Ray.h"
#include "Hitable.h"


class Material {
public:
	virtual bool scatter(const Ray& ray_in, const hit_record& rec, Vec3& attenuation, Ray& scattered) const = 0;
};


class Lambertian : public Material {
public:
	Vec3 albedo;

	Lambertian(const Vec3& a) : albedo(a) {};
	virtual bool scatter(const Ray& r_in, const hit_record& rec, Vec3& attenuation, Ray& scattered) const {
		Vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}
};

class Metal : public Material {
public: 
	Vec3 albedo;
	float fuzz;
	Metal(const Vec3& a, float f) : albedo(a) {
		if (f < 1) {
			fuzz = f;
		}
		else {
			fuzz = 1;
		}
	};
	virtual bool scatter(const Ray& r_in, const hit_record& rec, Vec3& attenuation, Ray& scattered) const {
		Vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = Ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}
};


float schlick(float cosine, float ref_index) {
	float r0 = (1 - ref_index) / (1 + ref_index);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}
class Dielectric : public Material {
public:
	float ref_index;
	Dielectric(float ri) : ref_index(ri) {};

	virtual bool scatter(const Ray& r_in, const hit_record& rec, Vec3& attenuation, Ray& scattered) const {
		Vec3 outward_normal;
		Vec3 reflected = reflect(r_in.direction(), rec.normal);
		float ni_over_nt;
		attenuation = Vec3(1.0, 1.0, 1.0);
		Vec3 refracted;
		float reflect_prob;
		float cosine;
		

		if (dot(r_in.direction(), rec.normal) > 0) {
			outward_normal = -rec.normal;
			ni_over_nt = ref_index;
			cosine = ref_index * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_index;
			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}

		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			reflect_prob = schlick(cosine, ref_index);
		}
		else {
			scattered = Ray(rec.p, reflected);
			reflect_prob = 1.0;
		}
		if (drand48() < reflect_prob) {
			scattered = Ray(rec.p, reflected);
		}
		else {
			scattered = Ray(rec.p, refracted);
		}

		return true;
	}

};