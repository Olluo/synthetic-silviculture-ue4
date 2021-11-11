// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"

#include "Branch.h"
#include "UObject/NoExportTypes.h"
#include "BranchNode.generated.h"

class UBranchSegment;

/**
 * @brief The type of the node.
 */
UENUM(BlueprintType)
enum class ENodeType : uint8
{
	Root UMETA(DisplayName = "Root Node"),
	Normal UMETA(DisplayName = "Normal Node"),
	Connecting UMETA(DisplayName = "Connecting Node"),
	Terminal UMETA(DisplayName = "Terminal Node")
};

UENUM(BlueprintType)
enum class ENodeSortMark : uint8
{
	None UMETA(DisplayName = "No Mark"),
	Temporary UMETA(DisplayName = "Temporary Mark"),
	Permanent UMETA(DisplayName = "Permanent Mark")
};

/**
 * A branch node in the branch module graph as described in section 5.1.
 */
UCLASS(BlueprintType)
class FORESTGENERATOR_API UBranchNode : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Add a new child branch to the list of children.
	 * @param Child The child branch
	 * @param bIsChildModule If this node is a part of a different module
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void AddChildBranch(UBranchSegment* Child, const bool bIsChildModule = false);

	/**
	 * @brief Set the parent branch.
	 * @param InParent The new parent
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetParentBranch(UBranchSegment* InParent);

	/**
	 * @brief Translate this node and all its available children.
	 * @param Translation The vector translation
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void Translate(const FVector& Translation);

	/**
	 * @brief Set this node to be type root.
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetRoot();

	/**
	 * @brief Increase the age of this node and all available children.
	 * @param DeltaAge How much to increase the age by
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void IncreaseAge(const float DeltaAge);

	/**
	 * @brief Recalculate the direction of this node from its parent.
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void RecalculateDirection();

	/**
	 * @brief Set the ID
	 */
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetID(const uint8 InID);

	/**
	* @brief Set the vigor
	*/
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetVigor(const float InVigor);
	
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetLightExposure(const float InLightExposure);
	
	UFUNCTION(BlueprintSetter, Category = "ForestGen")
	void SetDirection(const FRotator& Rotator);

	/**
	 * @brief Return the ID
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	uint8 GetID() const;

	/**
	 * @brief Get the position of the parent.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	FVector GetParentPosition() const;

	/**
	 * @brief Verify if the parent branch is available.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	bool IsParentBranchAvailable() const;

	/**
	 * @brief Get the diameter of the parent branch
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	float GetParentBranchDiameter() const;

	/**
	 * @brief Get the parent branch.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	UBranchSegment* GetParentBranch() const;

	/**
	 * @brief Get the position of the node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	const FVector& GetPosition() const;

	/**
	 * @brief Get the direction from the parent to this node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	const FVector& GetDirection() const;

	/**
	 * @brief Get the age of the node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	float GetAge() const;

	/**
	 * @brief Get the children branches attached to this node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	TArray<UBranchSegment*> GetChildrenBranches() const;

	/**
	 * @brief Get all the available children branches attached to this node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	TArray<UBranchSegment*> GetAvailableChildrenBranches() const;

	/**
	 * @brief Get the available children nodes attached to this node
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	TArray<UBranchNode*> GetAvailableChildren(const bool bIncludeChildModule = false);

	/**
	 * @brief Get the length of the branch from the parent to this node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	float GetParentBranchLength() const;

	/**
	 * @brief Get all the children attached to this node.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	TArray<UBranchNode*> GetChildren();

	/**
	 * @brief Verify if this node is type root.
	 */
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	bool IsRoot() const;

	/**
	* @brief Verify if this node is type terminal.
	*/
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	bool IsTerminal() const;

	/**
	* @brief Get the vigor of this node.
	*/
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	float GetVigor() const;
	
	UFUNCTION(BlueprintGetter, Category = "ForestGen")
	float GetLightExposure() const;

	/**
	 * @brief Draw this node using DrawDebug methods.
	 * @param WorldContext A WorldContext object that will be drawn to.
	 */
	UFUNCTION(BlueprintCallable, Category = "ForestGen")
	void DrawDebug(const UWorld* WorldContext);

	/**
	 * @brief Get all the FBranches of this node and all attached available children.
	 */
	TArray<FBranch> GetBranchTransforms();
	
	void Visit(TArray<UBranchNode*>& SortedNodes);
	void ResetSortMark();
	void ResetToTerminal();

protected:
	/**
	 * @brief The ID.
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	uint8 ID = 0;

	/**
	* @brief The parent branch segment.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	UBranchSegment* Parent;

	/**
	* @brief The children branch segments.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	TArray<UBranchSegment*> ChildrenBranches;

	/**
	* @brief The position.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	FVector Position = FVector::ZeroVector;

	/**
	 * @brief This is a unit vector direction from Parent.
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	FVector Direction = FVector::UpVector;

	/**
	* @brief The age.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	float PhysiologicalAge = 0;

	/**
	* @brief The type.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	ENodeType Type = ENodeType::Terminal;

	/**
	* @brief The vigor.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	float Vigor = 0.f;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ForestGen")
	float LightExposure = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	ENodeSortMark SortMark = ENodeSortMark::None;
};
