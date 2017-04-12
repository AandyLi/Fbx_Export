#include "FBXData.h"

FBXData::FBXData() {
}

FBXData::~FBXData() {
	if (meshCount > 0) {
		delete[] meshes;
	}
}

void FBXData::AddMesh(int count) {
	Mesh* newA = new Mesh[meshCount + count];

	for (int mesh = 0; mesh < meshCount; mesh++) {
		newA[mesh] = meshes[mesh];
	}
	if (meshCount != 0) {
		delete[] meshes;
	}

	meshes = newA;
	meshCount += count;
}
