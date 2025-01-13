#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PeriscopeActor.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UCameraComponent;

UCLASS()
class SUBMARINESIM_API APeriscopeActor : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    APeriscopeActor();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Function to rotate the periscope
    void RotatePeriscope(float AxisValue);

    // Public access to the Periscope Camera
    UCameraComponent* GetPeriscopeCamera() const { return PeriscopeCamera; }

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Periscope mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PeriscopeMesh;

    // Periscope camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* PeriscopeCamera;

private:
    // Rotation speed for the periscope
    float RotationSpeed;
};
