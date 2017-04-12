#include "FBXData.h"

FBXData::FBXData() {
}

FBXData::~FBXData() {
	delete[] meshes;
}

void FBXData::AddMesh(int count) {
	Mesh* newA = new Mesh[meshCount + 1];

	for (int mesh = 0; mesh < meshCount; mesh++) {
		newA[mesh] = meshes[mesh];
	}
	if (meshCount != 0) {
		delete[] meshes;
	}

	meshes = newA;
	meshCount++;
}
