/************************************************************************************
 *																					*
 * Copyright (C) 2021 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "RTMeshData.h"
#include "RTMeshActor.generated.h"

USTRUCT()
struct FRTImportedTexturePaths
{
	GENERATED_BODY()

	FString DiffusePath;
	FString NormalPath;
	FString MetallicPath;
	FString SpecularPath;
	FString RoughnessPath;
	FString EmissivePath;
	FString AmbientOcclusionPath;
};

UCLASS()
class RTMESHIMPORTER_API ARTMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARTMeshActor();

	// Empty root component
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = "RTMeshActor")
	USceneComponent* SceneComponent;

	// Array of procedural mesh components
//	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = "RTMeshActor")
	TMap<FString, UProceduralMeshComponent*> RuntimeMeshComponents;

	//UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "RTMeshActor")
	//FString MeshName;	

	// Original imported full path
	UPROPERTY(BlueprintReadOnly, SaveGame, VisibleDefaultsOnly, Category = "RTMeshActor")
	FString ImportedPath;	
			
private:

		const FName HasDiffuseName = FName(TEXT("HasDiffuseTexture"));
		const FName DiffuseName = FName(TEXT("DiffuseTexture"));	
		const FName BaseColorName = FName(TEXT("BaseColor"));
	
		const FName HasNormalName = FName(TEXT("HasNormalTexture"));
		const FName NormalName = FName(TEXT("NormalTexture"));
	
		const FName HasMetallicName = FName(TEXT("HasMetallicTexture"));
		const FName MetallicName = FName(TEXT("MetallicTexture"));
	
		const FName HasSpecularName = FName(TEXT("HasSpecularTexture"));
		const FName SpecularName = FName(TEXT("SpecularTexture"));
	
		const FName HasRoughnessName = FName(TEXT("HasRoughnessTexture"));
		const FName RoughnessName = FName(TEXT("RoughnessTexture"));

		const FName HasEmissiveName = FName(TEXT("HasEmissiveTexture"));
		const FName EmissiveName = FName(TEXT("EmissiveTexture"));
	
		const FName HasAmbientName = FName(TEXT("HasAmbientTexture"));
		const FName AmbientName = FName(TEXT("AmbientTexture"));

		// UV Transform names
		const FName RotationAngleName = FName(TEXT("RotationAngle"));
		const FName RotationCenterName = FName(TEXT("RotationCenter"));
		const FName UOffsetName = FName(TEXT("UOffset"));
		const FName VOffsetName = FName(TEXT("VOffset"));
		const FName UScaleName = FName(TEXT("UScale"));
		const FName VScaleName = FName(TEXT("VScale"));	

		TArray<FRTSaveSectionRecord> ImportedMeshData;

		TArray<FRTImportedTexturePaths> ImportedTexturePathsList;

		// Master material for dynamic material instances
		UMaterial* MasterMaterial;

		// Saved materials 
		UPROPERTY(SaveGame)
		TArray<FRTSaveMaterialRecord> SavedMaterials;
public:	
	// Draw mesh section
	void DrawMeshSection(const FRTMaterialInfo& MaterialInfo, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals,
		const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColor, const TArray<FProcMeshTangent>& Tangents, bool bCreateCollision = true);	

	/**
	 * Change the texture map of the section. Use this instead of directly change texture parameter in material instance dynamic.
	 * This function will save the texture path for saving and loading.
	 * @param ProceduralMeshComponent UProceduralMeshComponent that need to change texture map.
	 * @param TextureParemeterName Material instance dynamic parameter (i.e DiffuseTexture)
	 * @param TexturePath Texture full path
	 **/
	UFUNCTION(BlueprintCallable, Category = "RTMeshActor")
	void ChangeSectionTexture(UProceduralMeshComponent* ProceduralMeshComponent, FName TextureParameterName, FString TexturePath);
private:
	// Create procedural mesh component and add to ProceduralMeshComponents array
	void CreateRuntimeMeshComponent(FString ComponentName);	

	// Create section material
	void SetupMaterial(const FRTMaterialInfo& MaterialInfo);
	// Create section texture
	void SetupTexture(UMaterialInstanceDynamic* MaterialInstance, FName ParameterName, const FRTTextureInfo& TextureInfo, FRTImportedTexturePaths& OutPath);		

	inline bool IsTextureExist(const FRTTextureInfo& TextureInfo)
	{
		return TextureInfo.EmbeddedTextureBuffer.Num() > 0 || !TextureInfo.ImportTexturePath.IsEmpty();
	}	

	void UpdateImportedTexturePath(const FName TextureParameterName, FRTImportedTexturePaths& ImportedTexturePath, const FString NewTexturePath);

public:
	void SaveMeshToRecord(FRTSaveMeshRecord& OutRecord);
	void LoadMeshFromRecord(const FRTSaveMeshRecord& FromRecord);

private:
	void SaveTextureToRecord(int32 SectionIndex, UMaterialInstanceDynamic* MID, const FName ParameterName, FRTSaveTextureRecord& OutRecord);
	void LoadTextureFromRecord(const FRTSaveTextureRecord& FromRecord, FRTTextureInfo& ToTextureInfo);
	void ConvertTextureToArray(UTexture2D* InTexture, TArray<uint8>& OutArray);	
};
