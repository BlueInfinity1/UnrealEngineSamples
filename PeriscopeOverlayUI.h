#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include <SubmarineSim/SubmarineSimCharacter.h>
#include "PeriscopeOverlayUI.generated.h"

// Forward declarations
class UButton;
class UTextBlock;
class UEditableTextBox;
class UTorpedoLauncher;

UCLASS()
class SUBMARINESIM_API UPeriscopeOverlayUI : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    UTorpedoLauncher* TorpedoLauncher;

    // Periscope Camera (assigned by the PlayerController)
    UPROPERTY(BlueprintReadOnly, Category = "Periscope", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* PeriscopeCamera;

    // Measure Distance Button
    UPROPERTY(meta = (BindWidget))
    UButton* MeasureDistanceButton;

    UPROPERTY(meta = (BindWidget))
    UButton* LaunchButton;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torpedo Pipes", meta = (AllowPrivateAccess = "true"))
    TArray<UButton*> TorpedoPipeButtons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torpedo Pipes", meta = (AllowPrivateAccess = "true"))
    TArray<UTextBlock*> TorpedoPipeTextBlocks;

    UFUNCTION(BlueprintCallable, Category = "Periscope")
    void MeasureDistance();

    UFUNCTION()
    void OnDistanceInputChanged(const FText& Text);

    // Function to select a torpedo pipe
    UFUNCTION()
    void SelectTorpedoPipe(int PipeIndex);
    
    // Function to launch torpedoes
    UFUNCTION()
    void LaunchTorpedoes();

    void LaunchSingleTorpedo(int32 PipeIndex, FVector TargetLocation);

protected:
    // Called when the widget is constructed
    virtual void NativeConstruct() override;

    // Function called when a torpedo pipe button is clicked
    UFUNCTION()
    void OnTorpedoPipeButtonClicked();

private:
    // Boolean array to keep track of selected pipes
    TArray<bool> TorpedoPipesSelected;

    // Array to store button indices
    TArray<int32> TorpedoPipeButtonIndices;

    // Editable text box for distance input
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* DistanceInput;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* SpeedInput;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* BowAngleInput;

    // Text block for distance warning
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DistanceWarningText;
};
