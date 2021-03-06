//
// Created by Jiang Yinzuo on 2021/6/24.
//

#include "vec3.h"
#include "constants.h"

vec3 random_in_unit_sphere() {
  while (true) {
	auto p = vec3::random(-1, 1);
	if (p.length_squared() >= 1) continue;
	return p;
  }
}

/**
 * Lambertian Reflection
 */
vec3 random_unit_vector() {
  auto a = random_double(0, 2 * pi);
  auto z = random_double(-1, 1);
  auto r = sqrt(1 - z * z);
  return vec3(r * cos(a), r * sin(a), z);
}

vec3 random_in_hemisphere(const vec3 &normal) {
  vec3 in_unit_sphere = random_in_unit_sphere();
  if (dot(in_unit_sphere, normal) > 0.0)// In the same hemisphere as the normal
	return in_unit_sphere;
  else
	return -in_unit_sphere;
}

bool vec3::near_zero() const {
  // Return true if the vector is close to zero in all dimensions.
  const auto s = 1e-8;
  return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
}

vec3 reflect(const vec3 &v, const vec3 &n) {
  return v - 2 * dot(v, n) * n;
}

vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
  auto cos_theta = fmin(dot(-uv, n), 1.0);
  vec3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
  vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

vec3 random_in_unit_disk() {
  while (true) {
	auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
	if (p.length_squared() >= 1) continue;
	return p;
  }
}