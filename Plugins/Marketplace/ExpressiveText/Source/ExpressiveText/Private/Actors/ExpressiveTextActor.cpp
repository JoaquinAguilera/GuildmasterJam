// Copyright 2022 Guganana. All Rights Reserved.
#include "Actors/ExpressiveTextActor.h"
#include "Guganana/Logging.h"

#include "ExpressiveText/Public/Subsystems/ExpressiveTextSubsystem.h"

AExpressiveTextActor::AExpressiveTextActor() 
	: Super()
	, ExpressiveTextComponent(CreateDefaultSubobject<UExpressiveTextComponent>(TEXT("ExpressiveTextComponent")))
#if WITH_EDITORONLY_DATA
	, ActorToSnapTo(nullptr)
	, SurfaceSnapDistance(0.25f)
#endif
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorTickEnabled(true);
}

void AExpressiveTextActor::BeginPlay()
{
	Super::BeginPlay();
}

bool AExpressiveTextActor::ShouldTickIfViewportsOnly() const
{
	UWorld* World = GetWorld();
	if (World != nullptr && World->WorldType == EWorldType::Editor)
	{
		return true;
	}

	return false;
}

void AExpressiveTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
	if( UWorld* World = GetWorld() )
	{
		//check if world type is play in editor
		if( World->WorldType == EWorldType::Editor)
		{
			if( ExpressiveTextComponent )
			{
				ExpressiveTextComponent->EnsureWidgetCreated();
			}

			if (ActorToSnapTo)
			{
				const FVector& SnapActorLocation = ActorToSnapTo->GetActorLocation();
				FVector Direction = GetActorForwardVector() * -1.f;
				Direction.Normalize();

				FVector StartPoint = GetActorLocation() - Direction * 100.f;
				ProcessActorSnapping(StartPoint, Direction, ActorToSnapTo);
			}
		}
	}
#endif
}

#if WITH_EDITOR
void AExpressiveTextActor::SnapToObjectOnCamera()
{
	FVector CamLocation;
	FRotator CamRotation;
	GetViewportCameraInfo(CamRotation, CamLocation);

	auto HitResult = ProcessActorSnapping(CamLocation, CamRotation.Vector());

	if (HitResult.GetActor())
	{
		ActorToSnapTo = HitResult.GetActor();
	}
}
#endif

#if WITH_EDITOR
void AExpressiveTextActor::GetViewportCameraInfo(FRotator& OutRotation, FVector& OutLocation)
{
	OutRotation = FRotator::ZeroRotator;
	OutLocation = FVector::ZeroVector;
	
	if (UWorld* World = GetWorld())
	{
		
		GEngine->Exec(World, TEXT("ExpressiveTextEditor.UpdateViewportCameraPositioning"));

		if( auto* ExpressiveTextSubsystem = GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>() )
		{
			OutRotation = ExpressiveTextSubsystem->ViewportCameraRot;
			OutLocation = ExpressiveTextSubsystem->ViewportCameraLoc;
		}
	}
}
#endif

#if WITH_EDITOR
FHitResult AExpressiveTextActor::ProcessActorSnapping(FVector ProbeStart, FVector SnapDirection, AActor* SpecificActor /*= nullptr */)
{
	FHitResult HitResult;
	FVector End = ProbeStart + SnapDirection * 1000000.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;

	bool bHit = false;
	if (SpecificActor)
	{
		bHit = SpecificActor->ActorLineTraceSingle(HitResult, ProbeStart, End, ECC_Visibility, Params);
	}
	else
	{
		bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ProbeStart, End, ECC_Visibility, Params);
	}

	if (bHit)
	{

		FVector DesiredLocation = HitResult.ImpactPoint + HitResult.ImpactNormal * SurfaceSnapDistance;
		if(  FVector::DistSquared(DesiredLocation, GetActorLocation()) > 1.f )
		{
			SetActorLocation(DesiredLocation);
		}
		
		FRotator FinalRotation = FRotationMatrix::MakeFromXZ(HitResult.ImpactNormal, GetActorUpVector()).Rotator();
		SetActorRotation(FinalRotation);
	}

	return HitResult;
}
#endif