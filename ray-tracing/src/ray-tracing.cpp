//============================================================================
// Name        : ray-tracing.cpp
// Author      : Marcus Vinicius
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <fstream>
#include <iostream>
#include <iomanip>
#include <thread>

#include "rtweekend.h"
#include "vec3.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"

#include "color.h"
#include "ray.h"
#include "camera.h"

#include "material.h"

#include "read_file_controller.h"

#include <string>
#include "moving_sphere.h"

using namespace std;

const int bytesPerPixel = 3; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 40;
unsigned char *img;
int line_size;
int cel_size;

char* createBitmapFileHeader(int height, int width, int paddingSize) {
	int fileSize = fileHeaderSize + infoHeaderSize
			+ (bytesPerPixel * width + paddingSize) * height;

	static unsigned char fileHeader[] = { 0, 0, /// signature
			0, 0, 0, 0, /// image file size in bytes
			0, 0, 0, 0, /// reserved
			0, 0, 0, 0, /// start of pixel array
			};

	fileHeader[0] = (unsigned char) ('B');
	fileHeader[1] = (unsigned char) ('M');
	fileHeader[2] = (unsigned char) (fileSize);
	fileHeader[3] = (unsigned char) (fileSize >> 8);
	fileHeader[4] = (unsigned char) (fileSize >> 16);
	fileHeader[5] = (unsigned char) (fileSize >> 24);
	fileHeader[10] = (unsigned char) (fileHeaderSize + infoHeaderSize);

	return (char*) fileHeader;
}

char* createBitmapInfoHeader(int height, int width) {
	static unsigned char infoHeader[] = { 0, 0, 0, 0, /// header size
			0, 0, 0, 0, /// image width
			0, 0, 0, 0, /// image height
			0, 0, /// number of color planes
			0, 0, /// bits per pixel
			0, 0, 0, 0, /// compression
			0, 0, 0, 0, /// image size
			0, 0, 0, 0, /// horizontal resolution
			0, 0, 0, 0, /// vertical resolution
			0, 0, 0, 0, /// colors in color table
			0, 0, 0, 0, /// important color count
			};

	infoHeader[0] = (unsigned char) (infoHeaderSize);
	infoHeader[4] = (unsigned char) (width);
	infoHeader[5] = (unsigned char) (width >> 8);
	infoHeader[6] = (unsigned char) (width >> 16);
	infoHeader[7] = (unsigned char) (width >> 24);
	infoHeader[8] = (unsigned char) (height);
	infoHeader[9] = (unsigned char) (height >> 8);
	infoHeader[10] = (unsigned char) (height >> 16);
	infoHeader[11] = (unsigned char) (height >> 24);
	infoHeader[12] = (unsigned char) (1);
	infoHeader[14] = (unsigned char) (bytesPerPixel * 8);

	return (char*) infoHeader;
}

void writeBitmapFile(ofstream &out, int width, int height) {
	char padding[3] = { 0, 0, 0 };
	char color[3] = { 0, 0, 0 };
	int paddingSize = (4 - (width * bytesPerPixel) % 4) % 4;

	char *fileHeader = createBitmapFileHeader(height, width, paddingSize);
	char *infoHeader = createBitmapInfoHeader(height, width);

	out.write((char*) fileHeader, fileHeaderSize);
	out.write((char*) infoHeader, infoHeaderSize);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int p = (height - y) * line_size + x * cel_size;
			color[2] = img[p];
			color[1] = img[p + 1];
			color[0] = img[p + 2];
			out.write(color, sizeof(color));
		}
		out.write(padding, paddingSize);

	}
}

double hit_sphere(const point3 &center, double radius, const ray &r) {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;

	if (discriminant < 0) {
		return -1.0;
	} else {
		return (-half_b - sqrt(discriminant)) / a;
	}
}

