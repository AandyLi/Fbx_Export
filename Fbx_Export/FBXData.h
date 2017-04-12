#pragma once

struct Vertex {
	float position[3];
	float uv[2];
	float normal[3];
};

class Mesh {
public:
	Vertex* vertices;
	int vertexCount = 0;

	~Mesh() {
		if (vertexCount != 0) {
			delete[] vertices;
		}
	}

	void addVertex(int count) {
		Vertex* newA = new Vertex[vertexCount + count];

		for (int vertex = 0; vertex < vertexCount; vertex++) {
			newA[vertex] = vertices[vertex];
		}
		if (vertexCount != 0) {
			delete[] vertices;
		}

		vertices = newA;
		vertexCount += count;
	}
};

class FBXData {
public:
	Mesh* meshes;
	int meshCount = 0;

	FBXData();
	~FBXData();

	void AddMesh(int);
};
