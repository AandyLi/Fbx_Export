#pragma comment (lib, "libfbxsdk.lib")

#include <fbxsdk.h>
#include <iostream>
#include <fstream>
#include <string>

#include "FBXData.h"

using namespace std;

void StoreChildJoints(FBXData& data, int skeletonId, FbxNode* parentNode) {
	for (int child = 0; child < parentNode->GetChildCount(); child++) {
		FbxNode* node = parentNode->GetChild(child);
		for (int attribute = 0; attribute < node->GetNodeAttributeCount(); attribute++) {
			if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton) {
				data.skeletons[skeletonId].AllocateBone();

				for (unsigned int frame = 0; frame < data.skeletons[skeletonId].frameCount; frame++) {
					FbxTime time = FbxTime(FBXSDK_TC_MILLISECOND * frame * 1000 / 60.0);

					FbxDouble3 framePosition = node->LclTranslation.EvaluateValue(time, false);
					data.skeletons[skeletonId].positionFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3] = framePosition[0];
					data.skeletons[skeletonId].positionFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 1] = framePosition[1];
					data.skeletons[skeletonId].positionFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 2] = -framePosition[2];

					FbxDouble3 frameRotation = node->LclRotation.EvaluateValue(time, false);
					data.skeletons[skeletonId].rotationFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3] = -frameRotation[0] * 0.0174533;
					data.skeletons[skeletonId].rotationFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 1] = -frameRotation[1] * 0.0174533;
					data.skeletons[skeletonId].rotationFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 2] = frameRotation[2] * 0.0174533;
				}

				StoreChildJoints(data, skeletonId, node);
			}
		}
	}
}

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	FBXData data;
	char** files;
	int fileCount = 0;
	std::string fileName = "";
	float** meshOffsets;

	ifstream inFile("input.txt");
	if (inFile.is_open()) {
		string line;
		while (getline(inFile, line)) {
			if (line.substr(0, 5) == "Load:") {
				while (getline(inFile, line) && line.substr(0, 17) != "Position Offsets:") {
					if (line[0] != '#' && line.substr(line.find_last_of(".") + 1, 3) == "fbx") {
						{
							fileName = line.substr(line.find_last_of('\\') + 1);
							fileName.erase(4);
							char** newA = new char*[fileCount + 1];
							for (int file = 0; file < fileCount; file++) {
								newA[file] = files[file];
							}
							if (fileCount != 0) {
								delete[] files;
							}
							files = newA;
							fileCount++;
						}
						files[fileCount - 1] = new char[line.length() + 1];
						for (int letter = 0; letter < line.length(); letter++) {
							files[fileCount - 1][letter] = line[letter];
						}
						files[fileCount - 1][line.length()] = '\0';
					}
				}
				if (fileCount) {
					meshOffsets = new float*[fileCount];
				}
				for (int offset = 0; offset < fileCount; offset++) {
					meshOffsets[offset] = new float[3];
					meshOffsets[offset][0] = 0; meshOffsets[offset][1] = 0; meshOffsets[offset][2] = 0;
				}

			}
			if (line.substr(0, 17) == "Position Offsets:") {
				for (int offset = 0; offset < fileCount && getline(inFile, line) && line[0] != '#' && line[0] != ' ' && line != "" && line.substr(0, 5) != "Load:"; offset++) {
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

	int meshCount = 0, skeletonCount = 0;
	{
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
								meshCount++;
							}
						}
						else if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eNull) {
							for (int childI = 0; childI < node->GetChildCount(); childI++) {
								FbxNode* childNode = node->GetChild(childI);
								for (int attributeI = 0; attributeI < childNode->GetNodeAttributeCount(); attributeI++) {
									if (childNode->GetNodeAttributeByIndex(attributeI)->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
										FbxMesh* mesh = childNode->GetMesh();
										if (mesh->IsTriangleMesh()) {
											meshCount++;
										}
									}
								}
							}
						}
						else if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton) {
							skeletonCount++;
						}
					}
				}
			}
		}
	}

	data.AllocateMeshes(meshCount);
	data.AllocateSkeletons(skeletonCount);

	int meshId = 0, skeletonId = 0, groupId = 0;
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
							data.meshes[meshId].AllocateVertices(mesh->GetPolygonVertexCount());

							FbxProperty idProperty = node->FindProperty("id", false);
							FbxProperty collisionProperty = node->FindProperty("Collision", false);
							if (idProperty.IsValid()) {
								data.meshes[meshId].id = idProperty.Get<FbxInt>();
							}
							if (collisionProperty.IsValid()) {
								data.meshes[meshId].customAttribute = collisionProperty.Get<FbxEnum>();
							}

							FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(0);
							FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
							FbxFileTexture* texture = (FbxFileTexture*)property.GetSrcObject<FbxFileTexture>(0);
							if (texture) {
								data.meshes[meshId].texturePath = texture->GetRelativeFileName();
								data.meshes[meshId].texturePath = data.meshes[meshId].texturePath.substr(data.meshes[meshId].texturePath.find_last_of('\\') + 1);
							}

							FbxDouble3 meshPos = node->LclTranslation;
							int* vertexIndices = mesh->GetPolygonVertices();
							for (int vertex = 0; vertex < mesh->GetPolygonVertexCount(); vertex++) {
								int vertexOffset = 0;
								if (vertex % 3 == 1) {
									vertexOffset = 1;
								}
								else if (vertex % 3 == 2) {
									vertexOffset = -1;
								}

								FbxVector4 position = mesh->GetControlPointAt(vertexIndices[vertex]);
								position[0] += meshPos[0]; position[1] += meshPos[1]; position[2] += meshPos[2];
								data.meshes[meshId].vertices[vertex + vertexOffset].position[0] = position[0] + meshOffsets[file][0];
								data.meshes[meshId].vertices[vertex + vertexOffset].position[1] = position[1] + meshOffsets[file][1];
								data.meshes[meshId].vertices[vertex + vertexOffset].position[2] = -position[2] + meshOffsets[file][2];
							}

							const char* uvSetName = mesh->GetElementUV()->GetName();
							unsigned int polyCount = mesh->GetPolygonCount();
							for (unsigned int poly = 0; poly < polyCount; poly++) {
								for (unsigned int vertex = 0; vertex < 3; vertex++) {
									unsigned int vertexOffset = 0;
									if (vertex == 1) {
										vertexOffset = 1;
									}
									else if (vertex == 2) {
										vertexOffset = -1;
									}

									FbxVector2 uv; bool unmapped;
									mesh->GetPolygonVertexUV(poly, vertex, uvSetName, uv, unmapped);
									data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].uv[0] = uv[0];
									data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].uv[1] = uv[1];

									FbxVector4 normal;
									mesh->GetPolygonVertexNormal(poly, vertex, normal);
									data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].normal[0] = normal[0];
									data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].normal[1] = normal[1];
									data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].normal[2] = -normal[2];
								}
							}

							for (int deformerI = 0; deformerI < mesh->GetDeformerCount(); deformerI++) {
								FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerI, FbxDeformer::eSkin);

								if (skin) {
									for (int clusterI = 0; clusterI < skin->GetClusterCount(); clusterI++) {
										FbxCluster& cluster = *skin->GetCluster(clusterI);

										int connectedVertexCount = cluster.GetControlPointIndicesCount();
										int* connectedVertices = cluster.GetControlPointIndices();
										double* connectedVertexWeights = cluster.GetControlPointWeights();

										for (int connectedVI = 0; connectedVI < connectedVertexCount; connectedVI++) {
											for (int vertexI = 0; vertexI < data.meshes[meshId].vertexCount; vertexI++) {
												if (connectedVertices[connectedVI] == vertexIndices[vertexI]) {
													int vertexOffset = 0;
													if (vertexI % 3 == 1) {
														vertexOffset = 1;
													}
													else if (vertexI % 3 == 2) {
														vertexOffset = -1;
													}

													for (int connectedJI = 0; connectedJI < 4; connectedJI++) {
														if (data.meshes[meshId].vertices[vertexI + vertexOffset].weights[connectedJI] == 0) {
															data.meshes[meshId].vertices[vertexI + vertexOffset].weights[connectedJI] = connectedVertexWeights[connectedVI];
															break;
														}
													}
												}
											}
										}
									}
								}
							}

							meshId++;
						}
					}
					else if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton) {
						FbxTimeSpan timeSpan; float animationStart = -1; float animationEnd = -1;
						bool positionAnimated = node->LclTranslation.IsAnimated(); bool rotationAnimated = node->LclRotation.IsAnimated();

						if (positionAnimated) {
							node->LclTranslation.GetCurveNode()->GetAnimationInterval(timeSpan);
							animationStart = timeSpan.GetStart().GetSecondDouble();
							animationEnd = timeSpan.GetStop().GetSecondDouble();
						}
						if (rotationAnimated) {
							node->LclRotation.GetCurveNode()->GetAnimationInterval(timeSpan);
							if (timeSpan.GetStart().GetSecondDouble() < animationStart || animationStart == -1) {
								animationStart = timeSpan.GetStart().GetSecondDouble();
							}
							if (timeSpan.GetStop().GetSecondDouble() > animationEnd || animationEnd == -1) {
								animationEnd = timeSpan.GetStop().GetSecondDouble();
							}
						}

						if (animationEnd) {
							data.skeletons[skeletonId].AllocateBone(animationEnd);

							for (unsigned int frame = 0; frame < (unsigned int)((animationEnd)* 60 + 1); frame++) {
								FbxTime time = FbxTime(FBXSDK_TC_MILLISECOND * frame * 1000 / 60.0);

								FbxDouble3 framePosition = node->LclTranslation.EvaluateValue(time, false);
								data.skeletons[skeletonId].positionFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3] = framePosition[0];
								data.skeletons[skeletonId].positionFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 1] = framePosition[1];
								data.skeletons[skeletonId].positionFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 2] = -framePosition[2];

								FbxDouble3 frameRotation = node->LclRotation.EvaluateValue(time, false);
								data.skeletons[skeletonId].rotationFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3] = -frameRotation[0] * 0.0174533;
								data.skeletons[skeletonId].rotationFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 1] = -frameRotation[1] * 0.0174533;
								data.skeletons[skeletonId].rotationFrames[data.skeletons[skeletonId].boneCount - 1][frame * 3 + 2] = frameRotation[2] * 0.0174533;
							}
						}

						StoreChildJoints(data, skeletonId, node);

						skeletonId++;
					}
					else if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eCamera) {
						FbxCamera* camera = node->GetCamera();
						FbxDouble3 position = camera->Position.Get();
						FbxDouble3 lookPosition = camera->EvaluateLookAtPosition();

						data.camera.position[0] = position[0]; data.camera.position[1] = position[1]; data.camera.position[2] = -position[2];
						data.camera.lookPosition[0] = lookPosition[0]; data.camera.lookPosition[1] = lookPosition[1]; data.camera.lookPosition[2] = -lookPosition[2];
					}
					else if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eLight) {
						FbxDouble3 position = node->LclTranslation.Get();
						data.light.position[0] = position[0]; data.light.position[1] = position[1]; data.light.position[2] = -position[2];
					}
					else if (node->GetNodeAttributeByIndex(attribute)->GetAttributeType() == FbxNodeAttribute::EType::eNull) {
						bool meshGroup = false;
						for (int childI = 0; childI < node->GetChildCount(); childI++) {
							FbxNode* childNode = node->GetChild(childI);
							for (int attributeI = 0; attributeI < childNode->GetNodeAttributeCount(); attributeI++) {
								if (childNode->GetNodeAttributeByIndex(attributeI)->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
									FbxMesh* mesh = childNode->GetMesh();

									if (mesh->IsTriangleMesh()) {
										data.meshes[meshId].AllocateVertices(mesh->GetPolygonVertexCount());

										data.meshes[meshId].group = groupId;
										meshGroup = true;

										FbxProperty idProperty = node->FindProperty("id", false);
										FbxProperty collisionProperty = node->FindProperty("Collision", false);
										if (idProperty.IsValid()) {
											data.meshes[meshId].id = idProperty.Get<FbxInt>();
										}
										if (collisionProperty.IsValid()) {
											data.meshes[meshId].customAttribute = collisionProperty.Get<FbxEnum>();
										}

										FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(0);
										if (material) {
											FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
											FbxFileTexture* texture = (FbxFileTexture*)property.GetSrcObject<FbxFileTexture>(0);
											if (texture) {
												data.meshes[meshId].texturePath = texture->GetRelativeFileName();
												data.meshes[meshId].texturePath = data.meshes[meshId].texturePath.substr(data.meshes[meshId].texturePath.find_last_of('\\') + 1);
											}
										}

										FbxDouble3 meshPos = node->LclTranslation;
										int* vertexIndices = mesh->GetPolygonVertices();
										for (int vertex = 0; vertex < mesh->GetPolygonVertexCount(); vertex++) {
											int vertexOffset = 0;
											if (vertex % 3 == 1) {
												vertexOffset = 1;
											}
											else if (vertex % 3 == 2) {
												vertexOffset = -1;
											}

											FbxVector4 position = mesh->GetControlPointAt(vertexIndices[vertex]);
											position[0] += meshPos[0]; position[1] += meshPos[1]; position[2] += meshPos[2];
											data.meshes[meshId].vertices[vertex + vertexOffset].position[0] = position[0] + meshOffsets[file][0];
											data.meshes[meshId].vertices[vertex + vertexOffset].position[1] = position[1] + meshOffsets[file][1];
											data.meshes[meshId].vertices[vertex + vertexOffset].position[2] = -position[2] + meshOffsets[file][2];
										}

										const char* uvSetName = mesh->GetElementUV()->GetName();
										unsigned int polyCount = mesh->GetPolygonCount();
										for (unsigned int poly = 0; poly < polyCount; poly++) {
											for (unsigned int vertex = 0; vertex < 3; vertex++) {
												unsigned int vertexOffset = 0;
												if (vertex == 1) {
													vertexOffset = 1;
												}
												else if (vertex == 2) {
													vertexOffset = -1;
												}

												FbxVector2 uv; bool unmapped;
												mesh->GetPolygonVertexUV(poly, vertex, uvSetName, uv, unmapped);
												data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].uv[0] = uv[0];
												data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].uv[1] = uv[1];

												FbxVector4 normal;
												mesh->GetPolygonVertexNormal(poly, vertex, normal);
												data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].normal[0] = normal[0];
												data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].normal[1] = normal[1];
												data.meshes[meshId].vertices[poly * 3 + vertex + vertexOffset].normal[2] = -normal[2];
											}
										}

										for (int deformerI = 0; deformerI < mesh->GetDeformerCount(); deformerI++) {
											FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerI, FbxDeformer::eSkin);

											if (skin) {
												for (int clusterI = 0; clusterI < skin->GetClusterCount(); clusterI++) {
													FbxCluster& cluster = *skin->GetCluster(clusterI);

													int connectedVertexCount = cluster.GetControlPointIndicesCount();
													int* connectedVertices = cluster.GetControlPointIndices();
													double* connectedVertexWeights = cluster.GetControlPointWeights();

													for (int connectedVI = 0; connectedVI < connectedVertexCount; connectedVI++) {
														for (int vertexI = 0; vertexI < data.meshes[meshId].vertexCount; vertexI++) {
															if (connectedVertices[connectedVI] == vertexIndices[vertexI]) {
																int vertexOffset = 0;
																if (vertexI % 3 == 1) {
																	vertexOffset = 1;
																}
																else if (vertexI % 3 == 2) {
																	vertexOffset = -1;
																}

																for (int connectedJI = 0; connectedJI < 4; connectedJI++) {
																	if (data.meshes[meshId].vertices[vertexI + vertexOffset].weights[connectedJI] == 0) {
																		data.meshes[meshId].vertices[vertexI + vertexOffset].weights[connectedJI] = connectedVertexWeights[connectedVI];
																		break;
																	}
																}
															}
														}
													}
												}
											}
										}

										meshId++;
									}
								}
							}
						}

						if (meshGroup) {
							groupId++;
						}
					}
				}
			}
		}
	}

	if (fileCount > 0)
	{
		std::ofstream os(fileName + ".gay", std::ios::binary);

		char temp[76];
		std::string temp2;


		os.write((char*)&data.meshCount, sizeof(int));

		std::cout << data.meshCount << std::endl;

		for (int i = 0; i < data.meshCount; i++)
		{
			//Meshes

			data.meshes[i].texturePath += '\0';
			data.meshes[i].strLength = data.meshes[i].texturePath.size();

			os.write((char*)&data.meshes[i].strLength, (sizeof(int)));
			os.write((char*)data.meshes[i].texturePath.c_str(), data.meshes[i].strLength);

			std::cout << data.meshes[i].strLength << std::endl;
			std::cout << data.meshes[i].texturePath << std::endl;

			data.meshes[i].vertSize = sizeof(Vertex) * data.meshes[i].vertexCount;

			os.write((char*)&data.meshes[i].vertexCount, sizeof(int));
			os.write((char*)&data.meshes[i].vertSize, sizeof(int));

			std::cout << data.meshes[i].vertexCount << std::endl;
			std::cout << data.meshes[i].vertSize << std::endl;

			os.write((char*)(data.meshes[i].vertices), data.meshes[i].vertSize);

			// Bounding Boxes
			os.write((char*)&data.meshes[i].customAttribute, sizeof(int));

			// id
			os.write((char*)&data.meshes[i].id, sizeof(int));

		}

		os.close();

		//std::cout << data.meshes[0].vertices[0].position[0];

		manager->Destroy();
		for (int allocation = 0; allocation < fileCount; allocation++) {
			delete[] files[allocation];
			delete[] meshOffsets[allocation];
		}
		if (fileCount) {
			delete[] files;
			delete[] meshOffsets;
		}
	}

	getchar();
	return 0;
}
