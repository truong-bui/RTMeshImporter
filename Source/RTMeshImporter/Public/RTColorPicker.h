/************************************************************************************
 *																					*
 * Copyright (C) 2020 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Widgets/Colors/SColorPicker.h"
#include "RTColorPicker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorPickerChangeEvent, const FLinearColor&, NewColor);

class SRTColorPicker : public SColorPicker
{
	typedef SColorPicker Super;

public:
	FORCEINLINE void SetColorRGB(const FLinearColor& NewColor)
	{
		SetNewTargetColorRGB(NewColor, true);
	}

	FLinearColor InstantColor;
	bool Animation_SkipToFinalForOneTick = false;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		if (Animation_SkipToFinalForOneTick)
		{
			Animation_SkipToFinalForOneTick = false;
			Super::Tick(AllottedGeometry, InCurrentTime, 10000);
			SetNewTargetColorRGB(InstantColor, true);
			return;
		}

		Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	}
};

/**
 * 
 */
UCLASS()
class RTMESHIMPORTER_API URTColorPicker : public UWidget
{
	GENERATED_BODY()
	
protected:
	// Color Picker Slate
	TSharedPtr<SRTColorPicker> ColorPicker;
public:
	URTColorPicker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), Color(FLinearColor::White)
	{ }

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "RTColorPicker")
	FLinearColor Color;

	/** Should the color picker jump instantly to the chosen Color when it is first created? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RTColorPicker")
		bool bSkipAnimationOnConstruction = false;

	/** Called whenever the color is changed! */
	UPROPERTY(BlueprintAssignable, Category = "RTColorPicker")
	FOnColorPickerChangeEvent OnColorChanged;

	/** Get Color! */
	UFUNCTION(BlueprintPure, Category = "RTColorPicker")
		FLinearColor GetColor();

	/** Set Color Picker's Color! */
	UFUNCTION(BlueprintCallable, Category = "RTColorPicker")
	void SetColor(FLinearColor NewColor, bool bSkipAnimation = false);

	void ColorUpdated(FLinearColor NewValue);
	void ColorPickCancelled(FLinearColor NewValue);

public:
	// UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	// UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

#if WITH_EDITOR
	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// UWidget interface
	virtual const FText GetPaletteCategory() override;
#endif
};
