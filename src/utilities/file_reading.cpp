#include "file_reading.h"

#include <iostream>

#include <fstream>
#include <sstream>
#include <string>

// https://www.scratchapixel.com/lessons/3d-basic-rendering/obj-file-format//obj-file-format.html
bool load_obj_file(const std::filesystem::path &objFile, TriangleMeshFileData &data){
    std::ifstream file(objFile);
    if (!file) return false;

    std::string line;

    while(std::getline(file, line)){
        float x, y, z;
        int a, b, c, d;
        std::string tmp;
        std::string type;

        std::istringstream reader(line);

        // skip empty lines
        if(!(reader >> type)) continue;
            
        else if (type == "v" && (reader >> x >> y >> z)){
            data.positions.push_back(x);
            data.positions.push_back(y);
            data.positions.push_back(z);
        }
        else if (type == "vn" && (reader >> x >> y >> z)){
            data.normals.push_back(x);
            data.normals.push_back(y);
            data.normals.push_back(z);
        }
        else if (type == "vt" && (reader >> x >> y)){
            data.uvs.push_back(x);
            data.uvs.push_back(y);
        }
        else if (type == "f" && (reader >> a >> tmp >> b >> tmp >> c)){
            a = a > 0 ? a - 1 : (data.positions.size() / 3) - a;
            b = b > 0 ? b - 1 : (data.positions.size() / 3) - b;
            c = c > 0 ? c - 1 : (data.positions.size() / 3) - c;

            data.faces.push_back(a);
            data.faces.push_back(b);
            data.faces.push_back(c);
        }
    }

    return true;
}

bool load_tet_gen_file(
    const std::filesystem::path &nodePath,
    const std::filesystem::path &elementPath,
    const std::filesystem::path &facePath, 
    TetGenFileData &mesh){
	
	std::ifstream nodeFile(nodePath), elementFile(elementPath), faceFile(facePath);
    if (!nodeFile || !elementFile || !faceFile) return false;

	
    std::string line;

	// read the nodes
	// throw away the header
	std::getline(nodeFile, line);
    while(std::getline(nodeFile, line)){
		int id;
		float x, y, z;
	 	std::istringstream reader(line);
		reader >> id >> x >> y >> z;
		mesh.positions.push_back(x);
		mesh.positions.push_back(y);
		mesh.positions.push_back(z);
	}

	// read the faces
	// throw away the header
	std::getline(faceFile, line);
    while(std::getline(faceFile, line)){
		int id;
		int a, b, c;
	 	std::istringstream reader(line);
		reader >> id >> a >> b >> c;
		mesh.faces.push_back(a - 1);
		mesh.faces.push_back(b - 1);
		mesh.faces.push_back(c - 1);
	}

	// read the elements
	// throw away the header
	std::getline(elementFile, line);
    while(std::getline(elementFile, line)){
		int id;
		int a, b, c, d;
	 	std::istringstream reader(line);
		reader >> id >> a >> b >> c >> d;
		mesh.elements.push_back(a - 1);
		mesh.elements.push_back(b - 1);
		mesh.elements.push_back(c - 1);
		mesh.elements.push_back(d - 1);
	}

	// tet gen element models can also provide the mid points of each edge
	// they will always appear after the 4 main vertices of the tetrahedron
	// I won't handle that case since I don't need that data 
	mesh.element_size = 4;

    return true;
}