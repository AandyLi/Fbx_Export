#include "FBXData.h"

FBXData::FBXData() {
}

FBXData::~FBXData() {
	if (meshes != nullptr) {
		delete[] meshes;
	}
	if (skeletons != nullptr) {
		for (int skeletonI = 0; skeletonI < skeletonCount; skeletonI++) {
			for (int boneI = 0; boneI < skeletons[skeletonI].boneCount; boneI++) {
				delete[] skeletons[skeletonI].positionFrames[boneI];
				delete[] skeletons[skeletonI].rotationFrames[boneI];
			}
			delete[] skeletons[skeletonI].positionFrames;
			delete[] skeletons[skeletonI].rotationFrames;
		}

		delete[] skeletons;
	}
}

void FBXData::AllocateMeshes(int count) {
	meshes = new Mesh[meshCount + count];
	meshCount += count;
}

void FBXData::AllocateSkeletons(int count) {
	skeletons = new Skeleton[skeletonCount + count];
	skeletonCount += count;
}
