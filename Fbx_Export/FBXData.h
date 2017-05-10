#pragma once
#include <string>
using namespace std;
struct Vertex {
	float position[3];
	float uv[2];
	float normal[3];
};


class Mesh {
public:
	Vertex* vertices;
	int vertexCount = 0;
	int strLength;
	unsigned int vertSize = 0;
	string texturePath;

	// 0 - Not a Bounding Box
	// 1 - Above ground Bounding Box
	// 2 - Below ground Bounding Box
	// 3 - Above and Below ground Bounding Box
	// 4 - Pressure Plate Bounding Box
	// 5 - Lever Bounding Box
	int customAttribute = 0;

	Mesh() {}
	virtual ~Mesh() {
		if (vertexCount != 0) {
			delete[] vertices;

		}
	}

	void AllocateVertices (int count) {
		vertices = new Vertex[count];
		vertexCount += count;
	}
};

class FBXData {
public:
	Mesh* meshes;
	int meshCount = 0;

	FBXData();
	virtual ~FBXData();

	void AllocateMeshes(int);
};
