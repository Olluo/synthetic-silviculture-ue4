// Ollie Nicholls, 2021

#pragma once

#include "CoreMinimal.h"

#include "Branch.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"

#include "Plant.generated.h"

class UBranchModule;
class UBranchModuleManager;

USTRUCT(BlueprintType)
struct FPlantSettings : public FTableRowBase
{
	GENERATED_BODY()

	/**
	* @brief The max age of the plant, once Pt >= PMax, VRootMax is linearly interpolated to zero,
	* hence the vigor allocated to the plant is reduced until all modules are shed and the plant dies.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0"))
	int32 PMax = 950;

	/**
	* @brief The maximum vigor a plant can have in the root to limit growth potential.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float VRootMax = 900.f;

	/**
	* @brief Growth potential in the paper. The growth rate of the plant.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float Gp = 0.12f;

	/**
	* @brief Lambda in the paper.
	* The ratio of limiting lateral buds leading to a plant developing a trunk.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ApicalControl = 0.87f;

	/**
	* @brief Lambda mature in the paper.
	* Same usage as ApicalControl except applied once the plant reaches maturity.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ApicalControlMature = 0.34f;

	/**
	* @brief D in the paper.
	* Where buds develop into flowers preventing further growth.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Determinacy = 0.93f;

	/**
	* @brief D mature in the paper.
	* Same usage as Determinacy except applied once the plant reaches maturity.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeterminacyMature = 0.55f;

	/**
	* @brief F age in the paper.
	* The flowering age of the plant. Once the plant reaches this age it will start producing flowers and in turn,
	* offspring.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0"))
	int32 FAge = 57;

	/**
	* @brief Alpha in the paper.
	* This is the angle (-1 <= alpha <= 1) of the effect of tropisms on the plant. 
	* Alpha = >0 -> Plant is more driven by phototropism (light)
	* Alpha = <0 -> Plant is more driven by gravitropism (gravity)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Alpha = 0.66f;

	/**
	* @brief W 2 in the paper.
	* This is the weighting (<1) of how much tropism effects the orientation angle of new modules
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float W2 = 0.14f;

	/**
	* @brief G1 in the paper.
	* How fast the tropism effect decreases with time.
	* Note: Negative values do something funky so best keep it positive!
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "-5.0", ClampMax = "5.0"))
	float G1 = 0.2f;

	/**
	* @brief Phi in the paper.
	* The thickening factor - default thickness of new branches.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float Phi = 1.41f;

	/**
	* @brief Beta in the paper.
	* The scaling coefficient of branch length.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float Beta = 1.29f;

	/**
	* @brief VMin in the paper.
	* The minimum vigor clamp, vigor values under this will cause the module/node to be shed.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float VMin = 0.5f;

	/**
	* @brief VMax in the paper.
	* The maximum vigor clamp. Vigor cannot be higher than this in modules.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float VMax = 2.f;

	/**
	* @brief LMax in the paper.
	* The maximum branch length possible
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float LMax = 50.f;

	/**
	* @brief This is the overall strength of the tropism in the module adaptation stage.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0"))
	float TropismStrength = 1.f;

	/**
	* @brief This is how straight the main branch grows
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForestGen", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Straightness = 1.f;
};

UENUM(BlueprintType)
enum class EPlantState : uint8
{
	Young,
	Mature,
	Dead
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class FORESTGENERATOR_API UPlant : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintSetter, Category = "ForestGen|Plant")
	bool Initialize(UBranchModuleManager* InModuleManager, const FVector& InPosition,
	                const FPlantSettings& InSettings);

	UFUNCTION(BlueprintGetter, Category = "ForestGen|Plant")
	EPlantState GetState() const;
	
	UFUNCTION(BlueprintCallable, Category = "ForestGen|Plant")
	void ShedModules(TArray<UBranchModule*> Modules);
	
	UFUNCTION(BlueprintCallable, Category = "ForestGen|Plant")
	void Simulate(const float TimeStep = 1.f);

	UFUNCTION(BlueprintCallable, Category = "ForestGen|Plant")
	void DrawDebug(const UWorld* WorldContext) const;

	TArray<FBranch> GetBranchTransforms() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	UBranchModuleManager* BranchModuleManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	FVector Position;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	UBranchModule* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	TArray<UBranchModule*> BranchModules;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	EPlantState State = EPlantState::Young;

	/**
	* @brief The physiological age of the plant.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	float PT = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForestGen|Plant")
	FPlantSettings Settings;

private:
	bool bInitialized = false;

	void CalculateVigor();
	void Grow(const float TimeStep) const;
	TArray<UBranchModule*> TopologicalSortModules() const;
};
