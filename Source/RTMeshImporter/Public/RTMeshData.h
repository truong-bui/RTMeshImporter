/************************************************************************************
 *																					*
 * Copyright (C) 2021 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Misc/DateTime.h"
#include "RTMeshData.generated.h"

struct FRTTextureInfo
{
	// Texture path that is imported at runtime or from editor. Empty if there is no imported texture.
	FString ImportTexturePath;
	
	// Buffer data of embedded texture. Empty if there is no embedded texture.
	TArray<uint8> EmbeddedTextureBuffer;
};

// Material information that passes to RTMeshActor to create mesh materterial
USTRUCT(BlueprintType)
struct FRTMaterialInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString MaterialName;	

	UPROPERTY(BlueprintReadWrite)
	int32 MaterialIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SectionIndex = 0;
	
	UPROPERTY(BlueprintReadWrite)
	FString RuntimeMeshCompnenentName;

	UPROPERTY(BlueprintReadWrite)
	FLinearColor BaseColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite)
	float RotationAngle = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float RotationCenter = 0.5f;
	UPROPERTY(BlueprintReadWrite)
	float UOffset = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float VOffset = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float UScale = 1.0f;
	UPROPERTY(BlueprintReadWrite)
	float VScale = 1.0f;
	
	FRTTextureInfo DiffuseTexture;
	FRTTextureInfo NormalTexture;
	FRTTextureInfo MetallicTexture;
	FRTTextureInfo SpecularTexture;
	FRTTextureInfo RoughnessTexture;
	FRTTextureInfo EmissiveTexture ;
	FRTTextureInfo AmbientOcclusionTexture;
};

// Textures from runtime material
USTRUCT()
struct FRTSaveTextureRecord
{
	GENERATED_BODY()
	// Built-in texture path which is imported from editor or at runtime
	UPROPERTY(SaveGame)
	FString DefaultTexturePath;

	// Any embedded texture will be saved to byte array
	UPROPERTY(SaveGame)
	TArray<uint8> TextureBuffer;

	friend FArchive& operator<<(FArchive& Ar, FRTSaveTextureRecord& SaveTextureRecord)
	{		
		Ar << SaveTextureRecord.DefaultTexturePath;
		Ar << SaveTextureRecord.TextureBuffer;

		return Ar;
	}
};

// Saved material information
USTRUCT()
struct FRTSaveMaterialRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FString MaterialName;
	UPROPERTY(SaveGame)
	int32 MaterialIndex;
	UPROPERTY(SaveGame)
	int32 SectionIndex;
	UPROPERTY(SaveGame)
	FLinearColor BaseColor = FLinearColor::White;

	UPROPERTY(SaveGame)
	float RotationAngle = 0.0f;
	UPROPERTY(SaveGame)
	float RotationCenter = 0.5f;
	UPROPERTY(SaveGame)
	float UOffset = 0.0f;
	UPROPERTY(SaveGame)
	float VOffset = 0.0f;
	UPROPERTY(SaveGame)
	float UScale = 1.0f;
	UPROPERTY(SaveGame)
	float VScale = 1.0f;

	UPROPERTY(SaveGame)
	FRTSaveTextureRecord DiffuseTexture;
	UPROPERTY(SaveGame)
	FRTSaveTextureRecord NormalTexture;
	UPROPERTY(SaveGame)
	FRTSaveTextureRecord MetallicTexture;
	UPROPERTY(SaveGame)
	FRTSaveTextureRecord SpecularTexture;
	UPROPERTY(SaveGame)
	FRTSaveTextureRecord RoughnessTexture;
	UPROPERTY(SaveGame)
	FRTSaveTextureRecord EmissiveTexture;
	UPROPERTY(SaveGame)
	FRTSaveTextureRecord AmbientOcclusionTexture;
	

	friend FArchive& operator<<(FArchive& Ar, FRTSaveMaterialRecord RTMaterialData)
	{
		Ar << RTMaterialData.MaterialName;
		Ar << RTMaterialData.MaterialIndex;
		Ar << RTMaterialData.SectionIndex;
		Ar << RTMaterialData.BaseColor;
		Ar << RTMaterialData.RotationAngle;
		Ar << RTMaterialData.RotationCenter;
		Ar << RTMaterialData.UOffset;
		Ar << RTMaterialData.VOffset;
		Ar << RTMaterialData.UScale;
		Ar << RTMaterialData.VScale;
		Ar << RTMaterialData.DiffuseTexture;
		Ar << RTMaterialData.NormalTexture;
		Ar << RTMaterialData.MetallicTexture;
		Ar << RTMaterialData.SpecularTexture;
		Ar << RTMaterialData.RoughnessTexture;
		Ar << RTMaterialData.EmissiveTexture;
		Ar << RTMaterialData.AmbientOcclusionTexture;		

		return Ar;
	}
};

// Saved mesh section data
USTRUCT()
struct FRTSaveSectionRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FVector> Vertices;
	UPROPERTY(SaveGame)
	TArray<FVector> Normals;
	UPROPERTY(SaveGame)
	TArray<FVector2D> UV0;
	UPROPERTY(SaveGame)
	TArray<int32> Triangles;
	UPROPERTY(SaveGame)
	TArray<FVector> Tangents;
	UPROPERTY(SaveGame)
	TArray<FColor> VertexColor;	
	UPROPERTY(SaveGame)
	bool bCreateCollision;

	friend FArchive& operator<<(FArchive& Ar, FRTSaveSectionRecord& RTSectionRecord)
	{	
		Ar << RTSectionRecord.Vertices;
		Ar << RTSectionRecord.Normals;
		Ar << RTSectionRecord.UV0;
		Ar << RTSectionRecord.Triangles;
		Ar << RTSectionRecord.Tangents;
		Ar << RTSectionRecord.VertexColor;
		Ar << RTSectionRecord.bCreateCollision;

		return Ar;
	}
};


// Saved mesh data
USTRUCT()
struct FRTSaveMeshRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FName ActorName;
	UPROPERTY(SaveGame)
	FTransform ActorTransform;		
	UPROPERTY(SaveGame)
	TArray<FRTSaveSectionRecord> SectionRecords;
	// for extra save data from the mesh actor
	UPROPERTY(SaveGame)
	TArray<uint8> ActorData;

	friend FArchive& operator<<(FArchive& Ar, FRTSaveMeshRecord& RTMeshRecord)
	{
		Ar << RTMeshRecord.ActorName;
		Ar << RTMeshRecord.ActorTransform;
		Ar << RTMeshRecord.SectionRecords;
		Ar << RTMeshRecord.ActorData;

		return Ar;
	}
};

// Saved file data for the demo
USTRUCT()
struct FRTSaveFileRecord
{
	GENERATED_BODY()

	// This number is saved with the save file, so plugin will not load the save file
	// if the save file which contain an older number than the build (BuildVersion)
	uint32 SaveVersion;
	FDateTime Timestamp;
	TArray<FRTSaveMeshRecord> SaveActors;

	friend FArchive& operator<<(FArchive& Ar, FRTSaveFileRecord& FileRecord)
	{
		Ar << FileRecord.SaveVersion;
		Ar << FileRecord.Timestamp;
		Ar << FileRecord.SaveActors;

		return Ar;
	}
};