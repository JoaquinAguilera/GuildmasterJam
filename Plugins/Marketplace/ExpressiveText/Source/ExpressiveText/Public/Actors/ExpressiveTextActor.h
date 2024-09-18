// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Components/ExpressiveTextComponent.h"

#include "ExpressiveTextActor.generated.h"

#define PROBE_DISTANCE 10000000.f;

UCLASS(ClassGroup="Expressive Text", ConversionRoot, ComponentWrapperClass, meta=(ChildCanTick))
class EXPRESSIVETEXT_API AExpressiveTextActor : public AActor
{
    GENERATED_BODY()
public:
    AExpressiveTextActor();

    virtual void BeginPlay() override;
    virtual bool ShouldTickIfViewportsOnly() const override;
	virtual void Tick(float DeltaTime) override;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = ExpressiveText)
    UExpressiveTextComponent* ExpressiveTextComponent;

#if WITH_EDITORONLY_DATA

    UPROPERTY(EditInstanceOnly, Category = "Placement Helpers")
    AActor* ActorToSnapTo;

    UPROPERTY(EditInstanceOnly, Category = "Placement Helpers")
    float SurfaceSnapDistance;

#endif

#if WITH_EDITOR
	 UFUNCTION( BlueprintCallable, CallInEditor, Category="Placement Helpers")
	 void SnapToObjectOnCamera();

    void GetViewportCameraInfo( FRotator& ViewportCamRotation, FVector& ViewportCamLocation );
    FHitResult ProcessActorSnapping( FVector ProbeStart, FVector SnapDirection, AActor* SpecificActor = nullptr );
#endif

};