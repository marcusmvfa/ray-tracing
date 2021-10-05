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
//#include <thread>

#include "rtweekend.h"
#include "vec3.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"


#include "color.h"
#include "ray.h"
#include "camera.h"

#include "material.h"

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

double hit_sphere(const point3& center, double radius, const ray& r) {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius*radius;
        auto discriminant = half_b*half_b - a*c;

        if (discriminant < 0) {
            return -1.0;
        } else {
            return (-half_b - sqrt(discriminant) ) / a;
        }
}

color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
    	return color(0,0,0);

    if (world.hit(r, 0.001, infinity, rec)) {
    	ray scattered;
    	        color attenuation;
    	        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    	            return attenuation * ray_color(scattered, world, depth-1);
    	        return color(0,0,0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}

void trace_lines(int thread_id, int start_line, int end_line, int image_width, int image_height, camera *cam, hittable_list world, std::ostream outfile){
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	for (int j = start_line; j < end_line; j++) {
			std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
			for (int i = 0; i < image_width; ++i) {
				color pixel_color(0, 0, 0);
				for (int s = 0; s < samples_per_pixel; ++s) {
					auto u = (i + random_double()) / (image_width-1);
					auto v = (j + random_double()) / (image_height-1);
					ray r = cam->get_ray(u, v);
					pixel_color += ray_color(r, world, max_depth);
				}
				write_color(outfile, pixel_color, samples_per_pixel);
			}
		}
}

int main() {
	// Image
	const int samples_per_pixel = 100;
		const int max_depth = 50;
	    const auto aspect_ratio = 16.0 / 9.0;
	    const int image_width = 350;
	    const int image_height = static_cast<int>(image_width / aspect_ratio);
	    const int number_threads = 2;
	    const int lines_thread = image_height / number_threads;
	    int start = 0;
	    //conjunto de threads
//	    std::vector<std::thread> thread_vec;

	    std::cout << "height da imagem " << image_height;
	    std::cerr << "\nDone.\n";


	    // Camera
	    camera cam;


	    // World
	    hittable_list world;
	    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	       auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
	       auto material_left   = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
	           auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);

	       world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
	       world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
	       world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
	       world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

	    // Camera

	    auto viewport_height = 2.0;
	    auto viewport_width = aspect_ratio * viewport_height;
	    auto focal_length = 1.0;

	    auto origin = point3(0, 0, 0);
	    auto horizontal = vec3(viewport_width, 0, 0);
	    auto vertical = vec3(0, viewport_height, 0);
	    auto lower_left_corner = origin - horizontal/2 - vertical/2 - vec3(0, 0, focal_length);

	// Render

	ofstream outfile;
	outfile.open("image.ppm");

	//	#Colocar o esquema de ler o arquivo linha por linha


	//Thread


	//Utilizando threads
//
//	for(int i =0; i < number_threads; i++){
//
//		if(i + 1 == number_threads)
//			//Colocar o objeto em memória
//			thread_vec[i] = thread(trace_lines, i, start, image_height, image_width, image_height, &cam, world, outfile);
//		else
//			thread_vec[i] = thread(trace_lines, i, start, start + lines_thread, image_width, image_height, &cam, world, outfile);
//
//		start += lines_thread;
//	}
//
//	for(int i = 0; i < number_threads; i ++){
//		thread_vec[i].join();
//
//	}

//
	for (int j = image_height - 1; j >= 0; --j) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (image_width-1);
				auto v = (j + random_double()) / (image_height-1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			write_color(outfile, pixel_color, samples_per_pixel);
		}
	}
	std::cerr << "\nDone.\n";

	outfile.close();

	outfile.open("image.bmp", ios::binary | ios::out);
	writeBitmapFile(outfile, image_width, image_height);
	outfile.close();
}
