#include "FBXData.h"

FBXData::FBXData() {
}

FBXData::~FBXData() {
	if (meshes != nullptr) {
		delete[] meshes;
	}
}

void FBXData::AllocateMeshes(int count) {
	meshes = new Mesh[meshCount + count];
	meshCount += count;
}
