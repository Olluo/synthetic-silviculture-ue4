// Ollie Nicholls, 2021

#include "BranchNode.h"

#include "BranchSegment.h"
#include "DrawDebugHelpers.h"
#include "ForestGeneratorLog.h"


void UBranchNode::AddChildBranch(UBranchSegment* Child, const bool bIsChildModule)
{
	if (bIsChildModule)
	{
		if (Type == ENodeType::Terminal)
		{
			ChildrenBranches.Add(Child);
			Type = ENodeType::Connecting;

			UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: New child [%d] branch added."),
			       ID, Child->GetDestination()->GetID());
		}
		else
		{
			UE_LOG(LogForestGenerator, Verbose,
			       TEXT("Branch Node[%d]: Cannot add new module to this node as not terminal."), ID);
		}

		return;
	}

	if (Type != ENodeType::Connecting && ChildrenBranches.Num() < 5)
	{
		ChildrenBranches.Add(Child);

		if (Type != ENodeType::Root)
		{
			Type = ENodeType::Normal;
		}

		UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: New child [%d] branch added."),
		       ID, Child->GetDestination()->GetID());
	}
	else
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Branch Node[%d]: Already has max children, no new child added."), ID);
	}
}

void UBranchNode::SetParentBranch(UBranchSegment* InParent)
{
	Parent = InParent;

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: Parent [%d] branch added."),
	       ID, InParent->GetSource()->GetID());
}

void UBranchNode::Translate(const FVector& Translation)
{
	if (Translation == FVector::ZeroVector)
	{
		return;
	}

	Position += Translation;

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: Translating by (%s) to (%s)."),
	       ID, *Translation.ToString(), *Position.ToString());

	for (UBranchNode* Child : GetAvailableChildren(true))
	{
		Child->Translate(Translation);
	}
}

void UBranchNode::SetRoot()
{
	Type = ENodeType::Root;

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: Set to root."), ID);
}

void UBranchNode::IncreaseAge(const float DeltaAge)
{
	PhysiologicalAge += DeltaAge;

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: Aging by %f, age now: %f."), ID, DeltaAge,
	       PhysiologicalAge);

	for (UBranchNode* Child : GetAvailableChildren())
	{
		Child->IncreaseAge(DeltaAge);
	}
}

void UBranchNode::RecalculateDirection()
{
	if (Parent != nullptr)
	{
		const FVector ParentToChild = Position - GetParentPosition();

		Direction = ParentToChild.GetSafeNormal();

		UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Node[%d]: Direction set to %s."), ID, *Direction.ToString());
	}
}

void UBranchNode::SetID(const uint8 InID)
{
	ID = InID;
}

void UBranchNode::SetVigor(const float InVigor)
{
	Vigor = InVigor;
}

void UBranchNode::SetLightExposure(const float InLightExposure)
{
	LightExposure = InLightExposure;
}

void UBranchNode::SetDirection(const FRotator& Rotator)
{
	Direction = Rotator.RotateVector(Direction);
}

uint8 UBranchNode::GetID() const
{
	return ID;
}

FVector UBranchNode::GetParentPosition() const
{
	if (Parent == nullptr)
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Branch Node[%d]: Parent not set."), ID);
		return FVector::ZeroVector;
	}

	return Parent->GetSource()->GetPosition();
}

bool UBranchNode::IsParentBranchAvailable() const
{
	if (Parent == nullptr)
	{
		return false;
	}

	return Parent->IsAvailable();
}

float UBranchNode::GetParentBranchDiameter() const
{
	if (Parent == nullptr)
	{
		UE_LOG(LogForestGenerator, Warning, TEXT("Branch Node[%d]: Parent not set."), ID);
		return 0.f;
	}

	return Parent->GetDiameter();
}

UBranchSegment* UBranchNode::GetParentBranch() const
{
	return Parent;
}

const FVector& UBranchNode::GetPosition() const
{
	return Position;
}

const FVector& UBranchNode::GetDirection() const
{
	return Direction;
}

float UBranchNode::GetAge() const
{
	return PhysiologicalAge;
}

