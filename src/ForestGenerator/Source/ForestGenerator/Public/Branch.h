#pragma once

#include "CoreMinimal.h"

#include "Branch.generated.h"

/**
 * @brief Used to describe the branch transform. Useful when rendering.
 */
USTRUCT(BlueprintType)
struct FBranch
{
	GENERATED_BODY()

	/**
	 * @brief The source position of this branch
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	FVector Start = FVector::ZeroVector;

	/**
	* @brief The destination position of this branch
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	FVector End = FVector::UpVector;

	/**
	* @brief The diameter of this branch
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	float Diameter = 1.f;

	FBranch() = default;

	FBranch(const FVector& Start, const FVector& End, const float Diameter)
		: Start(Start),
		  End(End),
		  Diameter(Diameter)
	{
	}

	/**
	 * @brief This is used to get a transform that can be applied to the static mesh so that it is now this cylinder
	 * @param StaticMeshBoundingBox The bounding box of the static mesh cylinder used
	 * @return A transform that can be used to transform the static mesh cylinder to be the same as this
	 */
	FTransform GetCylinderTransform(const FBox& StaticMeshBoundingBox) const
	{
		// First get the sizes of the static mesh
		const FVector StaticMeshBoxSize = StaticMeshBoundingBox.GetSize();
		const float StaticMeshHeight = StaticMeshBoxSize.Z;
		const float StaticMeshDiameter = StaticMeshBoxSize.X;

		// Then get the sizes of this cylinder
		const FVector CylinderSize = End - Start;
		const float CylinderHeight = CylinderSize.Size();

		// Then calculate what to transform the static mesh by to get the correct sizing
		const float HeightTransform = CylinderHeight / StaticMeshHeight;
		float DiameterTransform = Diameter / StaticMeshDiameter;

		// There was an issue with there being DiameterTransform = 0
		if (DiameterTransform == 0.f)
		{
			DiameterTransform = 0.01f;
		}

		// Calculate the midpoint, used to transform the cylinder to the correct position
		const FVector Midpoint{0.f, 0.f, CylinderHeight / 2.f};

		// Calculate the rotator that will rotate the up vector of the static mesh so that it is the same direction as our cylinder
		const FRotator Rotation = CylinderSize.ToOrientationRotator() - FVector::UpVector.ToOrientationRotator();
		// The translation is the start of our cylinder plus the midpoint rotated using the rotator
		const FVector Translation = Start + Rotation.RotateVector(Midpoint);
		const FVector Scale{DiameterTransform, DiameterTransform, HeightTransform};

		// Finally return the transform
		return FTransform{Rotation, Translation, Scale};
	}
};
