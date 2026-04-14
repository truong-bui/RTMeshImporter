/************************************************************************************
 *																					*
 * Copyright (C) 2021 Truong Bui.													*
 * Website:	https://github.com/truong-bui/RTMeshImporter							*
 * Licensed under the MIT License. See 'LICENSE' file for full license information. *
 *																					*
 ************************************************************************************/


#include "RTColorPicker.h"

#define LOCTEXT_NAMESPACE "UMG"

FLinearColor URTColorPicker::GetColor()
{
	return Color;
}

void URTColorPicker::SetColor(FLinearColor NewColor, bool bSkipAnimation)
{
	if (!ColorPicker.IsValid()) return;

	// Skip anim?
	if (bSkipAnimation)
	{
		ColorPicker->InstantColor = NewColor;
		ColorPicker->Animation_SkipToFinalForOneTick = true;
	}
	else
	{
		ColorPicker->SetColorRGB(NewColor);
	}
}

void URTColorPicker::ColorUpdated(FLinearColor NewValue)
{
	Color = NewValue;

	if (OnColorChanged.IsBound())
	{
		OnColorChanged.Broadcast(Color);
	}
}

void URTColorPicker::ColorPickCancelled(FLinearColor NewValue)
{
	//Color Picking Cancelled! Do nothing!
}

void URTColorPicker::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ColorPicker.Reset();
}
//
TSharedRef<SWidget> URTColorPicker::RebuildWidget()
{
	FColorPickerArgs Args;

	// Initial Color
	Args.InitialColorOverride = Color;
	Args.bUseAlpha = true;
	Args.bOnlyRefreshOnOk = false;
	Args.bOnlyRefreshOnMouseUp = false;

	//Delegates!
	Args.OnColorCommitted = FOnLinearColorValueChanged::CreateUObject(this, &URTColorPicker::ColorUpdated);
	Args.OnColorPickerCancelled = FOnColorPickerCancelled::CreateUObject(this, &URTColorPicker::ColorPickCancelled);

	ColorPicker = SNew(SRTColorPicker)
		.TargetColorAttribute(Args.InitialColorOverride)
		.TargetFColors(Args.ColorArray ? *Args.ColorArray : TArray<FColor*>())
		.TargetLinearColors(Args.LinearColorArray ? *Args.LinearColorArray : TArray<FLinearColor*>())
		.TargetColorChannels(Args.ColorChannelsArray ? *Args.ColorChannelsArray : TArray<FColorChannels>())
		.UseAlpha(Args.bUseAlpha)
		.ExpandAdvancedSection(Args.bExpandAdvancedSection)
		.OnlyRefreshOnMouseUp(Args.bOnlyRefreshOnMouseUp && !Args.bIsModal)
		.OnlyRefreshOnOk(Args.bOnlyRefreshOnOk || Args.bIsModal)
		.OnColorCommitted(Args.OnColorCommitted)
		.PreColorCommitted(Args.PreColorCommitted)
		.OnColorPickerCancelled(Args.OnColorPickerCancelled)
		.OnInteractivePickBegin(Args.OnInteractivePickBegin)
		.OnInteractivePickEnd(Args.OnInteractivePickEnd)
		.DisplayGamma(Args.DisplayGamma);

	// Skip Animation?
	if (bSkipAnimationOnConstruction)
	{
		SetColor(Color, true); // Skip
	}

	return ColorPicker.ToSharedRef();
}

#if WITH_EDITOR
void URTColorPicker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Update Picker to Color property change!
	if (PropertyName == TEXT("Color"))
	{
		if (ColorPicker.IsValid())
		{
			ColorPicker->SetColorRGB(Color);
		}
	}
}
const FText URTColorPicker::GetPaletteCategory()
{
	return LOCTEXT("RTColorPicker", "RT Color Picker");
}
#endif

#undef LOCTEXT_NAMESPACE