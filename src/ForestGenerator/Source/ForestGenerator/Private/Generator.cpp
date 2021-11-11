// Ollie Nicholls, 2021


#include "Generator.h"

#include "Components/BillboardComponent.h"

#include "Manager.h"
#include "ForestGeneratorLog.h"
// TODO Generator logging

// Sets default values
AGenerator::AGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard Component"));
	BillboardComponent->SetHiddenInGame(false, true);
	RootComponent = BillboardComponent;

	ManagerComponent = CreateDefaultSubobject<UManager>(TEXT("Manager Component"));
	AddOwnedComponent(ManagerComponent);
}

// Called when the game starts or when spawned
void AGenerator::BeginPlay()
{
	Super::BeginPlay();
	ManagerComponent->SetWorldContext(GetWorld());
	Simulate();
	ManagerComponent->Render();
}

// Called every frame
void AGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenerator::Simulate()
{
	NumberOfPlants = FMath::Clamp(NumberOfPlants, 1, 100);
	MaxNumberOfPlants = FMath::Clamp(MaxNumberOfPlants, 1, 100);
	Time = FMath::Clamp(Time, 1, 10000);
	TimeStep = FMath::Clamp(TimeStep, 0.f, 10000.f);
	Temperature = FMath::Clamp(Temperature, -10.f, 33.f);
	Precipitation = FMath::Clamp(Precipitation, 10.f, 4300.f);

	ManagerComponent->Simulate(FSimulationSettings{
		                           NumberOfPlants, MaxNumberOfPlants, Time, TimeStep, Temperature, Precipitation
	                           }, BranchModulePrototypes, PlantTypes);
}
