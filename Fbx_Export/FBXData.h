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
	string texturePath;

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
