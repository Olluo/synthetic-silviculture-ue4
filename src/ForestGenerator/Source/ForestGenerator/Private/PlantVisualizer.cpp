// Ollie Nicholls, 2021


#include "PlantVisualizer.h"

#include "BranchModuleManager.h"
#include "Plant.h"
#include "ForestGeneratorLog.h"

// Sets default values
APlantVisualizer::APlantVisualizer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BranchMeshes"));
	InstancedStaticMeshComponent->SetMobility(EComponentMobility::Movable);
	InstancedStaticMeshComponent->SetCollisionProfileName("BlockAll");
	SetRootComponent(InstancedStaticMeshComponent);
}

// Called when the game starts or when spawned
void APlantVisualizer::BeginPlay()
{
	Super::BeginPlay();

	ModuleManager = NewObject<UBranchModuleManager>();
	if (!ModuleManager->Initialize(BranchModulePrototypes))
	{
		UE_LOG(LogForestGenerator, Error, TEXT("PlantVis: Couldn't initialize Module Manager."));
		return;
	}

	Time = FMath::Clamp(Time, 1, 10000);
	
	UStaticMesh* StaticMesh = InstancedStaticMeshComponent->GetStaticMesh();
	if (StaticMesh == nullptr)
	{
		UE_LOG(LogForestGenerator, Error, TEXT("PlantVis: Static Mesh not set."));
		return;
	}

	StaticMeshBoundingBox = StaticMesh->GetBoundingBox();

	if (PlantTypes == nullptr)
	{
		UE_LOG(LogForestGenerator, Error, TEXT("PlantVis: Plant Types not set."));
		return;
	}

	const TArray<FName> PlantNames = PlantTypes->GetRowNames();
	if (PlantNames.Num() == 0)
	{
		UE_LOG(LogForestGenerator, Error, TEXT("PlantVis: No Plant Types found in data table."));
		return;
	}

	const FName PlantName = PlantNames[0];
	UE_LOG(LogForestGenerator, Log, TEXT("Using plant: %s"), *PlantName.ToString());

	const FPlantSettings PlantSettings = *PlantTypes->FindRow<FPlantSettings>(PlantName, "");

	Plant = NewObject<UPlant>();
	if (!Plant->Initialize(ModuleManager, GetActorLocation(), PlantSettings))
	{
		UE_LOG(LogForestGenerator, Error, TEXT("PlantVis: Couldn't initialize Plant."));
		return;
	}

	GetWorldTimerManager().SetTimer(SimulationTimerHandle, this, &APlantVisualizer::Simulate, .1f, true);

	bInitialized = true;
}

void APlantVisualizer::Simulate()
{
	if (!bInitialized)
	{
		UE_LOG(LogForestGenerator, Log, TEXT("Plant not initialized, simulation not possible"));
		return;
	}

	if (--Time <= 0)
	{
		GetWorldTimerManager().ClearTimer(SimulationTimerHandle);
		// Plant->DrawDebug(GetWorld());
	}

	ModuleManager->CalculateLightExposures();
	Plant->Simulate();

	Render();

	if (Plant->GetState() == EPlantState::Dead)
	{
		GetWorldTimerManager().ClearTimer(SimulationTimerHandle);
	}
}

void APlantVisualizer::Render()
{
	InstancedStaticMeshComponent->ClearInstances();
	
	if (Plant->GetState() == EPlantState::Dead)
	{
		return;
	}
	
	TArray<FBranch> Branches = Plant->GetBranchTransforms();

	for (FBranch Branch : Branches)
	{
		FTransform BranchTransform = Branch.GetCylinderTransform(StaticMeshBoundingBox);
		UE_LOG(LogForestGenerator, Verbose, TEXT("Transform : %s"), *BranchTransform.ToString());
		InstancedStaticMeshComponent->AddInstance(BranchTransform);
	}
}
