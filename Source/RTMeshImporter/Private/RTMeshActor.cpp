/************************************************************************************
 *																					*
 * Copyright (C) 2021 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/


#include "RTMeshActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "ImageUtils.h"

// Sets default values
ARTMeshActor::ARTMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/RTMeshImporter/Materials/Master/M_PBR_Runtime_Master.M_PBR_Runtime_Master'"));
	if (Material.Succeeded())
	{
		MasterMaterial = Material.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find Master Material"));
	}

}

void ARTMeshActor::DrawMeshSection(const FRTMaterialInfo& MaterialInfo, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals,
	const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColor, const TArray<FRuntimeMeshTangent>& Tangents, bool bCreateCollision)
{			
	CreateRuntimeMeshComponent(MaterialInfo.RuntimeMeshCompnenentName);
	RuntimeMeshComponents[MaterialInfo.RuntimeMeshCompnenentName]->CreateSectionFromComponents(0, MaterialInfo.SectionIndex, MaterialInfo.MaterialIndex,
		Vertices, Triangles, Normals, UV0, VertexColor, Tangents, ERuntimeMeshUpdateFrequency::Infrequent, bCreateCollision);
	
	SetupMaterial(MaterialInfo);

	// Save section data for serializtion later
	FRTSaveSectionRecord SectionRecord = FRTSaveSectionRecord();
	SectionRecord.Vertices = Vertices;
	SectionRecord.Triangles = Triangles;
	SectionRecord.Normals = Normals;
	SectionRecord.UV0 = UV0;
	SectionRecord.VertexColor = VertexColor;

	SectionRecord.Tangents.Reserve(Tangents.Num());
	for (auto Tangent : Tangents)
	{
		SectionRecord.Tangents.Push(Tangent.TangentX);
	}

	SectionRecord.bCreateCollision = bCreateCollision;

	ImportedMeshData.Add(SectionRecord);
}

void ARTMeshActor::ChangeSectionTexture(URuntimeMeshComponentStatic* RuntimeMeshComponent, FName TextureParameterName, FString TexturePath)
{
	UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(RuntimeMeshComponent->GetMaterial(0));
	if (MID == nullptr)
	{
		MID = UMaterialInstanceDynamic::Create(MasterMaterial, RuntimeMeshComponent);
	}
	
	UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(TexturePath);
	if (NewTexture)
	{
		// Enable texture map
		FString HasTextureName = "Has" + TextureParameterName.ToString();
		MID->SetScalarParameterValue(FName(*HasTextureName), 1.0f);
		// Set texture map
		MID->SetTextureParameterValue(TextureParameterName, NewTexture);
		
		// Get current imported texture paths index
		int32 SectionIndex = 0;
		for (auto MeshComponent : RuntimeMeshComponents)
		{
			if (MeshComponent.Value == RuntimeMeshComponent)
			{
				break;
			}
			SectionIndex++;
		}
		
		// Update new texture path for this section's material
		UpdateImportedTexturePath(TextureParameterName, ImportedTexturePathsList[SectionIndex], TexturePath);
	}
}

void ARTMeshActor::CreateRuntimeMeshComponent(FString ComponentName)
{
	URuntimeMeshComponentStatic* MeshComponent = NewObject<URuntimeMeshComponentStatic>(this, FName(*ComponentName));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->RegisterComponent();
	
	RuntimeMeshComponents.Add(ComponentName, MeshComponent);
}

void ARTMeshActor::SetupMaterial(const FRTMaterialInfo& MaterialInfo)
{
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(MasterMaterial, RuntimeMeshComponents[MaterialInfo.RuntimeMeshCompnenentName], FName(*MaterialInfo.MaterialName));

	// UV Transform
	MID->SetScalarParameterValue(RotationAngleName, MaterialInfo.RotationAngle);
	MID->SetScalarParameterValue(RotationCenterName, MaterialInfo.RotationCenter);
	MID->SetScalarParameterValue(UOffsetName, MaterialInfo.UOffset);
	MID->SetScalarParameterValue(VOffsetName, MaterialInfo.VOffset);
	MID->SetScalarParameterValue(UScaleName, MaterialInfo.UScale);
	MID->SetScalarParameterValue(VScaleName, MaterialInfo.VScale);

	// Imported texture paths
	FRTImportedTexturePaths TexturePaths;

	// Configure diffuse setting
	if (IsTextureExist(MaterialInfo.DiffuseTexture))
	{
		MID->SetScalarParameterValue(HasDiffuseName, 1);
		SetupTexture(MID, DiffuseName, MaterialInfo.DiffuseTexture, TexturePaths);
		MID->SetVectorParameterValue(BaseColorName, MaterialInfo.BaseColor);
	}
	else
	{
		MID->SetScalarParameterValue(HasDiffuseName, 0);
		MID->SetVectorParameterValue(BaseColorName, MaterialInfo.BaseColor);
	}

	// Configure Normal Setting
	if (IsTextureExist(MaterialInfo.NormalTexture))
	{
		MID->SetScalarParameterValue(HasNormalName, 1);
		SetupTexture(MID, NormalName, MaterialInfo.NormalTexture, TexturePaths);
	}

	// Configure Metallic Setting
	if (IsTextureExist(MaterialInfo.MetallicTexture))
	{
		MID->SetScalarParameterValue(HasMetallicName, 1);
		SetupTexture(MID, MetallicName, MaterialInfo.MetallicTexture, TexturePaths);
	}

	// Configure Specular Setting
	if (IsTextureExist(MaterialInfo.SpecularTexture))
	{
		MID->SetScalarParameterValue(HasSpecularName, 1);
		SetupTexture(MID, SpecularName, MaterialInfo.SpecularTexture, TexturePaths);
	}

	// Configure Rougness Setting
	if (IsTextureExist(MaterialInfo.RoughnessTexture))
	{
		MID->SetScalarParameterValue(HasRoughnessName, 1);
		SetupTexture(MID, RoughnessName, MaterialInfo.RoughnessTexture, TexturePaths);
	}

	// Configure Emissive Setting
	if (IsTextureExist(MaterialInfo.EmissiveTexture))
	{
		MID->SetScalarParameterValue(HasEmissiveName, 1);
		SetupTexture(MID, EmissiveName, MaterialInfo.EmissiveTexture, TexturePaths);
	}

	// Configure Ambient Setting
	if (IsTextureExist(MaterialInfo.AmbientOcclusionTexture))
	{
		MID->SetScalarParameterValue(HasAmbientName, 1);
		SetupTexture(MID, AmbientName, MaterialInfo.AmbientOcclusionTexture, TexturePaths);
	}

	RuntimeMeshComponents[MaterialInfo.RuntimeMeshCompnenentName]->SetMaterial(MaterialInfo.MaterialIndex, MID);
	
	// Save all imported texture path for this section
	ImportedTexturePathsList.Add(TexturePaths);
}

void ARTMeshActor::SetupTexture(UMaterialInstanceDynamic* MaterialInstance, FName ParameterName,  const FRTTextureInfo& TextureInfo, FRTImportedTexturePaths& OutPath)
{	
	if (MaterialInstance == nullptr)
	{
		return;
	}
	
	if (!TextureInfo.ImportTexturePath.IsEmpty())
	{
		UTexture2D* NewTexture = nullptr;
			
		NewTexture = Cast<UTexture2D>(FSoftClassPath(TextureInfo.ImportTexturePath).TryLoad());

		if (NewTexture == nullptr)
		{
			NewTexture = FImageUtils::ImportFileAsTexture2D(TextureInfo.ImportTexturePath);
		}

		if (NewTexture)
		{
			MaterialInstance->SetTextureParameterValue(ParameterName, NewTexture);
			
			// Save the texture path to imported path array
			UpdateImportedTexturePath(ParameterName, OutPath, TextureInfo.ImportTexturePath);
		}		
	}
	else if(TextureInfo.EmbeddedTextureBuffer.Num() > 0)
	{
		UTexture2D* NewTexture = FImageUtils::ImportBufferAsTexture2D(TextureInfo.EmbeddedTextureBuffer);
		if (NewTexture)
		{
			MaterialInstance->SetTextureParameterValue(ParameterName, NewTexture);
		}		
	}	
}

void ARTMeshActor::UpdateImportedTexturePath(const FName TextureParameterName, FRTImportedTexturePaths& ImportedTexturePath, const FString NewTexturePath)
{
	if (TextureParameterName == DiffuseName)
	{
		ImportedTexturePath.DiffusePath = NewTexturePath;
	}
	else if (TextureParameterName == NormalName)
	{
		ImportedTexturePath.NormalPath = NewTexturePath;
	}
	else if (TextureParameterName == MetallicName)
	{
		ImportedTexturePath.MetallicPath = NewTexturePath;
	}
	else if (TextureParameterName == SpecularName)
	{
		ImportedTexturePath.SpecularPath = NewTexturePath;
	}
	else if (TextureParameterName == RoughnessName)
	{
		ImportedTexturePath.RoughnessPath = NewTexturePath;
	}
	else if (TextureParameterName == EmissiveName)
	{
		ImportedTexturePath.EmissivePath = NewTexturePath;
	}
	else if (TextureParameterName == AmbientName)
	{
		ImportedTexturePath.AmbientOcclusionPath = NewTexturePath;
	}
}

void ARTMeshActor::SaveMeshToRecord(FRTSaveMeshRecord& OutRecord)
{	
	SavedMaterials.Empty();
	int32 Index = 0;
	for (auto Section : RuntimeMeshComponents)
	{				
		UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Section.Value->GetMaterial(0));

		if (MID == nullptr)
		{
			MID = UMaterialInstanceDynamic::Create(MasterMaterial, Section.Value);
		}
				
		OutRecord.ActorName = FName(*GetName());
		OutRecord.ActorTransform = GetTransform();

		// Create material record
		FRTSaveMaterialRecord MaterialRecord;
		MaterialRecord.SectionIndex = 0; // Each RuntimeMeshComponent only has one section so the section index is always 0
		MaterialRecord.MaterialIndex = 0;
		MaterialRecord.MaterialName = MID->GetName();
		MaterialRecord.BaseColor = MID->K2_GetVectorParameterValue(BaseColorName);

		// UV Transform value
		MaterialRecord.RotationAngle = MID->K2_GetScalarParameterValue(RotationAngleName);
		MaterialRecord.RotationCenter = MID->K2_GetScalarParameterValue(RotationCenterName);
		MaterialRecord.UOffset = MID->K2_GetScalarParameterValue(UOffsetName);
		MaterialRecord.VOffset = MID->K2_GetScalarParameterValue(VOffsetName);
		MaterialRecord.UScale = MID->K2_GetScalarParameterValue(UScaleName);
		MaterialRecord.VScale = MID->K2_GetScalarParameterValue(VScaleName);
		
		// Save textures to record				
		SaveTextureToRecord(Index, MID, DiffuseName, MaterialRecord.DiffuseTexture);
		SaveTextureToRecord(Index, MID, NormalName, MaterialRecord.NormalTexture);
		SaveTextureToRecord(Index, MID, MetallicName, MaterialRecord.MetallicTexture);
		SaveTextureToRecord(Index, MID, SpecularName, MaterialRecord.SpecularTexture);
		SaveTextureToRecord(Index, MID, RoughnessName, MaterialRecord.RoughnessTexture);
		SaveTextureToRecord(Index, MID, EmissiveName, MaterialRecord.EmissiveTexture);
		SaveTextureToRecord(Index, MID, AmbientName, MaterialRecord.AmbientOcclusionTexture);
		
		SavedMaterials.Add(MaterialRecord);

		// Save to mesh record
		OutRecord.SectionRecords.Add(ImportedMeshData[Index]);
		
		// Increase index
		Index++;
	}
	
}

void ARTMeshActor::LoadMeshFromRecord(const FRTSaveMeshRecord& FromRecord)
{
	int32 Index = 0;
	for (auto Section : FromRecord.SectionRecords)
	{		
		FString ComponentName = FString::FromInt(Index);

		TArray<FRuntimeMeshTangent> Tangents;
		Tangents.Reserve(Section.Tangents.Num());
		for (auto Tangent : Section.Tangents)
		{
			Tangents.Push(FRuntimeMeshTangent(Tangent));
		}
		
		// Create Material Info
		FRTMaterialInfo MaterialInfo = FRTMaterialInfo();
		MaterialInfo.MaterialName = SavedMaterials[Index].MaterialName;
		MaterialInfo.SectionIndex = SavedMaterials[Index].SectionIndex;
		MaterialInfo.MaterialIndex = SavedMaterials[Index].MaterialIndex;
		MaterialInfo.RuntimeMeshCompnenentName = ComponentName;
		MaterialInfo.BaseColor = SavedMaterials[Index].BaseColor;
		
		// UV Transform
		MaterialInfo.RotationAngle = SavedMaterials[Index].RotationAngle;
		MaterialInfo.RotationCenter = SavedMaterials[Index].RotationCenter;
		MaterialInfo.UOffset = SavedMaterials[Index].UOffset;
		MaterialInfo.VOffset = SavedMaterials[Index].VOffset;
		MaterialInfo.UScale = SavedMaterials[Index].UScale;
		MaterialInfo.VScale = SavedMaterials[Index].VScale;

		// Texture
		LoadTextureFromRecord(SavedMaterials[Index].DiffuseTexture, MaterialInfo.DiffuseTexture);
		LoadTextureFromRecord(SavedMaterials[Index].NormalTexture, MaterialInfo.NormalTexture);
		LoadTextureFromRecord(SavedMaterials[Index].MetallicTexture, MaterialInfo.MetallicTexture);
		LoadTextureFromRecord(SavedMaterials[Index].SpecularTexture, MaterialInfo.SpecularTexture);
		LoadTextureFromRecord(SavedMaterials[Index].RoughnessTexture, MaterialInfo.RoughnessTexture);
		LoadTextureFromRecord(SavedMaterials[Index].EmissiveTexture, MaterialInfo.EmissiveTexture);
		LoadTextureFromRecord(SavedMaterials[Index].AmbientOcclusionTexture, MaterialInfo.AmbientOcclusionTexture);

		DrawMeshSection(MaterialInfo, Section.Vertices, Section.Triangles, Section.Normals, Section.UV0, Section.VertexColor, Tangents, Section.bCreateCollision);

		Index++;
	}

	// Empty the save data
	SavedMaterials.Empty();
}

void ARTMeshActor::SaveTextureToRecord(int32 SectionIndex, UMaterialInstanceDynamic* MID, const FName ParameterName, FRTSaveTextureRecord& OutRecord)
{	
	
	UTexture2D* Texture = Cast<UTexture2D>(MID->K2_GetTextureParameterValue(ParameterName));
	
	if (Texture == nullptr)
	{
		return;
	}

	// If texture is imported from editor so just save the path
	if (!Texture->HasAnyFlags(RF_Transient))
	{
		OutRecord.DefaultTexturePath = Texture->GetPathName();
		OutRecord.TextureBuffer.Empty();

		//UE_LOG(LogTemp, Warning, TEXT("SaveTextureToRecord DefaultTexturePath: %s"), *OutRecord.DefaultTexturePath);
	}
	// If texture is imported at runtime or embedded so we save it to byte array
	else
	{	
		bool bIsPathExist = false;

		// If the imported texture has path, so we will save the file path
		if (ParameterName == DiffuseName && !ImportedTexturePathsList[SectionIndex].DiffusePath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].DiffusePath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;
		}
		else if (ParameterName == NormalName && !ImportedTexturePathsList[SectionIndex].NormalPath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].NormalPath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;
		}
		else if (ParameterName == MetallicName && !ImportedTexturePathsList[SectionIndex].MetallicPath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].MetallicPath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;

		}
		else if (ParameterName == SpecularName && !ImportedTexturePathsList[SectionIndex].SpecularPath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].SpecularPath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;
		}
		else if (ParameterName == RoughnessName && !ImportedTexturePathsList[SectionIndex].RoughnessPath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].RoughnessPath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;
		}
		else if (ParameterName == EmissiveName && !ImportedTexturePathsList[SectionIndex].EmissivePath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].EmissivePath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;
		}
		else if (ParameterName == AmbientName && !ImportedTexturePathsList[SectionIndex].AmbientOcclusionPath.IsEmpty())
		{
			OutRecord.DefaultTexturePath = ImportedTexturePathsList[SectionIndex].AmbientOcclusionPath;
			OutRecord.TextureBuffer.Empty();
			bIsPathExist = true;
		}
		
		// If the texture path doesn't exist, so we will save the texture to byte array
		if (bIsPathExist == false)
		{
			OutRecord.DefaultTexturePath.Empty();
			ConvertTextureToArray(Texture, OutRecord.TextureBuffer);
		}		
	}
}

void ARTMeshActor::LoadTextureFromRecord(const FRTSaveTextureRecord& FromRecord, FRTTextureInfo& ToTextureInfo)
{		
	if (!FromRecord.DefaultTexturePath.IsEmpty())
	{
		//UE_LOG(LogTemp, Warning, TEXT("LoadTextureFromRecord DefaultTexturePath: %s"), *FromRecord.DefaultTexturePath);
		//UTexture2D* NewTexture = Cast<UTexture2D>(FSoftClassPath(FromRecord.DefaultTexturePath).TryLoad());
		//return NewTexture;

		ToTextureInfo.ImportTexturePath = FromRecord.DefaultTexturePath;
	}
	else
	{
		/*
		// Get the texture width and height
		//const int32 TextureWidth = FromRecord.Width;
		//const int32 TextureHeight = FromRecord.Height;
		// Calculate the resolution from a number of pixels in our array (bytes / 4).
		 const float Resolution = FGenericPlatformMath::Sqrt(FromRecord.TextureBuffer.Num() / 4);

		// Create a new transient UTexture2D in a desired pixel format for byte order: B, G, R, A.
		UTexture2D* NewTexture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);

		if (!NewTexture)
		{
			return nullptr;
		}			
		
		NewTexture->NeverStream = true;

		// Get a reference to MIP 0, for convenience.
		FTexture2DMipMap& Mip = NewTexture->PlatformData->Mips[0];

		// Calculate the number of bytes we will copy.
		const int32 BufferSize = FromRecord.TextureBuffer.Num();
		
		//const int32 BufferSize = TextureWidth * TextureHeight;

		// Mutex lock the MIP's data, not letting any other thread read or write it now.
		void* MipBulkData = Mip.BulkData.Lock(LOCK_READ_WRITE);

		// Pre-allocate enough space to copy our bytes into the MIP's bulk data.
		Mip.BulkData.Realloc(BufferSize);

		FMemory::Memcpy(MipBulkData, FromRecord.TextureBuffer.GetData(), BufferSize);
		
		// Mutex unlock the MIP's data, letting all other threads read or lock for writing.
		Mip.BulkData.Unlock();

		// Let the engine process new data.
		NewTexture->UpdateResource();
		
		return NewTexture;
		*/
		//return FImageUtils::ImportBufferAsTexture2D(FromRecord.TextureBuffer);
		ToTextureInfo.EmbeddedTextureBuffer = FromRecord.TextureBuffer;
	}	
}

