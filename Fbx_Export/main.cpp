#pragma comment (lib, "libfbxsdk.lib")

#include <fbxsdk.h>
#include <iostream>

#include "FBXData.h"

using namespace std;

enum col {above, below, nocoll}colvar;

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
						FbxVector4 meshPos = node->LclTranslation;
						int* array = mesh->GetPolygonVertices();

						for (int i = 0; i < mesh->GetPolygonVertexCount(); i++)
						{
							FbxVector4 position = mesh->GetControlPointAt(array[i])+ meshPos;
							FbxVector4 lNormal = mesh->GetElementNormal()->GetDirectArray().GetAt(i);
							FbxVector2 uv = mesh->GetElementUV()->GetDirectArray().GetAt(array[i]);
							

							data.meshes[data.meshCount - 1].vertices[i].position[0] = position[0];
							data.meshes[data.meshCount - 1].vertices[i].position[1] = position[1];
							data.meshes[data.meshCount - 1].vertices[i].position[2] = position[2];
							data.meshes[data.meshCount - 1].vertices[i].uv[0] = uv[0];
							data.meshes[data.meshCount - 1].vertices[i].uv[1] = uv[1];
							data.meshes[data.meshCount - 1].vertices[i].normal[0] = lNormal[0];
							data.meshes[data.meshCount - 1].vertices[i].normal[1] = lNormal[1];
							data.meshes[data.meshCount - 1].vertices[i].normal[2] = lNormal[2];

							cout << position[0] << ", " << position[1] << ", " << position[2] << endl;
							cout << uv[0] << ", " << uv[1] << endl;
							cout << lNormal[0] << ", " << lNormal[1] << ", " << lNormal[2] << endl;


							FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(0);

							FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
							
						
							FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(0));

							cout << texture->GetRelativeFileName() << endl;
							

							FbxProperty pAbove = node->FindProperty("Collision_above", false);
							FbxProperty pBelow = node->FindProperty("Collision_below", false);
							FbxProperty pMesh = node->FindProperty("Collision_mesh", false);
							
							if (pAbove.IsValid()) {
								colvar = above;
							}
							if (pBelow.IsValid()) {
								colvar = below;
							}
							if (pMesh.IsValid()) {
								colvar = nocoll;
							}

							switch (colvar)
							{
							case above:
								cout << "Collision is above!" << endl;
								break;
							
							case below:
								cout << "Collision is below!" << endl;
								break;
							
							case nocoll:
								cout << "Collision is none!" << endl;
								break;
							}

							
						}
						getchar();
					}
				}
			}
		}
	}

	getchar();
	manager->Destroy();
	return 0;
}
