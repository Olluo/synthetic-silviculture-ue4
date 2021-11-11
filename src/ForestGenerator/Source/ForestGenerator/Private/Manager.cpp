// Ollie Nicholls, 2021


#include "Manager.h"

#include "BranchModuleManager.h"
#include "Plant.h"
#include "ForestGeneratorLog.h"

// Sets default values for this component's properties
UManager::UManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UManager::Initialize(const FSimulationSettings& Settings,
                          const TArray<TSubclassOf<UBranchModule>>& BranchModulePrototypes,
                          UDataTable* PlantTypes)
{
	ModuleManager = NewObject<UBranchModuleManager>();
	ModuleManager->Initialize(BranchModulePrototypes);

	// TODO choose a plant type properly. For now just take known plant type
	UE_LOG(LogForestGenerator, Log, TEXT("Manager: Plant types available:"));
	for (FName PlantName : PlantTypes->GetRowNames())
	{
		UE_LOG(LogForestGenerator, Log, TEXT(" - %s"), *PlantName.ToString());
	}
	const FPlantSettings PlantSettings = *PlantTypes->FindRow<FPlantSettings>("Testing", "");

	Plants = TArray<UPlant*>();
	for (int32 i = 0; i < Settings.NumberOfPlants; i++)
	{
		// Choose random position TODO this would need an initial seeding then operate on plants reproducing
		const float FloatI = static_cast<float>(i * 40);
		FVector Position = FVector{FloatI, FloatI, FloatI};

		UPlant* NewPlant = NewObject<UPlant>();

		// Set parameters on plant
		NewPlant->Initialize(ModuleManager, Position, PlantSettings);

		// Add new plant to array so we can keep track of it in our sim loops
		Plants.Add(NewPlant);
	}
}

void UManager::Simulate(const FSimulationSettings& Settings,
                        const TArray<TSubclassOf<UBranchModule>>& BranchModulePrototypes,
                        UDataTable* PlantTypes)
{
	Initialize(Settings, BranchModulePrototypes, PlantTypes);

	for (int32 i = 0; i < Settings.Time; i += Settings.TimeStep)
	{
		TArray<UPlant*> Temp;

		// The module manager keeps track of all modules and calculates all light exposures as each module needs to
		// know what its neighbors are
		ModuleManager->CalculateLightExposures();

		for (UPlant* Plant : Plants)
		{
			Plant->Simulate(Settings.TimeStep);

			if (Plant->GetState() != EPlantState::Dead)
			{
				Temp.Add(Plant);
			}
		}

		Plants = Temp;
	}
}

void UManager::Render()
{
	if (!WorldContext)
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Manager: Can't render: World Context not available"));
		return;
	}

	UE_LOG(LogForestGenerator, Verbose, TEXT("Manager: Rendering available"));

	for (UPlant* Plant : Plants)
	{
		Plant->DrawDebug(WorldContext);
	}
}

void UManager::SetWorldContext(const UWorld* NewWorldContext)
{
	WorldContext = NewWorldContext;
}