void ARTMeshActor::ConvertTextureToArray(UTexture2D* InTexture, TArray<uint8>& OutArray)
{		
	OutArray.Empty();
	if (!InTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Texture is missing!"));
		return;
	}
	else
	{
		int32 InTextureX = InTexture->GetSizeX();
		int32 InTextureY = InTexture->GetSizeY();
		
		// Init array		
		TArray<FColor> ColorArray;
		ColorArray.SetNumUninitialized(InTextureX * InTextureY);
		
		uint8* Raw = static_cast<uint8*>(InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));				
		
		FColor Pixel = FColor(0, 0, 0, 255);

		// Reading the pixel
		for (int32 y = 0; y < InTextureY; y++)
		{
			for (int32 x = 0; x < InTextureX; x++)
			{
				int32 CurrentPixelIndex = InTextureX * y + x;
				Pixel.B = Raw[4 * CurrentPixelIndex];
				Pixel.G = Raw[4 * CurrentPixelIndex + 1];
				Pixel.R = Raw[4 * CurrentPixelIndex + 2];
				Pixel.A = Raw[4 * CurrentPixelIndex + 3];
				ColorArray[CurrentPixelIndex] = FColor(Pixel.R, Pixel.G, Pixel.B, Pixel.A);
			}
		}				

		InTexture->PlatformData->Mips[0].BulkData.Unlock();
		InTexture->UpdateResource();

		//int32 BufferSize = ColorArray.Num() * 4;
		//OutArray.SetNumUninitialized(BufferSize);

		//FMemory::Memcpy(OutArray.GetData(), ColorArray.GetData(), BufferSize);
		FImageUtils::CompressImageArray(InTextureX, InTextureY, ColorArray, OutArray);
	}
}