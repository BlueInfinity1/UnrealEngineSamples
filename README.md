# Unreal Engine C++ Samples: Periscope View & Torpedo Launching

This repository contains sample C++ code from an Unreal Engine 5 prototype project demonstrating a periscope UI for targeting and launching torpedoes in 3D game with a submarine. The implementation showcases dynamic UI element functionality, real-time enemy distance measurement, and predictive projectile path calculation.

## Video Demonstration

In the demonstration video, you can see how different values of the input field affect the target point that is chosen for each torpedo. The target points are marked by red wireframe spheres.

Watch the video [here](https://drive.google.com/file/d/1uc4X6cZZFsvT3vMQW0IELtLcYcGjPqGq/view?usp=drive_link).

> **Note:** In the video, the Submarine has been replaced with an **Airship** to better demonstrate the torpedo projectile (depicted by a grey sphere) paths. The field of vision of the periscope is larger than normally, as the dark cover around the periscope lens is transparent.
Enemy ships are depicted by white pillars.

## Code Overview

The main functionality is implemented in the `PeriscopeOverlayUI` class. Key aspects include:
  
- **Distance Measurement:**
  The `MeasureDistance` function projects the 3D positions of enemy ships into the 2D view of the camera, then identifies the enemy ship closest to the center of the screen. The ship must be approximately within the view of the periscope lens (center area not covered by the dark veil) for the measurement to work. This allows the system to update the distance input field based on the most relevant target within the periscope's view.

- **Torpedo Pipe Selection:**  
  The functions `OnTorpedoPipeButtonClicked` and `SelectTorpedoPipe` manage user input for selecting or deselecting torpedo pipes. The UI updates the button text and color to indicate selection status.

- **Launching Torpedoes:**  
  The `LaunchTorpedoes` method calculates the expected future position of the enemy based on current input values (distance, speed, and bow angle) and schedules the torpedo launches with delays. The `LaunchSingleTorpedo` function handles the actual firing from each selected torpedo pipe, complete with debug visualization of the projectile's estimated contact point.
