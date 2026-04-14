/************************************************************************************
 *																					*
 * Copyright (C) 2021 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTMeshActor.h"
#include <assimp/scene.h>
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/Archive.h"
#include "RTMeshImporterComponent.generated.h"

struct FRTSaveActorArchive : public FObjectAndNameAsStringProxyArchive
{
	FRTSaveActorArchive(FArchive& InInnerArchive) : FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RTMESHIMPORTER_API URTMeshImporterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URTMeshImporterComponent();


	// This is the current version of the game build. If this doesn't match the serialized version (SaveVersion) in FRTSaveFileData,
	// the save file is out of date and shouldn't be loaded!
	const uint32 BuildVersion = 1;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/**
	 * Import 3d mesh from file
	 * @param Path 3D file path to import
	 * @return Mesh actor which is created after import
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshImporter")
	ARTMeshActor* ImportMesh(const FString Path);

	/**
	 * Import 3d mesh from file with custom transform of the mesh. 
	 * Note: Imported transform will translate the transformation 
	 * of the vertices not the tranformation of the mesh actor.
	 * @param Path 3D file path to import
	 * @param ImportTransform Custom transform for vertices
	 * @return Mesh actor which is created after import
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshImporter")
	ARTMeshActor* ImportMeshWithTransform(const FString Path, const FTransform& ImportTransform);

	/**
	 * Helper function for open supported 3d file formats 
	 * @param bIsFileSelected True if user select any file
	 * @param OutFiles List of selected file. Since we only allow selecting a single file so the array always has 1 element if user select any file
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshImporter")
	void Open3DFileDialog(bool& bIsFileSelected, TArray<FString>& OutFiles);

	/**
	 * Helper function for open texture files
	 * @param bIsFileSelected True if user select any file
	 * @param OutFiles List of selected file. Since we only allow selecting a single file so the array always has 1 element if user select any file
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshImporter")
	void OpenTextureDialog(bool& bIsFileSelected, TArray<FString>& OutFiles);

	/**
	 * Save all RTMeshActors are on the level. File is saved at "Saved" folder
	 * @param Filename Name of the save file	 
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshImporter")
	void SaveRTMeshesToFile(FString Filename);

	/**
	 * Load RTMesh from file
	 * @param Filename Name of the save file
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshImporter")
	void LoadRTMeshesFromFile(FString Filename);
private:
	// Parsing assimp structures
	void ProcessNode(const aiNode* Node, const aiScene* Scene, ARTMeshActor* MeshActor, const FTransform& ImportTransform);

	UTexture2D* GetTexture(const aiScene* Scene, const aiMaterial* AIMaterial, const aiTextureType TextureType, const FString FullPath);

	void GetTextureInfo(const aiScene* Scene, const aiMaterial* AIMaterial, const aiTextureType TextureType, const FString FullPath, FRTTextureInfo& OutTextureInfo);

	// Import embedded texture from assimp to ue
	UTexture2D* ImportEmbeddedTexture(const aiTexture* EmbeddedTexture);

	void ConvertEmbeddedTextureToBuffer(const aiTexture* EmbeddedTexture, TArray<uint8>& OutBuffer);

	bool GetMatColor(const char* pKey, unsigned int type, unsigned int idx, const aiMaterial* AIMaterial, FLinearColor& Result);	
};
