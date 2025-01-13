#include "PeriscopeActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

APeriscopeActor::APeriscopeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create and attach the periscope mesh
    PeriscopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PeriscopeMesh"));
    RootComponent = PeriscopeMesh;

    // Create and attach the camera component
    PeriscopeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PeriscopeCamera"));
    PeriscopeCamera->SetupAttachment(PeriscopeMesh);

    // Adjust the default rotation speed
    RotationSpeed = 45.0f; // degrees per second
}

void APeriscopeActor::BeginPlay()
{
    Super::BeginPlay();
}

void APeriscopeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APeriscopeActor::RotatePeriscope(float AxisValue)
{
    // Rotate the periscope based on the input axis value
    if (AxisValue != 0.0f)
    {
        FRotator NewRotation = FRotator(0.0f, AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds(), 0.0f);
        AddActorLocalRotation(NewRotation);
    }
}
