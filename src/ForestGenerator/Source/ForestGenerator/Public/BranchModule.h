// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"

#include "Branch.h"
#include "UObject/NoExportTypes.h"

#include "BranchModule.generated.h"

class UBranchNode;
class UBranchSegment;
class UBranchModuleManager;

/**
* @brief 
*/
USTRUCT(BlueprintType)
struct FGraphEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Generator")
	int32 Source = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Generator")
	int32 Destination = -1;

	FGraphEdge() = default;

	FGraphEdge(const int32 Source, const int32 Destination)
		: Source(Source),
		  Destination(Destination)
	{
	}
};

/**
* @brief A definition that describes how the branch module graph is structured
*/
USTRUCT(BlueprintType)
struct FGraphDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Generator")
	TArray<FGraphEdge> Edges;

	FGraphDefinition() = default;

	FGraphDefinition(const TArray<FGraphEdge>& Edges)
		: Edges(Edges)
	{
	}
};

/**
* @brief
*/
USTRUCT(BlueprintType)
struct FBranchGraph
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	UBranchNode* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	TArray<UBranchNode*> Nodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	TArray<UBranchSegment*> Branches;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	TArray<UBranchSegment*> AvailableBranches;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	int32 PhysiologicalAge = 0;

	FBranchGraph() = default;
};

/**
* @brief This is used when calculating vigor as the graph needs to be topologically sorted
*/
UENUM(BlueprintType)
enum class ESortMark : uint8
{
	None UMETA(DisplayName = "No Mark"),
	Temporary UMETA(DisplayName = "Temporary Mark"),
	Permanent UMETA(DisplayName = "Permanent Mark")
};

/**
 * 
 */
UCLASS(Blueprintable)
class FORESTGENERATOR_API UBranchModule : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Forest Generator")
	FGraphDefinition GetGraphDefinition();
	virtual FGraphDefinition GetGraphDefinition_Implementation();

	UFUNCTION(BlueprintSetter, Category = "Forest Generator")
	void Initialize(const FGraphDefinition& NewGraphDefinition, const FVector& InPosition,
	                UBranchModuleManager* InModuleManager, const FRotator InOrientation);

	UFUNCTION(BlueprintSetter, Category = "Forest Generator")
	void SetID(const int32 InID);

	UFUNCTION(BlueprintSetter, Category = "Forest Generator")
	void SetVigor(const float InVigor);

	UFUNCTION(BlueprintSetter, Category = "Forest Generator")
	void ResetSortMark();

	UFUNCTION(BlueprintSetter, Category = "Forest Generator")
	void IncreaseLightExposure(const float InLightExposure);

	UFUNCTION(BlueprintGetter, Category = "Forest Generator")
	int32 GetID() const;

	UFUNCTION(BlueprintGetter, Category = "Forest Generator")
	float GetLightExposure() const;

	UFUNCTION(BlueprintGetter, Category = "Forest Generator")
	float GetVigor() const;

	UFUNCTION(BlueprintGetter, Category = "Forest Generator")
	const TArray<UBranchModule*>& GetChildren() const;

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void DrawBoundingSphere(const UWorld* WorldContext) const;

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void DrawDebug(const UWorld* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void Visit(TArray<UBranchModule*>& SortedNodes);

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	UBranchNode* GetRootNode();
	
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	UBranchModule* AttachNewBranchModule(UBranchNode* ParentNode, const float ApicalControl, const float Determinacy);

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void CalculatePerNodeVigor(const float ApicalControl);

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void Shed();

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	bool IsShed();
	
	/**
	* @brief Section 5.3 of the paper.
	* The main module development function that handles aging, adding new nodes,
	* adapting node positions due to tropism, attaching new modules, and branch segment growing.
	* @param DT Delta time since last call, the simulation time step in the main loop
	* @param VMin Minimum vigor clamp
	* @param VMax Maximum vigor clamp
	* @param GP Growth potential
	* @param Phi The default thickness of branches
	* @param Beta Branch length scaling coefficient
	* @param LMax Max branch length
	* @param G1 Control how fast the effect of tropism decreases with time
	* @param Alpha Control the angle of tropism, negative represents gravitropsim, positive phototropism
	* @param GDir Normalized direction of gravity
	* @param TropismStrength The overall strength of the tropism
	* @param Straightness How straight the growth of the plant is
	* @param ApicalControl The ratio of limiting lateral buds leading to a plant developing a trunk
	* @param Determinacy Where buds develop into flowers preventing further growth
	* @param bCanSpawnChildren If the branch can spawn children or not
	*/
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void Grow(const float DT, const float VMin, const float VMax, const float GP, const float Phi,
	          const float Beta, const float LMax, const float G1, const float Alpha,
	          const FVector& GDir, const float TropismStrength, const float Straightness, const float ApicalControl,
	          const float Determinacy, const bool bCanSpawnChildren);

	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	TArray<UBranchNode*> TopologicalSortNodes() const;

	TArray<FBranch> GetBranchTransforms() const;

	void Orientate(const TArray<FSphere> Neighbors, const FRotator& InitialOrientation);
	void CalculateLightExposure(const TArray<FSphere>& IntersectingNeighbors);
	const FSphere& GetBoundingSphere() const;
	float GetAge();
	static float CalculateCollisions(const FSphere& Sphere, const TArray<FSphere>& IntersectingNeighbors);
	static float CalculateIntersectingVolume(const FSphere& Sphere, const FSphere& Neighbor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Generator")
	FGraphDefinition GraphDefinition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	FBranchGraph Graph;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	TArray<UBranchNode*> FinalNodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	float PhysiologicalAge = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	float AgeMature = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	UBranchModuleManager* ModuleManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	TArray<UBranchModule*> Children;

	/**
	* @brief The ID
	* This is helpful when determining if this branch is main or lateral as main branch will always have a lower ID
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	int32 ID = -1;

	FSphere BoundingSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	float LightExposure = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	float Vigor = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	ESortMark SortMark = ESortMark::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	FRotator Orientation = FRotator::ZeroRotator;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	bool bShed = false;

private:
	void CalculateBoundingSphere();
	void SpawnChildNodes(UBranchNode* Parent, const float Straightness) const;
	void GrowGraph(const float Straightness);
	void IncreaseAge(const float DeltaAge, const float Straightness);
	TArray<UBranchNode*> GetAvailableNodes();
	TArray<UBranchNode*> GetTerminalNodes();

	bool bTesting = true;
};