color ray_color(const ray &r, const hittable &world, int depth) {
	hit_record rec;
	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void trace_lines(int thread_id, int start_line, int end_line, int image_width,
		int image_height, camera *cam, hittable_list world, image *img) {
	const int samples_per_pixel = 100;
	const int max_depth = 50;

//	ofstream outfile;
//		outfile.open("image.ppm");

	for (int j = start_line; j < end_line; j++) {
//		std::cerr << "\rThread: " << thread_id << "- Scanlines remaining: " << j
//				<< ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam->get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
//				write_color(outfile, pixel_color, samples_per_pixel);
			img->set(i, image_height - j, pixel_color, samples_per_pixel);
		}
	}
//	outfile.close();
}

void imprimir(int idthread, int numberline) {
	std::cerr << 'id da thread:' << idthread << ' - ' << numberline;
}

hittable_list random_scene() {
	hittable_list world;

	auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -3; a < 3; a++) {
		for (int b = -3; b < 3; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2,
					b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + vec3(0, random_double(0, .5), 0);
					world.add(
							make_shared<moving_sphere>(center, center2, 0.0,
									1.0, 0.2, sphere_material));
					world.add(
							make_shared<sphere>(center, 0.2, sphere_material));
				} else if (choose_mat < 0.95) {
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(
							make_shared<sphere>(center, 0.2, sphere_material));
				} else {
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(
							make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

int main() {
	// Image
	const int samples_per_pixel = 100;
	const int max_depth = 35;
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 260;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int number_threads = 8;
	const int lines_thread = image_height / number_threads;
	int start = 0;
	image img(image_width, image_height);
//	read_file_controller readFile;

	//conjunto de threads
//	    std::vector<std::thread> thread_vec;

	// Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;

	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus,
			0.0, 1.0);

	// World
	auto world = random_scene();
	auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
	auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
	auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
	auto material_die = make_shared<dielectric>(1.5);

//	world.add(
//			make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0,
//					material_ground));
//	world.add(make_shared<sphere>(point3(-1.0, 1.0, -1.5), 0.5, material_die));
//	world.add(make_shared<sphere>(point3(-1.0, 1.0, -1.5), -0.4, material_die));
//	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
//	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

//	for(int i = 0; i < readFile.number_of_forms; i++){
//		world.add(readFile.getGeometry());
//	}

	//	#Colocar o esquema de ler o arquivo linha por linha
	//[0,2,5],[

	double x = -7;
	double z = 0;

	for (int imag = 0; imag < 50; imag++) {

		if(x <= -7){
			z = imag * 0.2;
		}
		if(z >= 7){
			x = imag * 0.2;
		}
		if(x >= 7){
			z= imag * -0.2;
		}
		if(z <= -7){
			x = imag * -0.2;
		}

		cout << "( " << x << "," << z << ")";

		point3 lookfrom(x , 2, z);
		point3 lookat(0, 0, 0);
		vec3 vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera cam(lookfrom, lookat, vup, 30, aspect_ratio, aperture,
				dist_to_focus, 0.0, 1.0);

		std::cout << "\nNovaImagem" << imag << "\n";

		std::string file_name = "imagens/imagem";
		file_name += std::to_string(imag);
		file_name += ".bmp";

		ofstream outfile;
		outfile.open(file_name);

		//Thread

		std::vector<std::thread> threads;

		for (int i = 0; i < number_threads; i++) {
			if (i + 1 == number_threads)
				//Colocar o objeto em memória
				threads.push_back(
						thread(trace_lines, i, start, image_height, image_width,
								image_height, &cam, world, &img));
			else
				//		  			threads.push_back(thread(imprimir, i, start));
				threads.push_back(
						thread(trace_lines, i, start, start + lines_thread,
								image_width, image_height, &cam, world, &img));

			start += lines_thread;
		}

		for (auto &th : threads) {
			th.join();
		}

		start = 0;

		img.writeBitmapFile(outfile);
		outfile.close();
	}

	std::cerr << "\nDone.\n";

}
