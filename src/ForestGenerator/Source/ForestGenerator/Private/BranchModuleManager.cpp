// Ollie Nicholls, 2021


#include "BranchModuleManager.h"

#include "BranchModule.h"
#include "ForestGeneratorLog.h"


bool UBranchModuleManager::Initialize(const TArray<TSubclassOf<UBranchModule>>& BranchModulePrototypes)
{
	if (bInitialized)
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Module Manager: Cannot initialize as already initialized."));
		return false;
	}

	if (BranchModulePrototypes.Num() == 0)
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Module Manager: No prototypes set to initialize with."));
		return false;
	}

	for (TSubclassOf<UBranchModule> BranchModulePrototype : BranchModulePrototypes)
	{
		// TODO when adding a module, decide its morphospace based on average children number and number of nodes
		UBranchModule* Prototype = NewObject<UBranchModule>(this, BranchModulePrototype);
		GraphPrototypes.Add(Prototype->GetGraphDefinition());
	}

	bInitialized = true;
	return true;
}

UBranchModule* UBranchModuleManager::GenerateBranchModule(int32 ApicalControl, int32 Determinacy,
                                                          const FVector& InPosition, const FRotator& InitialOrientation)
{
	// TODO Use parameters. For now just return a BranchModule object that uses the first graph prototype

	const FGraphDefinition SelectedGraph = GraphPrototypes[0];

	UBranchModule* NewModule = NewObject<UBranchModule>();
	NewModule->SetID(NextID);
	NewModule->Initialize(SelectedGraph, InPosition, this, InitialOrientation);
	// NewModule->Orientate(GetNeighborBoundingSpheres(NewModule), InitialOrientation);

	// Add this to be tracked
	BranchModules.Add(NewModule);

	NextID++;

	return NewModule;
}

void UBranchModuleManager::CalculateLightExposures()
{
	for (UBranchModule* BranchModule : BranchModules)
	{
		BranchModule->CalculateLightExposure(GetNeighborBoundingSpheres(BranchModule));
	}
}

void UBranchModuleManager::RemoveModule(UBranchModule* BranchModule)
{
	BranchModules.Remove(BranchModule);
}

int UBranchModuleManager::GetNumberOfModules() const
{
	return BranchModules.Num();
}

TArray<FSphere> UBranchModuleManager::GetNeighborBoundingSpheres(UBranchModule* QueryModule) const
{
	TArray<FSphere> AllBoundingSpheres;
	TArray<FSphere> NeighborBoundingSpheres;
	const FSphere QueryModuleBoundingSphere = QueryModule->GetBoundingSphere();

	for (UBranchModule* BranchModule : BranchModules)
	{
		if (BranchModule != QueryModule)
		{
			AllBoundingSpheres.Add(BranchModule->GetBoundingSphere());
		}
	}

	for (FSphere BoundingSphere : AllBoundingSpheres)
	{
		if (BoundingSphere.Intersects(QueryModuleBoundingSphere))
		{
			NeighborBoundingSpheres.Add(BoundingSphere);
		}
	}

	UE_LOG(LogForestGenerator, Verbose, TEXT("Module Manager: Neighbors: %d"), NeighborBoundingSpheres.Num());

	return NeighborBoundingSpheres;
}
