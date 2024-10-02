// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_AoE.generated.h"

/**
 * Base class for creating shaped targeting actors in a 2D space. Currently not set up for Multiplayer
 */
UCLASS(Blueprintable)
class GUILDMASTER_API AGameplayAbilityTargetActor_AoE : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:

	AGameplayAbilityTargetActor_AoE(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector BoxExtents = FVector(100, 100, 100);

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ShapingFunction(const FVector& Position);

	virtual bool ShapingFunction_Implementation(const FVector& Position) { return true; }

protected:
	UFUNCTION(BlueprintCallable, CallInEditor)
	void TestFunc() {PerformOverlap(); };

	TArray<TWeakObjectPtr<AActor>> PerformOverlap();

	FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<TWeakObjectPtr<AActor>>& Actors) const;


	UFUNCTION(BlueprintCallable)
	void DebugDrawShape(int Resolution = 50) 
	{
		FVector StepSize = BoxExtents / FMath::Max(1, Resolution);

		const FVector Origin = StartLocation.GetTargetingTransform().GetLocation();

		FVector StartingPoint = Origin - FVector(BoxExtents.X * 0.5f, BoxExtents.Z * 0.5f, 0.0);

		constexpr bool bPersistent = false;
		constexpr float LifeTime = 2.0f;

		for (int i = 0; i < Resolution; i++)
		{
			for (int j = 0; j < Resolution; j++)
			{
				FVector pos = StartingPoint + FVector(StepSize.X * i, StepSize.Y * j, 0);
				
				DrawDebugSphere(GetWorld(), pos, 16, 16, ShapingFunction(pos) ?  FColor::Green : FColor::Red, bPersistent, LifeTime);
			}
		}
	}
};
