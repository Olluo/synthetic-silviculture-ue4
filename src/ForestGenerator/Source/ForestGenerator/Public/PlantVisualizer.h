// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "PlantVisualizer.generated.h"

class UBranchModule;
class UBranchModuleManager;
class UDataTable;
class UPlant;

/**
 * @brief This actor is used to visualise a single plants growth. Useful for playing around with the settings for
 * specific plants then using those settings in the main Generator.
 */
UCLASS(BlueprintType)
class FORESTGENERATOR_API APlantVisualizer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlantVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/**
	 * @brief The main execution loop of this plant. When this is called one time step of the plant is simulated.
	 * Cannot be called unless bInitialized is true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Forest Generator")
	void Simulate();

protected:

	/**
	 * @brief The plant type data to use when generating this plant. Will use the first row in the data table.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	UDataTable* PlantTypes;

	/**
	* @brief The maximum time the simulations runs for.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Forest Generator",
		meta = (ClampMin = "1", ClampMax = "10000"))
	int32 Time = 1;

	/**
	* @brief The Branch Module Prototypes to use.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	TArray<TSubclassOf<UBranchModule>> BranchModulePrototypes;
	
	/**
	* @brief To render a tree we use instanced static meshes, so this component tracks and spawns them.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest Generator")
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;

	/**
	* @brief Keeps track of all the branch modules and is used to call all the CalculateLightExposure methods on each
	* of the branch modules.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	UBranchModuleManager* ModuleManager;

	/**
	* @brief The plant that is being visualized here.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	UPlant* Plant;

	/**
	* @brief Bounding box of the InstancedStaticMeshComponent.
	*/
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	FBox StaticMeshBoundingBox;

	/**
	 * @brief Used to run the simulation method continually.
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	FTimerHandle SimulationTimerHandle;

	/**
	 * @brief If the initialization of this object has been done.
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Forest Generator")
	bool bInitialized = false;

private:
	/**
	 * @brief Renders the current plant. Called during simulation.
	 */
	void Render();
};
