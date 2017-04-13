#include "FBXData.h"

FBXData::FBXData() {
}

FBXData::~FBXData() {
	if (meshCount != 0) {
		delete[] meshes;
	}
}

void FBXData::AllocateMeshes(int count) {
	meshes = new Mesh[meshCount + count];
	meshCount += count;
}