TArray<UBranchSegment*> UBranchNode::GetChildrenBranches() const
{
	return ChildrenBranches;
}

TArray<UBranchSegment*> UBranchNode::GetAvailableChildrenBranches() const
{
	TArray<UBranchSegment*> AvailableChildrenBranches;

	if (Type == ENodeType::Connecting)
	{
		AvailableChildrenBranches.Add(ChildrenBranches[0]);
	}

	for (UBranchSegment* ChildBranch : ChildrenBranches)
	{
		if (ChildBranch->IsAvailable())
		{
			AvailableChildrenBranches.Add(ChildBranch);
		}
	}

	return AvailableChildrenBranches;
}

TArray<UBranchNode*> UBranchNode::GetAvailableChildren(const bool bIncludeChildModule)
{
	TArray<UBranchNode*> AvailableChildren;

	if (bIncludeChildModule && Type == ENodeType::Connecting)
	{
		AvailableChildren.Add(ChildrenBranches[0]->GetDestination());
	}
	else
	{
		for (UBranchSegment* ChildBranch : ChildrenBranches)
		{
			if (ChildBranch->IsAvailable())
			{
				AvailableChildren.Add(ChildBranch->GetDestination());
			}
		}
	}

	return AvailableChildren;
}

float UBranchNode::GetParentBranchLength() const
{
	if (Parent != nullptr)
	{
		const FVector ParentPosition = GetParentPosition();
		return (ParentPosition - Position).Size();
	}

	return 0.f;
}

void UBranchNode::DrawDebug(const UWorld* WorldContext)
{
	if (WorldContext == nullptr)
	{
		return;
	}

	DrawDebugSphere(WorldContext, Position, 1.f, 8, FColor::Red, true, -1.f, 0, 0.1f);

	if (ChildrenBranches.Num() == 0)
	{
		return;
	}

	for (UBranchNode* Child : GetAvailableChildren())
	{
		DrawDebugCylinder(WorldContext, Position,
		                  Child->GetPosition(), Child->GetParentBranchDiameter() / 2.f, 8,
		                  FColor::Blue, true);
		Child->DrawDebug(WorldContext);
	}
}

TArray<FBranch> UBranchNode::GetBranchTransforms()
{
	TArray<FBranch> BranchTransforms;

	if (ChildrenBranches.Num() == 0)
	{
		return BranchTransforms;
	}

	for (UBranchNode* Child : GetAvailableChildren(true))
	{
		FBranch Branch{Position, Child->GetPosition(), Child->GetParentBranchDiameter()};
		BranchTransforms.Add(Branch);
		BranchTransforms.Append(Child->GetBranchTransforms());
	}

	return BranchTransforms;
}

void UBranchNode::Visit(TArray<UBranchNode*>& SortedNodes)
{
	if (SortMark == ENodeSortMark::Permanent)
	{
		return;
	}

	if (SortMark == ENodeSortMark::Temporary)
	{
		// This should be impossible so very bad here
		UE_LOG(LogForestGenerator, Fatal, TEXT("Branch Module: Branch module graph is not a DAG!"));
	}

	SortMark = ENodeSortMark::Temporary;

	for (UBranchNode* Child : GetAvailableChildren())
	{
		Child->Visit(SortedNodes);
	}

	SortMark = ENodeSortMark::Permanent;
	SortedNodes.Add(this);
}

void UBranchNode::ResetSortMark()
{
	SortMark = ENodeSortMark::None;
}

void UBranchNode::ResetToTerminal()
{
	ChildrenBranches.Reset();
	Type = ENodeType::Terminal;
}

TArray<UBranchNode*> UBranchNode::GetChildren()
{
	TArray<UBranchNode*> Children;

	for (UBranchSegment* ChildBranch : ChildrenBranches)
	{
		Children.Add(ChildBranch->GetDestination());
	}

	return Children;
}

bool UBranchNode::IsRoot() const
{
	return Type == ENodeType::Root;
}

bool UBranchNode::IsTerminal() const
{
	return Type == ENodeType::Terminal;
}

float UBranchNode::GetVigor() const
{
	return Vigor;
}

float UBranchNode::GetLightExposure() const
{
	return LightExposure;
}
