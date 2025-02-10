#include "PeriscopeOverlayUI.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TorpedoLauncher.h"
#include "EngineUtils.h"


// Note: This code dynamically assigns TorpedoPipeButtons and TorpedoPipeTextBlocks because elements
// assigned through Blueprints may be lost during compilation or edits due to a bug in Unreal Engine.
// The workaround avoids reliance on Blueprint bindings by programmatically finding widgets
// by name at runtime.

// Due to how UE's object duplication works in the editor, the first text block uses the name "SelectedText"
// (without a "_1" suffix), while subsequent text blocks use "SelectedText_1", "SelectedText_2", etc.
// This naming pattern is handled here to ensure all elements are dynamically assigned correctly.

void UPeriscopeOverlayUI::NativeConstruct()
{
    Super::NativeConstruct();

    // Find the submarine in the world by its tag name
    AActor* Submarine = nullptr;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (It->ActorHasTag(FName("Submarine"))) // Match by tag
        {
            Submarine = *It;
            UE_LOG(LogTemp, Log, TEXT("Submarine found and stored: %s"), *Submarine->GetName());
            break;
        }
    }

    if (!Submarine)
    {
        UE_LOG(LogTemp, Warning, TEXT("Submarine not found in the world!"));
        return;
    }

    // Get the TorpedoLauncher component from the submarine
    TorpedoLauncher = Submarine->FindComponentByClass<UTorpedoLauncher>();
    if (!TorpedoLauncher)
    {
        UE_LOG(LogTemp, Warning, TEXT("TorpedoLauncher component not found on submarine!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("TorpedoLauncher found successfully on the submarine."));


    // Bind the Launch button to the LaunchTorpedoes function
    if (LaunchButton)
    {
        LaunchButton->OnClicked.AddDynamic(this, &UPeriscopeOverlayUI::LaunchTorpedoes);
        UE_LOG(LogTemp, Log, TEXT("LaunchButton successfully bound to LaunchTorpedoes"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("LaunchButton is not assigned!"));
    }

    if (DistanceInput)
    {
        // Bind the OnTextChanged event to the OnDistanceInputChanged function
        DistanceInput->OnTextChanged.AddDynamic(this, &UPeriscopeOverlayUI::OnDistanceInputChanged);
        UE_LOG(LogTemp, Log, TEXT("Bound OnDistanceInputChanged to DistanceInput's OnTextChanged event."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DistanceInput is not assigned in NativeConstruct!"));
    }

    // Hide DistanceWarningText initially
    if (DistanceWarningText)    
        DistanceWarningText->SetText(FText::GetEmpty());
    
    if (MeasureDistanceButton)
    {
        MeasureDistanceButton->OnClicked.AddDynamic(this, &UPeriscopeOverlayUI::MeasureDistance);
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("MeasureDistanceButton is not assigned!"));
    }

    // Clear the arrays to avoid duplicates if this is called more than once
    TorpedoPipeButtons.Empty();
    TorpedoPipeTextBlocks.Empty();

    // Populate the arrays dynamically
    for (int32 Index = 0; Index < 5; ++Index) // Assuming you have 5 torpedo pipes
    {
        // Handle button names
        FString ButtonName;
        if (Index == 0) // Special case for the first button
        {
            ButtonName = TEXT("TorpedoPipeButton");
        }
        else
        {
            ButtonName = FString::Printf(TEXT("TorpedoPipeButton_%d"), Index);
        }

        UButton* FoundButton = Cast<UButton>(GetWidgetFromName(FName(*ButtonName)));
        if (FoundButton)
        {
            TorpedoPipeButtons.Add(FoundButton);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find button: %s"), *ButtonName);
        }

        // Handle text block names
        FString TextBlockName;
        if (Index == 0) // Special case for the first text block
        {
            TextBlockName = TEXT("SelectedText");
        }
        else
        {
            TextBlockName = FString::Printf(TEXT("SelectedText_%d"), Index);
        }

        UTextBlock* FoundTextBlock = Cast<UTextBlock>(GetWidgetFromName(FName(*TextBlockName)));
        if (FoundTextBlock)
        {
            TorpedoPipeTextBlocks.Add(FoundTextBlock);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find text block: %s"), *TextBlockName);
        }
    }

    // Debug: Log the dynamically assigned elements
    for (int32 i = 0; i < TorpedoPipeButtons.Num(); ++i)
    {
        if (TorpedoPipeButtons[i])
        {
            UE_LOG(LogTemp, Log, TEXT("TorpedoPipeButtons[%d]: %s"), i, *TorpedoPipeButtons[i]->GetName());
        }
    }

    for (int32 i = 0; i < TorpedoPipeTextBlocks.Num(); ++i)
    {
        if (TorpedoPipeTextBlocks[i])
        {
            UE_LOG(LogTemp, Log, TEXT("TorpedoPipeTextBlocks[%d]: %s"), i, *TorpedoPipeTextBlocks[i]->GetName());
        }
    }

    // Initialize selection states
    TorpedoPipesSelected.SetNum(TorpedoPipeButtons.Num());
    for (bool& IsSelected : TorpedoPipesSelected)
    {
        IsSelected = true;
    }

    // DEBUG LINE, UNCOMMENT FOR QUICKER TESTING
	// TorpedoPipesSelected[0] = true;

    // Bind click events for all buttons
    for (int32 Index = 0; Index < TorpedoPipeButtons.Num(); ++Index)
    {
        if (TorpedoPipeButtons[Index])
        {
            UE_LOG(LogTemp, Log, TEXT("Add torpedo pipe button clicked for Index: %d"), Index);
            TorpedoPipeButtons[Index]->OnClicked.AddDynamic(this, &UPeriscopeOverlayUI::OnTorpedoPipeButtonClicked);
        }
    }
}

void UPeriscopeOverlayUI::MeasureDistance()
{
    if (!PeriscopeCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("PeriscopeCamera is not assigned!"));
        return;
    }

    FVector CameraLocation = PeriscopeCamera->GetComponentLocation();
    float ScreenHeightPercentageThreshold = 0.38f;

    // Initialize variables to track the closest "EnemyShip"
    AActor* ClosestEnemy = nullptr;
    float ClosestDistance = FLT_MAX;

    // Iterate over all actors with the "EnemyShip" tag
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;

        if (Actor->ActorHasTag(FName("EnemyShip")))
        {
            // Calculate the distance from the camera to the actor
            float Distance = FVector::Dist(CameraLocation, Actor->GetActorLocation());

            // Project the actor's world position to screen space
            FVector2D ScreenPosition;
            bool bProjected = UGameplayStatics::ProjectWorldToScreen(
                GetOwningPlayer(),
                Actor->GetActorLocation(),
                ScreenPosition
            );

            if (bProjected)
            {
                // Get the screen size
                FVector2D ViewportSize;
                GEngine->GameViewport->GetViewportSize(ViewportSize);

                // Calculate the distance from the screen center
                FVector2D ScreenCenter = ViewportSize / 2.0f;
                float DistanceFromCenter = FVector2D::Distance(ScreenPosition, ScreenCenter);

                // Check if the actor is within the 38% screen height threshold
                if (DistanceFromCenter <= ViewportSize.Y * ScreenHeightPercentageThreshold)
                {
                    // If this actor is closer than the previous closest, update
                    if (Distance < ClosestDistance)
                    {
                        ClosestDistance = Distance;
                        ClosestEnemy = Actor;
                    }
                }
            }
        }
    }

    // Update DistanceInput input field based on whether a closest enemy was found
    if (ClosestEnemy)
    {
        // Convert the distance to meters
        float DistanceInMeters = ClosestDistance / 100.0f;

        // Update DistanceInput
        if (DistanceInput)
        {
            FString FormattedDistance = FString::Printf(TEXT("%d"), FMath::RoundToInt(DistanceInMeters));
            FText FormattedDistanceText = FText::FromString(FormattedDistance);
            DistanceInput->SetText(FormattedDistanceText);

            // Call the OnDistanceInputChanged function manually with the updated text
            OnDistanceInputChanged(FormattedDistanceText);

            UE_LOG(LogTemp, Log, TEXT("Closest enemy ship '%s' is at a distance of %.2f meters."), *ClosestEnemy->GetActorLabel(), DistanceInMeters);
        }
    }
    else
    {
        // If no enemy is found, set distance to 0
        if (DistanceInput)
        {
            FString FormattedDistance = TEXT("0");
            FText FormattedDistanceText = FText::FromString(FormattedDistance);
            DistanceInput->SetText(FormattedDistanceText);

            // Call the OnDistanceInputChanged function manually with the updated text
            OnDistanceInputChanged(FormattedDistanceText);

            UE_LOG(LogTemp, Warning, TEXT("No enemy ship found within the view of the PeriscopeCamera. Distance set to 0."));
        }
    }
}

// Method for handling button clicks
void UPeriscopeOverlayUI::OnTorpedoPipeButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Torpedo button clicked"));

    // Find the index of the clicked button
    for (int32 Index = 0; Index < TorpedoPipeButtons.Num(); ++Index)
    {
        if (TorpedoPipeButtons[Index])
        {
            // Compare the clicked button with the current button in the array
            if (TorpedoPipeButtons[Index]->HasAnyUserFocus()) // Check if this is the button that received focus from the click
            {
                UE_LOG(LogTemp, Log, TEXT("Button clicked at index: %d"), Index);
                SelectTorpedoPipe(Index); // Call the function to toggle the state
                break;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Button at index %d is null"), Index);
        }
    }
}


void UPeriscopeOverlayUI::SelectTorpedoPipe(int PipeIndex)
{
    if (TorpedoPipesSelected.IsValidIndex(PipeIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("Toggling selection state for pipe at index: %d"), PipeIndex);

        // Toggle selection
        TorpedoPipesSelected[PipeIndex] = !TorpedoPipesSelected[PipeIndex];

        // Update the button text and color
        if (TorpedoPipeTextBlocks.IsValidIndex(PipeIndex))
        {
            FText NewText = TorpedoPipesSelected[PipeIndex] ? FText::FromString("SELECTED") : FText::FromString("NOT SELECTED");
            TorpedoPipeTextBlocks[PipeIndex]->SetText(NewText);

            FSlateColor NewColor = TorpedoPipesSelected[PipeIndex]
                ? FSlateColor(FLinearColor::Green) // Green for "SELECTED"
                : FSlateColor(FLinearColor::Red);   // Red for "NOT SELECTED"
            TorpedoPipeTextBlocks[PipeIndex]->SetColorAndOpacity(NewColor);

            UE_LOG(LogTemp, Log, TEXT("Torpedo pipe %d is now %s"), PipeIndex, *NewText.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Text block for pipe %d is not assigned"), PipeIndex);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid index for pipe selection: %d"), PipeIndex);
    }
}

void UPeriscopeOverlayUI::OnDistanceInputChanged(const FText& Text)
{    
    float Distance = FCString::Atof(*Text.ToString());
    if (Distance > 5000.0f)
    {
        DistanceWarningText->SetText(FText::FromString("Warning: Distance of the enemy ship exceeds 5000 meters!"));
    }
    else
    {
        DistanceWarningText->SetText(FText::GetEmpty());
    }
}

// TODO: Refactor so that LaunchTorpedoes() and LaunchSingleTorpedo() are in their own separate class that could be named "TorpedoLauncher"
void UPeriscopeOverlayUI::LaunchTorpedoes()
{
    if (!TorpedoLauncher)
    {
        UE_LOG(LogTemp, Warning, TEXT("TorpedoLauncher not found! Cannot launch torpedoes."));
        return;
    }

    if (!PeriscopeCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("PeriscopeCamera not found! Cannot calculate target direction."));
        return;
    }

    // Get input values
    float Distance = DistanceInput ? FCString::Atof(*DistanceInput->GetText().ToString()) * 100.0f : 1000.0f; // Convert to UE units (100 UE units = 1 meter)
    UE_LOG(LogTemp, Log, TEXT("LaunchTorpedoes: Retrieved DistanceInput = %s (%.2f UE Units)"), *DistanceInput->GetText().ToString(), Distance);

    float TargetSpeed = SpeedInput ? FCString::Atof(*SpeedInput->GetText().ToString()) * 100.0f : 0.0f; // Convert to UE units
    float AngleOnBow = BowAngleInput ? FCString::Atof(*BowAngleInput->GetText().ToString()) : 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Inputs: Distance = %.2f UE Units, TargetSpeed = %.2f, AngleOnBow = %.2f"), Distance, TargetSpeed, AngleOnBow);

    if (Distance <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Distance! Defaulting to 1000."));
    }

    // Torpedo speed in UE units per second (15.43 m/s)
    float TorpedoSpeed = 1543.0f;

    // Calculate the time it will take for the torpedo to reach the target
    float TimeToTarget = Distance / TorpedoSpeed;

    // Get the direction the periscope camera is pointing
    FVector CameraForward = PeriscopeCamera->GetForwardVector();

    // Get the location of the player's submarine
    FVector SubmarineLocation = TorpedoLauncher->GetOwner()->GetActorLocation();
    UE_LOG(LogTemp, Log, TEXT("Submarine Location: %s"), *SubmarineLocation.ToString());

    // Calculate the initial direction to the enemy ship (based on periscope)
    FVector EnemyInitialDirection = CameraForward * Distance;
    FVector EnemyInitialLocation = SubmarineLocation + EnemyInitialDirection;

    // Adjust Z of the initial enemy location
    EnemyInitialLocation.Z = -200.0f; // Set target Z to -200 (2 meters below surface)
    UE_LOG(LogTemp, Log, TEXT("Adjusted Enemy Initial Position: %s"), *EnemyInitialLocation.ToString());

    // Derive EnemyShipBowPointingDirection
    FVector EnemyToPlayer = SubmarineLocation - EnemyInitialLocation;
    float EnemyToPlayerAngle = FMath::Atan2(EnemyToPlayer.Y, EnemyToPlayer.X) * 180.0f / PI; // Convert to degrees
    if (EnemyToPlayerAngle < 0.0f)
    {
        EnemyToPlayerAngle += 360.0f; // Normalize to [0, 360)
    }

    // Calculate the EnemyShipBowPointingAngle from AngleOnBow
    float EnemyShipBowPointingAngle = FMath::Fmod(EnemyToPlayerAngle - AngleOnBow, 360.0f);
    if (EnemyShipBowPointingAngle < 0.0f)
    {
        EnemyShipBowPointingAngle += 360.0f; // Ensure positive angle
    }

    FVector EnemyShipBowPointingDirection = FRotationMatrix(FRotator(0, EnemyShipBowPointingAngle, 0)).GetUnitAxis(EAxis::X);

    UE_LOG(LogTemp, Log, TEXT("AngleOnBow: %.2f, EnemyShipBowPointingAngle: %.2f"), AngleOnBow, EnemyShipBowPointingAngle);
    UE_LOG(LogTemp, Log, TEXT("EnemyShipBowPointingDirection: %s"), *EnemyShipBowPointingDirection.ToString());

    // Calculate the predicted future position of the enemy ship
    FVector TargetFutureLocation = EnemyInitialLocation + (EnemyShipBowPointingDirection * TargetSpeed * TimeToTarget);

    // Adjust Z of the future target location
    TargetFutureLocation.Z = -200.0f; // Ensure Z remains -200
    UE_LOG(LogTemp, Log, TEXT("Adjusted Predicted Target Future Location: %s"), *TargetFutureLocation.ToString());

    // Launch torpedoes from selected pipes with a delay
    int32 LaunchDelay = 1; // Delay between launches in seconds
    float CurrentDelay = 0.2; // Initialize delay tracker

    for (int32 i = 0; i < TorpedoPipesSelected.Num(); i++)
    {
        if (TorpedoPipesSelected[i])
        {
            // Create a unique timer handle for this pipe
            FTimerHandle TimerHandle;

            // Create a delegate to pass the PipeIndex and TargetFutureLocation
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUObject(
                this,
                &UPeriscopeOverlayUI::LaunchSingleTorpedo,
                i, // PipeIndex
                TargetFutureLocation // Target location
            );

            // Schedule the launch for this pipe
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                TimerDelegate,
                CurrentDelay, // Delay before firing
                false // Do not loop                
            );

            // Increment the delay for the next torpedo
            CurrentDelay += LaunchDelay;
        }
    }
}


