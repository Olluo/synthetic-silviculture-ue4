// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"

#include "BranchSegment.generated.h"

class UBranchNode;

/**
* @brief An edge (referred to as branch segment) in the graph.
* Connects two nodes n1 & n2 and represents an individual branch segment in the plant.
*/
UCLASS(BlueprintType)
class FORESTGENERATOR_API UBranchSegment : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void Initialize(UBranchNode* InSource, UBranchNode* InDestination);

	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetDiameter(const float InDiameter);

	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void MakeAvailable();
	
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetDepth(int32 InDepth);

	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	UBranchNode* GetSource() const;

	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	UBranchNode* GetDestination() const;

	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	float GetDiameter() const;

	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	bool IsAvailable() const;

	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	int32 GetDepth() const;

	UFUNCTION(BlueprintCallable, Category = "ForestGen")
	FString ToString() const;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	class UBranchNode* Source;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	class UBranchNode* Destination;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	float Diameter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	bool bAvailable = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	int32 Depth = 0;
};
