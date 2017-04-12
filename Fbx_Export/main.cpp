#pragma comment (lib, "libfbxsdk.lib")

#include <fbxsdk.h>
#include <iostream>

#include "FBXData.h"

using namespace std;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(30547);

	FBXData data;

	const char* fileName = "file.fbx";

	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ioSettings);

	FbxImporter* importer = FbxImporter::Create(manager, "");
	if (!importer->Initialize(fileName, -1, manager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		getchar();
		exit(-1);
	}

	FbxScene* scene = FbxScene::Create(manager, "myScene");
	importer->Import(scene);
	importer->Destroy();

	FbxNode* rootNode = scene->GetRootNode();
	if (rootNode) {
		for (int child = 0; child < rootNode->GetChildCount(); child++) {
			FbxNode* node = rootNode->GetChild(child);
			for (int attribute = 0; attribute < node->GetNodeAttributeCount(); attribute++) {
				if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
					FbxMesh* mesh = node->GetMesh();
					if (mesh->IsTriangleMesh()) {
						data.AddMesh(1);
						data.meshes[data.meshCount - 1].addVertex(mesh->GetPolygonVertexCount());
					}
				}
			}
		}
	}

	getchar();
	manager->Destroy();
	return 0;
}
