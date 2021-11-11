// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BranchModuleManager.generated.h"

class UBranchModule;
struct FGraphDefinition;

/**
 * This is used to keep track of all the branch modules in the simulation and is responsible for calling methods
 * that need to be called on all current branch modules.
 */
UCLASS(BlueprintType, Blueprintable)
class FORESTGENERATOR_API UBranchModuleManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initializes the manager.
	 * @param BranchModulePrototypes The prototypes to use in the simulation
	 * @return If initialization was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	bool Initialize(const TArray<TSubclassOf<UBranchModule>>& BranchModulePrototypes);

	/**
	 * @brief Section 5.2.2: Selects from the list of graph prototypes and generate a branch module based on the input params.
	 * @param ApicalControl Lambda
	 * @param Determinacy D
	 * @param InPosition The position where the branch module will be spawned
	 * @param InitialOrientation The initial orientation used when optimizing the orientation
	 * @return The newly created branch module
	 */
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	UBranchModule* GenerateBranchModule(int32 ApicalControl, int32 Determinacy, const FVector& InPosition,
	                                    const FRotator& InitialOrientation = FRotator::ZeroRotator);

	/**
	 * @brief Signal all branch modules to calculate their light exposure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void CalculateLightExposures();

	/**
	 * @brief Removes a branch module from the simulation.
	 * @param BranchModule The branch module
	 */
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void RemoveModule(UBranchModule* BranchModule);
	
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	int GetNumberOfModules() const;

protected:
	/**
	 * @brief The graph prototypes that can be chosen from.
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	TArray<FGraphDefinition> GraphPrototypes;

	/**
	 * @brief All the branch modules in the simulation 
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	TArray<UBranchModule*> BranchModules;

	/**
	 * @brief Which ID to give to the next spawned branch module
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	int32 NextID = 0;

	/**
	* @brief 
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	bool bInitialized = false;

private:
	TArray<FSphere> GetNeighborBoundingSpheres(UBranchModule* QueryModule) const;
};
