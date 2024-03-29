// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Blueprint/UserWidget.h"
#include "UI_Blank.generated.h"

UCLASS()
class VREXPERIMENTSBASE_API AUI_Blank : public AInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUI_Blank(const FObjectInitializer& ObjectInitializer);
	void SetWidgetClass(TSubclassOf<UUserWidget> widget_class, const FVector2D& draw_size);
	UUserWidget* GetWidget();
	UFUNCTION(BlueprintCallable)
	void SetEnabled(bool enabled);
	UFUNCTION(BlueprintCallable)
	void SetVisibility(bool visibility);

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	class UWidgetComponent* WidgetComponent;
};
