// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "OutlineComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAINMAGIC_API UOutlineComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOutlineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SetLineThickness(float Thickness);
};