void UPeriscopeOverlayUI::LaunchSingleTorpedo(int32 PipeIndex, FVector TargetFutureLocation)
{
	UE_LOG(LogTemp, Log, TEXT("Launching torpedo from pipe %d"), PipeIndex);
    // Torpedo speed in UE units per second
    float TorpedoSpeed = 1543.0f; // TODO: Refactor, as this is redundant and should be fetched from a centralized location

    if (TorpedoLauncher && TorpedoLauncher->SpawnPoints.IsValidIndex(PipeIndex))
    {
        // Get the spawn point for the given pipe
        FTransform SpawnTransform = TorpedoLauncher->SpawnPoints[PipeIndex];
        FVector SpawnLocation = SpawnTransform.GetLocation();
        SpawnLocation.Z = -200.0f; // Ensure consistent Z-coordinate for the spawn location

        // Fire the torpedo
        UE_LOG(LogTemp, Log, TEXT("Pipe %d: SpawnLocation = %s, TargetLocation = %s"), PipeIndex, *SpawnLocation.ToString(), *TargetFutureLocation.ToString());
        TorpedoLauncher->FireTorpedoFromPipe(PipeIndex, TargetFutureLocation);

        // Calculate the direction from the pipe to the target
        FVector DirectionToTarget = (TargetFutureLocation - SpawnLocation).GetSafeNormal();

        // Calculate the estimated contact point
        float DistanceToTarget = FVector::Dist(TargetFutureLocation, SpawnLocation);
        float TimeToImpact = DistanceToTarget / TorpedoSpeed; // Assume constant torpedo speed

        FVector EstimatedContactPoint = SpawnLocation + DirectionToTarget * TorpedoSpeed * TimeToImpact;

        // Draw a debug sphere at the estimated contact point
        DrawDebugSphere(
            GetWorld(),
            EstimatedContactPoint,
            1000.0f, // Radius of the sphere
            12, // Segments for better visibility
            FColor::Red,
            false, // Persistent (set to true for debugging across frames)
            10.0f // Lifetime of the sphere
        );

        UE_LOG(LogTemp, Log, TEXT("Estimated contact point for pipe %d: %s"), PipeIndex, *EstimatedContactPoint.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid spawn point for pipe %d"), PipeIndex);
    }
}
