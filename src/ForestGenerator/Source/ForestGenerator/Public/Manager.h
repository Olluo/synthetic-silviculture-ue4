// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BranchModule.h"
#include "BranchModuleManager.h"


#include "Manager.generated.h"

USTRUCT(BlueprintType)
struct FSimulationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen")
	int32 NumberOfPlants = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen")
	int32 MaxNumberOfPlants = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen")
	int32 Time = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen")
	float TimeStep = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen")
	float Temperature = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen")
	float Precipitation = 1392.0f;

	FSimulationSettings() = default;

	FSimulationSettings(const int32 NumberOfPlants, const int32 MaxNumberOfPlants, const int32 Time,
	                    const float TimeStep, const float Temperature, const float Precipitation)
		: NumberOfPlants(NumberOfPlants),
		  MaxNumberOfPlants(MaxNumberOfPlants),
		  Time(Time),
		  TimeStep(TimeStep),
		  Temperature(Temperature),
		  Precipitation(Precipitation)
	{
	}
};

/**
 * Tracks trees
 * stores ecosystem params
 * Ticks/simulates each tree
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORESTGENERATOR_API UManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "ForestGen|Manager")
	void Initialize(const FSimulationSettings& Settings,
	                const TArray<TSubclassOf<UBranchModule>>& BranchModulePrototypes,
	                class UDataTable* PlantTypes);

	UFUNCTION(BlueprintCallable, Category = "ForestGen|Manager")
	void Simulate(const FSimulationSettings& Settings,
	              const TArray<TSubclassOf<UBranchModule>>& BranchModulePrototypes,
	              class UDataTable* PlantTypes);

	UFUNCTION(BlueprintCallable, Category = "ForestGen|Manager")
	void Render();

	UFUNCTION(BlueprintCallable, Category = "ForestGen|Manager")
	void SetWorldContext(const UWorld* NewWorldContext);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Manager")
	TArray<class UPlant*> Plants;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Manager")
	const UWorld* WorldContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Manager")
	UBranchModuleManager* ModuleManager;
};
