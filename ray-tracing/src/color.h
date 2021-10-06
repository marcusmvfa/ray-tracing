/*
 * colorClass.h
 *
 *  Created on: Aug 29, 2021
 *      Author: marcusviniciusdafonsecaantunes
 */
#ifndef COLOR_H_
#define COLOR_H_

#include "vec3.h"

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

class image {
	int width;
	int height;
	unsigned char *img;
	int line_size;
	int cel_size;
public:
	image(int w, int h) :
			width(w), height(h) {
		img = new unsigned char[width * height * 3 * sizeof(int)];
		cel_size = sizeof(unsigned char) * 3;
		line_size = cel_size * width;
	}
	~image() {
		delete img;
	}
	void set(int x, int y, color pixel_color, int samples_per_pixel) {
		auto r = pixel_color.x();
		auto g = pixel_color.y();
		auto b = pixel_color.z();

		// Divide the color total by the number of samples and gamma-correct for gamma=2.0.
		auto scale = 1.0 / samples_per_pixel;
		r = sqrt(scale * r);
		g = sqrt(scale * g);
		b = sqrt(scale * b);

		unsigned char *pos = &img[y * line_size + x * cel_size];

		//rgb para arquivos bitmap
		//rbg para arquivos ppm
		pos[2] = (unsigned char)(255 * clamp(r, 0.0, 0.999));
		pos[1] = (unsigned char)(255 * clamp(g, 0.0, 0.999));
		pos[0] = (unsigned char)(255 * clamp(b, 0.0, 0.999));
	}
	void write_colors(std::ostream &out) {
		unsigned char *pos;

		out << setfill('0');
		out << "P3\n" << setw(5) << width << " " << setw(5) << height << "\n255\n";

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				pos = &img[y * line_size + x * cel_size];
				out << setw(3) << int{pos[0]} << " " << setw(3) << int{pos[1]} << " " << setw(3) << int{pos[2]} << "   ";
			}
			out << "\n";
		}
	}

	void writeBitmapFile(ofstream &out) {
		char padding[3] = { 0, 0, 0 };
		char color[3] = { 0, 0, 0 };
		int paddingSize = (4 - (width * bytesPerPixel) % 4) % 4;

		char *fileHeader = createBitmapFileHeader(height, width,
				paddingSize);
		char *infoHeader = createBitmapInfoHeader(height, width);

		out.write( (char*)fileHeader, fileHeaderSize );
		out.write( (char*)infoHeader, infoHeaderSize );

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int p = (height-y) * line_size + x * cel_size;
				color[0] = img[p];
				color[1] = img[p+1];
				color[2] = img[p+2];
				out.write( color, sizeof(color) );
			}
			out.write( padding, paddingSize );

		}
	}

private:
	const int bytesPerPixel = 3; /// red, green, blue
	const int fileHeaderSize = 14;
	const int infoHeaderSize = 40;


	char* createBitmapFileHeader(int height, int width,
			int paddingSize) {
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

		return (char*)fileHeader;
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

		return (char*)infoHeader;
	}
};

void write_color(std::ostream &out, color pixel_color) {
	// Write the translated [0,255] value of each color component.
	out << static_cast<int>(255.999 * pixel_color.x()) << ' '
			<< static_cast<int>(255.999 * pixel_color.y()) << ' '
			<< static_cast<int>(255.999 * pixel_color.z()) << '\n';
}

void write_color(std::ostream &out, color pixel_color, int samples_per_pixel) {
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	// Divide the color total by the number of samples and gamma-correct for gamma=2.0.
	auto scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	// Write the translated [0,255] value of each color component.
	out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
			<< static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
			<< static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

#endif /* COLOR_H_ */
