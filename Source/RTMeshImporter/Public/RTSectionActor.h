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
#include "Components/RuntimeMeshComponentStatic.h"
#include "RTSectionActor.generated.h"

UCLASS()
class RTMESHIMPORTER_API ARTSectionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARTSectionActor();
	
	UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = "RTSectionActor")
	URuntimeMeshComponentStatic* RuntimeMeshComponent;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
};
