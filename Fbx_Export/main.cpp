#pragma comment (lib, "libfbxsdk.lib")

#include <fbxsdk.h>
#include <iostream>
#include <fstream>
#include <string>

#include "FBXData.h"

using namespace std;

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	FBXData data;
	const char** files;
	int fileCount = 0;
	float** meshOffsets;

	ifstream inFile("input.txt");
	if (inFile.is_open()) {
		string line;
		while (getline(inFile, line)) {
			if (line.substr(0, 5) == "Load:") {
				while (getline(inFile, line) && line.substr(line.find_last_of(".") + 1, 3) == "fbx") {
					if (line[0] != '#') {
						{
							const char** newA = new const char*[fileCount + 1];
							for (int file = 0; file < fileCount; file++) {
								newA[file] = files[file];
							}
							if (fileCount != 0) {
								delete[] files;
							}
							files = newA;
							fileCount++;
						}
						files[fileCount - 1] = line.c_str();
					}
				}
			}
			if (fileCount) {
				meshOffsets = new float*[fileCount];
			}
			for (int offset = 0; offset < fileCount; offset++) {
				meshOffsets[offset] = new float[3];
				meshOffsets[offset][0] = 0; meshOffsets[offset][1] = 0; meshOffsets[offset][2] = 0;
			}
			if (line.substr(0, 17) == "Position Offsets:") {
				for (int offset = 0; offset < fileCount && getline(inFile, line) && line[0] != '#' && line[0] != ' ' && line != ""; offset++) {
					float x = stof(line.substr(0, line.find_first_of(' ')));
					float y = stof(line.substr(line.find_first_of(' ') + 1, line.find_last_of(' ')));
					float z = stof(line.substr(line.find_last_of(' ') + 1));
					meshOffsets[offset][0] = x; meshOffsets[offset][1] = y; meshOffsets[offset][2] = z;
				}
			}
		}
	}
	inFile.close();

	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ioSettings);

	for (int file = 0; file < fileCount; file++) {
		FbxImporter* importer = FbxImporter::Create(manager, "");
		if (!importer->Initialize(files[file], -1, manager->GetIOSettings())) {
			printf("Call to FbxImporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
			getchar();
			exit(-1);
		}

		FbxScene* scene = FbxScene::Create(manager, "scene");
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

							FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(0);
							FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
							FbxFileTexture* texture = (FbxFileTexture*)property.GetSrcObject<FbxFileTexture>(0);
							data.meshes[data.meshCount - 1].texturePath = texture->GetRelativeFileName();

							FbxDouble3 meshPos = node->LclTranslation;
							int* vertexIndices = mesh->GetPolygonVertices();
							for (int vertex = 0; vertex < mesh->GetPolygonVertexCount(); vertex++) {
								FbxVector4 position = mesh->GetControlPointAt(vertexIndices[vertex]);
								position[0] += meshPos[0]; position[1] += meshPos[1]; position[2] += meshPos[2];
								data.meshes[data.meshCount - 1].vertices[vertex].position[0] = position[0] + meshOffsets[file][0];
								data.meshes[data.meshCount - 1].vertices[vertex].position[1] = position[1] + meshOffsets[file][1];
								data.meshes[data.meshCount - 1].vertices[vertex].position[2] = position[2] + meshOffsets[file][2];

								FbxVector2 uv = mesh->GetElementUV()->GetDirectArray().GetAt(vertexIndices[vertex]);
								data.meshes[data.meshCount - 1].vertices[vertex].uv[0] = uv[0];
								data.meshes[data.meshCount - 1].vertices[vertex].uv[1] = uv[1];

								FbxVector4 normal = mesh->GetElementNormal()->GetDirectArray().GetAt(vertex);
								data.meshes[data.meshCount - 1].vertices[vertex].normal[0] = normal[0];
								data.meshes[data.meshCount - 1].vertices[vertex].normal[1] = normal[1];
								data.meshes[data.meshCount - 1].vertices[vertex].normal[2] = normal[2];
							}
						}
					}
				}
			}
		}
	}

	for (int allocation = 0; allocation < fileCount; allocation++) {
		delete files[allocation];
		delete meshOffsets[allocation];
	}
	if (fileCount) {
		delete[] files;
		delete[] meshOffsets;
	}
	manager->Destroy();
	return 0;
}
