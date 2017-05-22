#pragma once
#include <string>
using namespace std;

struct Light {
	float position[3] = { 0, 0, 0 };
};

struct Camera {
	float position[3] = { 0, 0, 0 };
	float lookPosition[3] = { 0, 0, 0 };
};

class Skeleton {
public:
	int boneCount = 0;
	int frameCount = 0;

	float** positionFrames = nullptr;
	float** rotationFrames = nullptr;

	void AllocateBone(float animationEnd = -1) {
		float** new_positionFrames = new float*[boneCount + 1];
		float** new_rotationFrames = new float*[boneCount + 1];
		for (int boneI = 0; boneI < boneCount; boneI++) {
			new_positionFrames[boneI] = positionFrames[boneI];
			new_rotationFrames[boneI] = rotationFrames[boneI];
		}
		delete[] positionFrames; delete[] rotationFrames;
		positionFrames = new_positionFrames; rotationFrames = new_rotationFrames;

		if (animationEnd != -1) {
			frameCount = animationEnd * 60 + 1;
		}

		positionFrames[boneCount] = new float[frameCount * 3];
		rotationFrames[boneCount] = new float[frameCount * 3];

		boneCount++;
	}
};

struct Vertex {
	float position[3];
	float uv[2];
	float normal[3];

	float weights[4] = { 0, 0, 0, 0 };
};

class Mesh {
public:
	Vertex* vertices;
	int vertexCount = 0;
	int strLength = 0;
	unsigned int vertSize = 0;
	string texturePath;

	// 0 - Not a Bounding Box
	// 1 - Above ground Bounding Box
	// 2 - Below ground Bounding Box
	// 3 - Above and Below ground Bounding Box
	// 4 - Pressure Plate Bounding Box
	// 5 - Lever Bounding Box
	int customAttribute = 0;
	int id = -1;

	int group = -1;

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
	Mesh* meshes = nullptr;
	int meshCount = 0;

	Skeleton* skeletons = nullptr;
	int skeletonCount = 0;

	Camera camera;
	Light light;

	FBXData();
	virtual ~FBXData();

	void AllocateMeshes(int);
	void AllocateSkeletons(int);
};
