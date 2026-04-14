/************************************************************************************
 *																					*
 * Copyright (C) 2020 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/


#include "RTMeshImporterComponent.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include "RTMeshData.h"
#include "ImageUtils.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "RuntimeMeshCore.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"

// Sets default values for this component's properties
URTMeshImporterComponent::URTMeshImporterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void URTMeshImporterComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

ARTMeshActor* URTMeshImporterComponent::ImportMesh(const FString Path)
{	
	const FTransform ImportTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(1.0f, 1.0f, 1.0f));

	return ImportMeshWithTransform(Path, ImportTransform);
}

ARTMeshActor* URTMeshImporterComponent::ImportMeshWithTransform(const FString Path, const FTransform& ImportTransform)
{	
	// Start read the file	
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(TCHAR_TO_UTF8(*Path), aiProcessPreset_TargetRealtime_MaxQuality & (~aiProcess_SplitLargeMeshes));

	if (Scene == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot open file: %s"), *Path);
		UE_LOG(LogTemp, Error, TEXT("Import Error: %s"), UTF8_TO_TCHAR(Importer.GetErrorString()));
		return nullptr;
	}
	else
	{
		ARTMeshActor* MeshActor = GetWorld()->SpawnActor<ARTMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		//MeshActor->MeshName = FPaths::GetCleanFilename(Path);
		MeshActor->ImportedPath = Path;

		// Process assimp node
		ProcessNode(Scene->mRootNode, Scene, MeshActor, ImportTransform);

		return MeshActor;
	}
}

void URTMeshImporterComponent::Open3DFileDialog(bool& bIsFileSelected, TArray<FString>& OutFiles)
{
	void* ParentWindowHandle = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		//A value of 0 represents single file selection while a value of 1 represents multiple file selection
		uint32 SelectionFlags = 0;

		// Supported formats
		FString FileTypes = TEXT("Autodesk|*.fbx|AutoCAD DXF|*.dxf|Collada|*.dae;*.xml|glTF|*.gltf;*.glb|Blender|*.blend|3D Studio Max|*.3ds;*.ase|Wavefront Object|*obj");
		FileTypes.Append("|Industry Foundation Classes|*.ifc|XGL|*.xgl;*.zgl|Stanford Polygon Library|*.ply|LightWave|*.lwo;*.lws");
		FileTypes.Append("|Modo|*.lxo|Stereolithography|*.stl|DirectX X|*.x|AC3D|*.ac|Milkshape 3D|*.ms3d|TrueSpace|*.cob;*.scn|Biovision BVH|*.bvh|CharacterStudio Motion|*.csm|Quick3D|*.q3o;*.q3s");
		//	FileTypes.Append("|Raw Triangles|*.raw|Irrlicht|*.irrmesh;*.irr;*.xml|Ogre|*.mesh.xml;*.skeleton.xml;*.material");
		//	FileTypes.Append("|Valve Model|*.smd;*.vta|Quake I|*.mdl|Quake II|*.md2|Quake III|*.md3;*.pk3|RtCW|*.mdc|Doom 3|*.md5mesh;");		
		//	FileTypes.Append("|Object File Format|*.off|Terragen Terrain|*.ter|3D GameStudio|*.mdl;*.hmp");	
		FileTypes.Append("|All Files|*.*");

		bIsFileSelected = DesktopPlatform->OpenFileDialog(ParentWindowHandle, "Select a 3D file to import", FPaths::GetProjectFilePath(), FString(""), FileTypes, SelectionFlags, OutFiles);
	}
}

void URTMeshImporterComponent::OpenTextureDialog(bool& bIsFileSelected, TArray<FString>& OutFiles)
{
	void* ParentWindowHandle = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		// A value of 0 represents single file selection while a value of 1 represents multiple file selection
		uint32 SelectionFlags = 0;

		//FString FileTypes = TEXT("Portable Network Graphics|*.png|JPEG|*.jpg|Targa|*.tga|Bitmap|*.bmp|Photoshop|*.psd");
		FString FileTypes = TEXT("Portable Network Graphics|*.png|JPEG|*.jpg");
		FileTypes.Append("|All Files|*.*");

		bIsFileSelected = DesktopPlatform->OpenFileDialog(ParentWindowHandle, "Select a texture file to import", FPaths::GetProjectFilePath(), FString(""), FileTypes, SelectionFlags, OutFiles);
	}

	
}

void URTMeshImporterComponent::SaveRTMeshesToFile(FString Filename)
{
	// Saved RTMeshActors
	TArray<FRTSaveMeshRecord> SaveActors;

	// Find all RTMeshActors on the map
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTMeshActor::StaticClass(), Actors);

	// Saving each actor to record
	for (auto Actor : Actors)
	{
		// Cast to RTMeshActor
		ARTMeshActor* RTMesh = Cast<ARTMeshActor>(Actor);
		FRTSaveMeshRecord MeshRecord;
		
		// Save all necessary data to MeshRecord
		RTMesh->SaveMeshToRecord(MeshRecord);

		// Write actor's 'SaveGame' property to byte array
		FMemoryWriter MemoryWriter(MeshRecord.ActorData, true);
		FRTSaveActorArchive Ar(MemoryWriter);
		
		// Serialize actor
		RTMesh->Serialize(Ar);

		// Add to saved list
		SaveActors.Add(MeshRecord);
	}

	// Create save file data
	FRTSaveFileRecord SaveFileRecord;
	SaveFileRecord.SaveVersion = BuildVersion;
	SaveFileRecord.Timestamp = FDateTime::Now();
	SaveFileRecord.SaveActors = SaveActors;

	FBufferArchive BinaryData;
	BinaryData << SaveFileRecord;
	
	// Compress File 
	//tmp compressed data array 
	TArray<uint8> CompressedData;
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(CompressedData, NAME_Zlib, ECompressionFlags::COMPRESS_BiasMemory);
	//Send entire binary array/archive to compressor 
	Compressor << BinaryData;
	//send archive serialized data to binary array 
	Compressor.Flush();

	// Save binary to file
	FString SaveFile = FPaths::ProjectSavedDir() / Filename;

	if (FFileHelper::SaveArrayToFile(CompressedData, *SaveFile))
	{
		UE_LOG(LogTemp, Warning, TEXT("Save Sucess! %s"), *SaveFile);		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Save Failed!"));
	}

	CompressedData.Empty();
	Compressor.FlushCache();	
	Compressor.Close();		
	BinaryData.FlushCache();
	BinaryData.Empty();
	BinaryData.Close();
}

void URTMeshImporterComponent::LoadRTMeshesFromFile(FString Filename)
{
	FString SaveFile = FPaths::ProjectSavedDir() / Filename;

	// Load data from file
	TArray<uint8> BinaryData;
	if (!FFileHelper::LoadFileToArray(BinaryData, *SaveFile))
	{
		UE_LOG(LogTemp, Warning, TEXT("Load Failed!"));
		return;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Load Succeeded!"));
	}
	
	// Decompress File 
	FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(BinaryData, NAME_Zlib, ECompressionFlags::COMPRESS_BiasMemory);
	//Decompression Error? 
	if (Decompressor.GetError())
	{
		UE_LOG(LogTemp, Warning, TEXT("FArchiveLoadCompressedProxy>> ERROR : File Was Not Compressed!"));
		return;		
	}

	//Decompress 
	FBufferArchive DecompressedBinaryArray;
	Decompressor << DecompressedBinaryArray;

	FMemoryReader FromBinary = FMemoryReader(DecompressedBinaryArray, true);
	FromBinary.Seek(0);

	FRTSaveFileRecord SaveFileRecord;
	FromBinary << SaveFileRecord;

	// Clean up
	BinaryData.Empty();	
	Decompressor.FlushCache();
	Decompressor.Close();
	DecompressedBinaryArray.Empty();
	DecompressedBinaryArray.Close();
	FromBinary.FlushCache();
	FromBinary.Close();
	
	// If the build version is not the same as the saved version so we won't process loading
	if (BuildVersion == SaveFileRecord.SaveVersion)
	{
		for (FRTSaveMeshRecord MeshRecord : SaveFileRecord.SaveActors)
		{
			FVector SpawnPos = MeshRecord.ActorTransform.GetLocation();
			FRotator SpawnRot = MeshRecord.ActorTransform.Rotator();
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = MeshRecord.ActorName;

			ARTMeshActor* NewMesh = GetWorld()->SpawnActor<ARTMeshActor>(ARTMeshActor::StaticClass(), SpawnPos, SpawnRot, SpawnParams);

			FMemoryReader MemoryReader(MeshRecord.ActorData, true);
			FRTSaveActorArchive Ar(MemoryReader);
			NewMesh->Serialize(Ar);
			NewMesh->SetActorTransform(MeshRecord.ActorTransform);
			// Load mesh data from record			
			NewMesh->LoadMeshFromRecord(MeshRecord);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Load failed! Saved file is incompatible!"));
	}
	
}

void URTMeshImporterComponent::ProcessNode(const aiNode* Node, const aiScene* Scene, ARTMeshActor* MeshActor, const FTransform& ImportTransform)
{
	// Process all the node's meshes
	for (uint32 i = 0; i < Node->mNumMeshes; i++)
	{
		TArray<FVector> Vertices;
		TArray<FVector> Normals;
		TArray<FVector2D> UV0;
		TArray<int32> Triangles;
		TArray<FRuntimeMeshTangent> Tangents;
		TArray<FColor> VertexColor;

		// Get current mesh for processing
		uint32 MeshIndex = Node->mMeshes[i];
		aiMesh* Mesh = Scene->mMeshes[MeshIndex];

		// Calculate the total triangle count
		uint32 TriangleCount = 0;
		for (uint32 j = 0; j < Mesh->mNumFaces; j++)
		{
			aiFace Face = Mesh->mFaces[j];
			TriangleCount += Face.mNumIndices;
		}

		// Reserve array memory
		Triangles.Reserve(TriangleCount);
		Vertices.Reserve(Mesh->mNumVertices);
		Normals.Reserve(Mesh->mNumVertices);
		UV0.Reserve(Mesh->mNumVertices);
		Tangents.Reserve(Mesh->mNumVertices);
		VertexColor.Reserve(Mesh->mNumVertices);

		// Convert assimp node transform to ue transform
		aiMatrix4x4 TransformMatrix = Node->mTransformation;
		FMatrix Matrix;
		Matrix.M[0][0] = TransformMatrix.a1; Matrix.M[0][1] = TransformMatrix.b1; Matrix.M[0][2] = TransformMatrix.c1; Matrix.M[0][3] = TransformMatrix.d1;
		Matrix.M[1][0] = TransformMatrix.a2; Matrix.M[1][1] = TransformMatrix.b2; Matrix.M[1][2] = TransformMatrix.c2; Matrix.M[1][3] = TransformMatrix.d2;
		Matrix.M[2][0] = TransformMatrix.a3; Matrix.M[2][1] = TransformMatrix.b3; Matrix.M[2][2] = TransformMatrix.c3; Matrix.M[2][3] = TransformMatrix.d3;
		Matrix.M[3][0] = TransformMatrix.a4; Matrix.M[3][1] = TransformMatrix.b4; Matrix.M[3][2] = TransformMatrix.c4; Matrix.M[3][3] = TransformMatrix.d4;

		// Modify original mesh transform before create mesh
		FTransform RelativeTransform = FTransform(Matrix);
		RelativeTransform *= ImportTransform;

		// Create the vertex
		for (uint32 j = 0; j < Mesh->mNumVertices; j++)
		{
			FVector Vertex = FVector(Mesh->mVertices[j].y, Mesh->mVertices[j].x, Mesh->mVertices[j].z);
			// Translate the transform
			Vertex = RelativeTransform.TransformPosition(Vertex);
			Vertices.Push(Vertex);

			if (Mesh->HasNormals())
			{
				Normals.Push(FVector(Mesh->mNormals[j].y, Mesh->mNormals[j].x, Mesh->mNormals[j].z));
			}
			else
			{
				//TODO Generate Flat normals
				UE_LOG(LogTemp, Warning, TEXT("No Normals"));
			}

			// UV Coordinates - inconsistent coordinates
			if (Mesh->HasTextureCoords(0))
			{
				UV0.Push(FVector2D(Mesh->mTextureCoords[0][j].x, -Mesh->mTextureCoords[0][j].y));
			}

			if (Mesh->HasTangentsAndBitangents())
			{
				Tangents.Push(FRuntimeMeshTangent(Mesh->mTangents[j].y, Mesh->mTangents[j].x, Mesh->mTangents[j].z));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No Tangents"));
			}

			if (Mesh->HasVertexColors(0))
			{
				VertexColor.Push(FColor(Mesh->mColors[0][j].r, Mesh->mColors[0][j].g, Mesh->mColors[0][j].b, Mesh->mColors[0][j].a));
			}
			else
			{
				VertexColor.Push(FColor::White);
			}
		}

		// Create the triangle
		for (uint32 j = 0; j < Mesh->mNumFaces; j++)
		{
			aiFace Face = Mesh->mFaces[j];
			for (uint32 index = 0; index < Face.mNumIndices; index++)
			{
				Triangles.Push(Face.mIndices[index]);
			}
		}

		// Get current section material
		aiMaterial* AIMaterial = Scene->mMaterials[Mesh->mMaterialIndex];

		// Create Material Info
		FRTMaterialInfo MaterialInfo = FRTMaterialInfo();
		MaterialInfo.MaterialName = FString(UTF8_TO_TCHAR(AIMaterial->GetName().C_Str()));
		MaterialInfo.SectionIndex = 0; // Each section will create a seperate component so there is always 1 section per mesh component
		MaterialInfo.MaterialIndex = 0;
		MaterialInfo.RuntimeMeshCompnenentName = FString::FromInt(MeshIndex);

		// Get the diffuse texture if exist
		GetTextureInfo(Scene, AIMaterial, aiTextureType_DIFFUSE, MeshActor->ImportedPath, MaterialInfo.DiffuseTexture);
		GetMatColor(AI_MATKEY_COLOR_DIFFUSE, AIMaterial, MaterialInfo.BaseColor);

		// Get the other textures if exist
		GetTextureInfo(Scene, AIMaterial, aiTextureType_NORMALS, MeshActor->ImportedPath, MaterialInfo.NormalTexture);
		GetTextureInfo(Scene, AIMaterial, aiTextureType_METALNESS, MeshActor->ImportedPath, MaterialInfo.MetallicTexture);
		GetTextureInfo(Scene, AIMaterial, aiTextureType_SPECULAR, MeshActor->ImportedPath, MaterialInfo.SpecularTexture);
		GetTextureInfo(Scene, AIMaterial, aiTextureType_DIFFUSE_ROUGHNESS, MeshActor->ImportedPath, MaterialInfo.RoughnessTexture);
		GetTextureInfo(Scene, AIMaterial, aiTextureType_EMISSIVE, MeshActor->ImportedPath, MaterialInfo.EmissiveTexture);
		GetTextureInfo(Scene, AIMaterial, aiTextureType_AMBIENT_OCCLUSION, MeshActor->ImportedPath, MaterialInfo.AmbientOcclusionTexture);
		if (MaterialInfo.AmbientOcclusionTexture.EmbeddedTextureBuffer.Num() <= 0 && MaterialInfo.AmbientOcclusionTexture.ImportTexturePath.IsEmpty())
		{
			GetTextureInfo(Scene, AIMaterial, aiTextureType_AMBIENT, MeshActor->ImportedPath, MaterialInfo.AmbientOcclusionTexture);
		}

		// Create section and material in ARTMeshActor
		MeshActor->DrawMeshSection(MaterialInfo, Vertices, Triangles, Normals, UV0, VertexColor, Tangents, true);
	}

	// Do the same for all of its children
	for (uint32 i = 0; i < Node->mNumChildren; i++)
	{
		ProcessNode(Node->mChildren[i], Scene, MeshActor, ImportTransform);
	}	
}

UTexture2D* URTMeshImporterComponent::GetTexture(const aiScene* Scene, const aiMaterial* AIMaterial, const aiTextureType TextureType, const FString FullPath)
{
	UTexture2D* Texture = nullptr;

	// Check if the material contains any diffuse texture
	if (AIMaterial->GetTextureCount(TextureType) > 0)
	{
		aiString AITexturePath;

		// Get the texture path from assimp
		AIMaterial->GetTexture(TextureType, 0, &AITexturePath);

		const aiTexture* EmbeddedTexture = Scene->GetEmbeddedTexture(AITexturePath.C_Str());

		// Check if there is a embedded texture
		if (EmbeddedTexture != nullptr)
		{
			Texture = ImportEmbeddedTexture(EmbeddedTexture);
			FString TextureName = FString(UTF8_TO_TCHAR(EmbeddedTexture->mFilename.C_Str()));
			if (!TextureName.IsEmpty() && Texture != nullptr)
			{
				Texture->Rename(*TextureName);
			}
		}
		// Looking for the diffuse texture at the file path
		else
		{
			// Current imported directory
			FString ImportedDir = FPaths::GetPath(FullPath).Replace(TEXT("\\"), TEXT("/"), ESearchCase::IgnoreCase);

			//Convert aiString to FString
			FString TexturePath = FString(AITexturePath.C_Str()).Replace(TEXT("\\"), TEXT("/"), ESearchCase::IgnoreCase);

			FString TextureName = FString(Scene->GetShortFilename(AITexturePath.C_Str()));

			FString ImportedDir_TexturePath = FPaths::Combine(ImportedDir, TexturePath);

			FString ImportedDir_TextureName = FPaths::Combine(ImportedDir, TextureName);

			if (FPaths::FileExists(TexturePath))
			{
				Texture = FImageUtils::ImportFileAsTexture2D(TexturePath);
			}
			else if (FPaths::FileExists(ImportedDir_TexturePath))
			{
				Texture = FImageUtils::ImportFileAsTexture2D(ImportedDir_TexturePath);
			}
			else if (FPaths::FileExists(ImportedDir_TextureName))
			{
				Texture = FImageUtils::ImportFileAsTexture2D(ImportedDir_TextureName);
			}

			if (Texture != nullptr)
			{
				Texture->Rename(*TextureName);
				switch (TextureType)
				{
				case aiTextureType_NORMALS:
					Texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
					Texture->SRGB = false;
					break;
				}
			}
		}
	}

	return Texture;
}

void URTMeshImporterComponent::GetTextureInfo(const aiScene* Scene, const aiMaterial* AIMaterial, const aiTextureType TextureType, const FString FullPath, FRTTextureInfo& OutTextureInfo)
{

	OutTextureInfo.ImportTexturePath.Empty();
	OutTextureInfo.EmbeddedTextureBuffer.Empty();

	// Check if the material contains any diffuse texture
	if (AIMaterial->GetTextureCount(TextureType) > 0)
	{
		aiString AITexturePath;

		// Get the texture path from assimp
		AIMaterial->GetTexture(TextureType, 0, &AITexturePath);

		const aiTexture* EmbeddedTexture = Scene->GetEmbeddedTexture(AITexturePath.C_Str());

		// Check if there is a embedded texture
		if (EmbeddedTexture != nullptr)
		{					
			ConvertEmbeddedTextureToBuffer(EmbeddedTexture, OutTextureInfo.EmbeddedTextureBuffer);
		}
		// Looking for the diffuse texture at the file path
		else
		{
			// Current imported directory
			FString ImportedDir = FPaths::GetPath(FullPath).Replace(TEXT("\\"), TEXT("/"), ESearchCase::IgnoreCase);

			//Convert aiString to FString
			FString TexturePath = FString(AITexturePath.C_Str()).Replace(TEXT("\\"), TEXT("/"), ESearchCase::IgnoreCase);

			FString TextureName = FString(Scene->GetShortFilename(AITexturePath.C_Str()));

			FString ImportedDir_TexturePath = FPaths::Combine(ImportedDir, TexturePath);

			FString ImportedDir_TextureName = FPaths::Combine(ImportedDir, TextureName);

			if (FPaths::FileExists(TexturePath))
			{
				OutTextureInfo.ImportTexturePath = TexturePath;
			}
			else if (FPaths::FileExists(ImportedDir_TexturePath))
			{				
				OutTextureInfo.ImportTexturePath = ImportedDir_TexturePath;
			}
			else if (FPaths::FileExists(ImportedDir_TextureName))
			{				
				OutTextureInfo.ImportTexturePath = ImportedDir_TextureName;
			}
			else
			{
				OutTextureInfo.ImportTexturePath.Empty();
			}			
		}
	}
}

UTexture2D* URTMeshImporterComponent::ImportEmbeddedTexture(const aiTexture* EmbeddedTexture)
{
	uint8* Pixels = reinterpret_cast<uint8*> (EmbeddedTexture->pcData);
	size_t size = 0;
	if (EmbeddedTexture->mHeight == 0)
	{
		size = EmbeddedTexture->mWidth;
	}
	else
	{
		size = EmbeddedTexture->mWidth * EmbeddedTexture->mHeight;
	}

	// Convert c array to TArray
	TArray<uint8> Buffer;
	Buffer.SetNumUninitialized(size);
	for (size_t i = 0; i < size; i++)
	{
		Buffer[i] = Pixels[i];
	}
	
	return FImageUtils::ImportBufferAsTexture2D(Buffer);
}

void URTMeshImporterComponent::ConvertEmbeddedTextureToBuffer(const aiTexture* EmbeddedTexture, TArray<uint8>& OutBuffer)
{
	uint8* Pixels = reinterpret_cast<uint8*> (EmbeddedTexture->pcData);
	size_t size = 0;
	if (EmbeddedTexture->mHeight == 0)
	{
		size = EmbeddedTexture->mWidth;
	}
	else
	{
		size = EmbeddedTexture->mWidth * EmbeddedTexture->mHeight;
	}

	// Convert c array to TArray
	OutBuffer.Empty();
	OutBuffer.SetNumUninitialized(size);
	for (size_t i = 0; i < size; i++)
	{
		OutBuffer[i] = Pixels[i];
	}	
}

bool URTMeshImporterComponent::GetMatColor(const char* pKey, unsigned int type, unsigned int idx, const aiMaterial* AIMaterial, FLinearColor& Result)
{
	aiColor3D AIColor3D = aiColor3D(0.f, 0.f, 0.f);
	if (AIMaterial->Get(pKey, type, idx, AIColor3D) == AI_SUCCESS)
	{
		Result = FLinearColor(AIColor3D.r, AIColor3D.g, AIColor3D.b);
		return true;
	}

	return false;
}

