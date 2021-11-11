// Ollie Nicholls, 2021
// TODO Plant dying once there are no more modules left

#include "Plant.h"

#include "BranchModule.h"
#include "BranchModuleManager.h"
#include "ForestGeneratorLog.h"


bool UPlant::Initialize(UBranchModuleManager* InModuleManager, const FVector& InPosition,
                        const FPlantSettings& InSettings)
{
	if (bInitialized)
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Plant: Cannot initialize as already initialized."));
		return false;
	}

	BranchModuleManager = InModuleManager;
	Position = InPosition;

	Settings.PMax = FMath::Max(InSettings.PMax, 0);
	Settings.VRootMax = FMath::Max(InSettings.VRootMax, 0.f);
	Settings.Gp = FMath::Max(InSettings.Gp, 0.f);
	Settings.ApicalControl = FMath::Clamp(InSettings.ApicalControl, 0.f, 1.f);
	Settings.ApicalControlMature = FMath::Clamp(InSettings.ApicalControlMature, 0.f, 1.f);
	Settings.Determinacy = FMath::Clamp(InSettings.Determinacy, 0.f, 1.f);
	Settings.DeterminacyMature = FMath::Clamp(InSettings.DeterminacyMature, 0.f, 1.f);
	Settings.FAge = FMath::Max(InSettings.FAge, 0);
	Settings.Alpha = FMath::Clamp(InSettings.Alpha, -1.f, 1.f);
	Settings.W2 = FMath::Clamp(InSettings.W2, 0.f, 1.f);
	Settings.G1 = FMath::Clamp(InSettings.G1, -5.f, 5.f);
	Settings.Phi = FMath::Max(InSettings.Phi, 0.f);
	Settings.Beta = FMath::Max(InSettings.Beta, 0.f);
	Settings.VMin = FMath::Max(InSettings.VMin, 0.f);
	Settings.VMax = FMath::Max(InSettings.VMax, 0.f);
	Settings.LMax = FMath::Max(InSettings.LMax, 0.f);
	Settings.TropismStrength = FMath::Max(InSettings.TropismStrength, 0.f);
	Settings.Straightness = FMath::Clamp(InSettings.Straightness, 0.f, 1.f);

	if (Settings.VMax <= Settings.VMin)
	{
		Settings.VMax = Settings.VMin + .1f;
	}

	// Add the root module
	UBranchModule* BranchModule0 = BranchModuleManager->GenerateBranchModule(
		Settings.ApicalControl, Settings.Determinacy, Position);
	Root = BranchModule0;

	bInitialized = true;
	return true;
}

EPlantState UPlant::GetState() const
{
	return State;
}

void UPlant::ShedModules(TArray<UBranchModule*> Modules)
{
	for (UBranchModule* Module : Modules)
	{
		if (Module->GetAge() > 2.f && Module->GetVigor() < Settings.VMin)
		{
			BranchModuleManager->RemoveModule(Module);
			Module->Shed();
			if (Root == Module)
			{
				Root = nullptr;
				State = EPlantState::Dead;
			}
		}
	}
}

void UPlant::Simulate(const float TimeStep)
{
	// Modules should have light exposures pre calculated before this
	CalculateVigor();
	Grow(TimeStep);
	PT += TimeStep;
}

void UPlant::DrawDebug(const UWorld* WorldContext) const
{
	Root->DrawDebug(WorldContext);
}

TArray<FBranch> UPlant::GetBranchTransforms() const
{
	return Root->GetBranchTransforms();
}

void UPlant::CalculateVigor()
{
	// Sort the nodes into a topological order for a basipetal pass
	TArray<UBranchModule*> SortedModules = TopologicalSortModules();

	// Accumulate Qu into Qtotal at uroot
	for (UBranchModule* Module : SortedModules)
	{
		float LightExposure = 0.f;

		// Sum up all Qus
		for (UBranchModule* Child : Module->GetChildren())
		{
			LightExposure += Child->GetLightExposure();
		}

		Module->IncreaseLightExposure(LightExposure);
	}

	const float QTotal = Root->GetLightExposure();
	UE_LOG(LogForestGenerator, Log, TEXT("Plant: Qtotal = %f."), QTotal);

	// To simulate gradual plant senescence, reduce the vigor once plant reaches max age
	const float PMax = static_cast<float>(Settings.PMax);
	if (PT >= PMax)
	{
		// Linearly interpolate Vrootmax to zero
		const float LerpAlpha = (PT - PMax) / PMax;
		// Note could use Lerp here instead of stable if this is slowing system down
		Settings.VRootMax = FMath::LerpStable(0.f, Settings.VRootMax, LerpAlpha);
	}

	// Vu is always clamped to Vrootmax as plants can only store so much energy
	float VU = FMath::Min(QTotal, Settings.VRootMax);

	// Reverse the list as redistributing in an acropetal pass
	Algo::Reverse(SortedModules);
	ensure(SortedModules[0] == Root);
	Root->SetVigor(VU);

	// Redistribute Vu through plant
	for (UBranchModule* Module : SortedModules)
	{
		VU = Module->GetVigor();

		TArray<UBranchModule*> Children = Module->GetChildren();

		if (Children.Num() == 1)
		{
			// If the module only has one child, the remaining vigor goes to them
			UBranchModule* Child = Children[0];
			Child->SetVigor(VU);
		}
		else if (Children.Num() > 1)
		{
			UBranchModule* MainChild = nullptr;

			for (UBranchModule* Child : Children)
			{
				if (MainChild != nullptr)
				{
					MainChild = (Child->GetID() < MainChild->GetID()) ? Child : MainChild;
				}
				else
				{
					MainChild = Child;
				}
			}

			Children.Remove(MainChild);

			const float QUM = MainChild->GetLightExposure();
			const float QUL = Module->GetLightExposure() - QUM;
			const float Lambda = Settings.ApicalControl;

			// Eq 2
			float VUM = 0.f;
			
			if (QUM != 0.f)
			{
				VUM = VU * (Lambda * QUM) / (Lambda * QUM + (1 - Lambda) * QUL);
			}
			
			const float VUL = VU - VUM;

			MainChild->SetVigor(VUM);

			for (UBranchModule* Child : Children)
			{
				Child->SetVigor(VUL);
			}
		}
	}

	ShedModules(SortedModules);
}

void UPlant::Grow(const float TimeStep) const
{
	if (Root != nullptr)
	{
		const bool bCanSpawnChildren = BranchModuleManager->GetNumberOfModules() < 100;
		Root->Grow(TimeStep, Settings.VMin, Settings.VMax, Settings.Gp, Settings.Phi, Settings.Beta, Settings.LMax,
		           Settings.G1, Settings.Alpha, FVector::DownVector, Settings.TropismStrength, Settings.Straightness,
		           Settings.ApicalControl, Settings.Determinacy, bCanSpawnChildren);
	}
}


TArray<UBranchModule*> UPlant::TopologicalSortModules() const
{
	TArray<UBranchModule*> SortedNodes;

	Root->Visit(SortedNodes);

	for (UBranchModule* SortedNode : SortedNodes)
	{
		SortedNode->ResetSortMark();
	}

	return SortedNodes;
}
