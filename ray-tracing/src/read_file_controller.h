/*
 * read_file_controller.h
 *
 *  Created on: Oct 24, 2021
 *      Author: marcusviniciusdafonsecaantunes
 */

#ifndef READ_FILE_CONTROLLER_H_
#define READ_FILE_CONTROLLER_H_

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "vec3.h"
#include "hittable_list.h"

class read_file_controller {
	//number of images
public:
	int number_images = 1;
	int number_of_forms = 0;
	std::string file_name = "";
	hittable_list list;
private:
	std::vector<std::string> out;
	int linhaMatriz = 0;

public:
	read_file_controller() {
		getFileName();

	}

	void splitLine(std::string const &str, const char delim,
			std::vector<std::string> &out) {
		// construct a stream from the string
		std::stringstream ss(str);

		std::string s;
		int j = 0;
		while (std::getline(ss, s, delim)) {
			out.push_back(s);
		}

		number_of_forms++;
		getGeometry();

		out.clear();
	}

	void getFileName() {

		cout
				<< "Enter the name of the file with extension, it must be inside the project root: "
				<< endl;

//		cin >> file_name;
		file_name = "formas.txt";

//		file_name = "imagens/imagem";
//		file_name += std::to_string(imag);
//		file_name += ".bmp";

		cout << "\n\nName of the file selected: " << file_name << endl;

		std::string line;
		std::ifstream file(file_name);

		while (std::getline(file, line)) {
			// using printf() in all tests for consistency
			printf("%s\n", line.c_str());

			if (line[0] != '#')
				splitLine(line, ' ', out);

		}

	}

	hittable getGeometry() {
		// [0] Forma Geométrica
		// [1] X
		// [2] Y
		// [3] Z
		// [4] Raio (No caso da Esfera)
		// [5] material
		// [6] R ou dispercao (dieletric)
		// [7] G
		// [8] B
		// [9] dispercao

		point3 coord(std::stod(out[1]), std::stod(out[2]), std::stod(out[3]));

		double raio = std::stod(out[4]);

		if (out[0] == "Esfera") {
			if (out[5] == "material_ground") {
				cout << "\nentrou no material_groud\n";
				auto material = make_shared<lambertian>(color(0.8, 0.8, 0.0));

				 list.add(make_shared<sphere>(coord, raio, material));

			} else if (out[5] == "material_center") {
				cout << "\nentrou no material_center\n";
				auto material = make_shared<lambertian>(color(0.7, 0.3, 0.3));
				list.add(make_shared<sphere>(coord, raio, material));

			} else if (out[5] == "material_left") {
				cout << "\nentrou no material_left\n";
				auto material = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
				list.add(make_shared<sphere>(coord, raio, material));

			} else if (out[5] == "material_right") {
				cout << "\nentrou no material_right\n";
				auto material = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
				list.add(make_shared<sphere>(coord, raio, material));
			}

		}
//
//		if (out[0] == "Esfera") {
//			if (out[5] == "lambertian") {
//				auto material = make_shared<lambertian>(
//						color(std::stod(out[6]), std::stod(out[7]),
//								std::stod(out[8])));
//
//				list.add(make_shared<sphere>(coord, raio, material));
//
//			} else if (out[5] == "metal") {
//
//				auto material = make_shared<metal>(
//						color(std::stod(out[6]), std::stod(out[7]),
//								std::stod(out[8])), std::stod(out[9]));
//				list.add(make_shared<sphere>(coord, raio, material));
//
//			}
//			else if (out[5] == "dieletric") {
//
//				auto material = make_shared<dieletric>(0.3);
//				list.add(make_shared<sphere>(coord, raio, material));
//
//			}

//		}

	}

	void setNumberImages() {

		cout << "Enter then number of images:" << endl;

		cin >> number_images;

		cout << "\nNumber: " << number_images << " \n";
	}

};

#endif /* READ_FILE_CONTROLLER_H_ */
