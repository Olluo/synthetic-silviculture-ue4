// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BranchModule.h"
#include "Plant.h"

#include "Generator.generated.h"


class UManager;
/**
 * The actor that is placed in the world responsible for positioning the forest and grabbing vertex data for the manager
 */
UCLASS(BlueprintType)
class FORESTGENERATOR_API AGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "ForestGen|Generator")
	void Simulate();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Generator")
	class UBillboardComponent* BillboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Generator")
	UManager* ManagerComponent;

	/**
	 * @brief The starting number of plants that are spawned
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen|Generator",
		meta = (ClampMin = "1", ClampMax = "100"))
	int32 NumberOfPlants = 1;

	/**
	* @brief The maximum number of plants that can be spawned
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen|Generator",
		meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxNumberOfPlants = 100;

	/**
	* @brief The maximum time the simulations runs for
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen|Generator",
		meta = (ClampMin = "1", ClampMax = "10000"))
	int32 Time = 1;

	/**
	* @brief The time between each simulation step
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen|Generator",
		meta = (ClampMin = "0.0", ClampMax = "10000"))
	float TimeStep = 1.f;

	/**
	* @brief The Branch Module Prototypes to use
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ForestGen|Generator")
	TArray<TSubclassOf<UBranchModule>> BranchModulePrototypes;

	// TArray<TSubclassOf<UPlant>> PlantTypes;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ForestGen|Generator")
	class UDataTable* PlantTypes;

	/**
	* @brief The temperature in C
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen|Generator",
		meta = (ClampMin = "-10.0", ClampMax = "33.0"))
	float Temperature = 20.0f;

	/**
	* @brief The amount of precipitation in mm
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen|Generator",
		meta = (ClampMin = "10.0", ClampMax = "4300.0"))
	float Precipitation = 1392.0f;
};
