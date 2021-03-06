//
// Created by Jiang Yinzuo on 2021/6/25.
//

#include "camera.h"
#include "constants.h"
#include "utils.h"
#include "lambertian.h"
#include "metal.h"
#include "color.h"
#include "hittable_list.h"
#include "shape/sphere.h"
#include "dielectric.h"

#include <memory>
#include <iostream>
#include <thread>

using std::make_shared;

color ray_color(const Ray &r, const Hittable &world, int depth) {
  hit_record rec;

  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
	return kBlackColor;

  if (world.hit(r, 0.001, infinity, rec)) {
	Ray scattered;
	color attenuation;
	if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
	  return attenuation * ray_color(scattered, world, depth - 1);
	return color(0, 0, 0);
  }

  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
  hittable_list world;

  auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  world.add(make_shared<Sphere>(point3(0, -1000, 0), 1000, ground_material));

  for (int a = -4; a < 4; a++) {
	for (int b = -4; b < 4; b++) {
	  auto choose_mat = random_double();
	  point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

	  if ((center - point3(4, 0.2, 0)).length() > 0.9) {
		shared_ptr<material> sphere_material;

		if (choose_mat < 0.8) {
		  // diffuse
		  auto albedo = color::random() * color::random();
		  sphere_material = make_shared<lambertian>(albedo);
		  world.add(make_shared<Sphere>(center, 0.2, sphere_material));
		} else if (choose_mat < 0.95) {
		  // metal
		  auto albedo = color::random(0.5, 1);
		  auto fuzz = random_double(0, 0.5);
		  sphere_material = make_shared<Metal>(albedo, fuzz);
		  world.add(make_shared<Sphere>(center, 0.2, sphere_material));
		} else {
		  // glass
		  sphere_material = make_shared<dielectric>(1.5);
		  world.add(make_shared<Sphere>(center, 0.2, sphere_material));
		}
	  }
	}
  }

  auto material1 = make_shared<dielectric>(1.5);
  world.add(make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  world.add(make_shared<Sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = make_shared<Metal>(color(0.7, 0.6, 0.5), 0.0);
  world.add(make_shared<Sphere>(point3(4, 1, 0), 1.0, material3));

  return world;
}

class Render {
 private:
  // Image
  constexpr static double aspect_ratio = 3.0 / 2.0;
  constexpr static int image_width = 400;
  constexpr static int image_height = static_cast<int>(image_width / aspect_ratio);
  constexpr static int samples_per_pixel = 500;
  constexpr static int max_depth = 50;
 public:
  explicit Render(const char *file_name) : world_(random_scene()) {
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;

	cam_ = new Camera(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);
	jpg_writer_ = new JPGWriter(file_name, image_width, image_height);
  }

  Render(Render &) = delete;
  Render &operator=(Render &) = delete;

  void Start() {
	int image_height_mid = image_height / 2;
	std::thread t1(Render::RenderTask,
	this, image_height - 1, image_height_mid);
	std::thread t2(Render::RenderTask,
	this, image_height_mid - 1, 0);

	t1.join();
	t2.join();
  }

  ~Render() {
	delete cam_;
	delete jpg_writer_;
  }

 private:
  hittable_list world_;
  Camera *cam_;
  JPGWriter *jpg_writer_;

  static void RenderTask(Render *render, int image_height_max, int image_height_min) {
	for (int j = image_height_max; j >= image_height_min; --j) {
	  std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
	  for (int i = 0; i < image_width; ++i) {
		color pixel_color(0, 0, 0);
		for (int s = 0; s < samples_per_pixel; ++s) {
		  auto u = (i + random_double()) / (image_width - 1);
		  auto v = (j + random_double()) / (image_height - 1);
		  Ray r = render->cam_->get_ray(u, v);
		  pixel_color += ray_color(r, render->world_, max_depth);
		}
		render->jpg_writer_->WriteColor(pixel_color, i, image_height_max - j, samples_per_pixel);
	  }
	}
  }
};

int main() {
  Render r{"render.jpg"};
  r.Start();
  std::cerr << "\nDone.\n";
  return 0;
}