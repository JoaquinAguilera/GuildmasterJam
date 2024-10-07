// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilityTargetActor_AoE.h"
#include "AbilitySystemLog.h"

#include "GameFramework/Pawn.h"
#include "WorldCollision.h"
#include "Abilities/GameplayAbility.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

#include "Components/DecalComponent.h"

AGameplayAbilityTargetActor_AoE::AGameplayAbilityTargetActor_AoE(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	ShouldProduceTargetDataOnServer = true;
	bDebug = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("SceneRoot");

	DecalComponent = CreateDefaultSubobject<UDecalComponent>("Decal");
	DecalComponent->SetupAttachment(RootComponent);
	

	// I have no fucking clue why but when I rotate Y it rotates on the Z axis. Z -> X and X -> Y
	DecalComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
}

TArray<TWeakObjectPtr<AActor>> AGameplayAbilityTargetActor_AoE::PerformOverlap()
{
	bool bTraceComplex = false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AoEOverlapTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = false;

	TArray<FOverlapResult> Overlaps;


	const FTransform TargetingTransform = StartLocation.GetTargetingTransform();
	const FVector Origin = TargetingTransform.GetLocation();
	const FCollisionShape Bounds = FCollisionShape::MakeBox(TargetingTransform.GetRotation().RotateVector(BoxExtents * 0.5f));

	GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECC_GameTraceChannel1), Bounds, Params);

	TArray<TWeakObjectPtr<AActor>>	HitActors;

	for (int32 i = 0; i < Overlaps.Num(); ++i)
	{
		//Should this check to see if these pawns are in the AimTarget list?
		APawn* PawnActor = Overlaps[i].OverlapObjectHandle.FetchActor<APawn>();
		if (PawnActor && !HitActors.Contains(PawnActor) && Filter.FilterPassesForActor(PawnActor))
		{
			HitActors.Add(PawnActor);
		}
	}

	if (bDebug)
	{
		const FColor& DebugColor = HitActors.Num() == 0 ? FColor::Red : FColor::Green;
		UE_VLOG_WIREBOX(SourceActor, LogAbilitySystem, Log, FBox(Origin - Bounds.GetExtent(), Origin + Bounds.GetExtent()), DebugColor, TEXT("TargetActor Radius"));

#if UE_ENABLE_DEBUG_DRAWING
		constexpr bool bPersistent = false;
		constexpr float LifeTime = 2.0f;
		DrawDebugBox(GetWorld(), Origin, Bounds.GetExtent(), DebugColor, bPersistent, LifeTime);
#endif // UE_ENABLE_DEBUG_DRAWING
	}

	return HitActors;

}

FGameplayAbilityTargetDataHandle AGameplayAbilityTargetActor_AoE::MakeTargetData(const TArray<TWeakObjectPtr<AActor>>& Actors) const
{
	if (OwningAbility)
	{
		return StartLocation.MakeTargetDataHandleFromActors(Actors, false);
	}

	return FGameplayAbilityTargetDataHandle();
}
