// Ollie Nicholls, 2021
// TODO Branch module shedding if vigor is below VMin


#include "BranchModule.h"

#include "BranchModuleManager.h"
#include "DrawDebugHelpers.h"
#include "BranchNode.h"
#include "BranchSegment.h"
#include "ForestGeneratorLog.h"

FGraphDefinition UBranchModule::GetGraphDefinition_Implementation()
{
	return GraphDefinition;
}

void UBranchModule::Initialize(const FGraphDefinition& NewGraphDefinition, const FVector& InPosition,
                               UBranchModuleManager* InModuleManager, const FRotator InOrientation)
{
	ModuleManager = InModuleManager;

	TArray<FGraphEdge> Edges = NewGraphDefinition.Edges;

	if (Edges.Num() < 1)
	{
		UE_LOG(LogForestGenerator, Fatal,
		       TEXT("Branch Module[%d]: Invalid Definition: Graph must have at least 1 edge."), ID);
		return;
	}

	TSortedMap<int32, UBranchNode*> NodesMap;

	for (const FGraphEdge Edge : Edges)
	{
		const int32 SourceID = Edge.Source;
		const int32 DestinationID = Edge.Destination;

		if (SourceID == DestinationID)
		{
			UE_LOG(LogForestGenerator, Fatal,
			       TEXT("Branch Module[%d]: Invalid Definition: The Source and Destination ID are the same: [%d]."), ID,
			       SourceID);
			return;
		}

		UBranchNode* Source;
		UBranchNode* Destination;

		if (NodesMap.Find(SourceID) == nullptr)
		{
			Source = NewObject<UBranchNode>();
			Source->SetID(SourceID);
			NodesMap.Add(SourceID, Source);
		}
		else
		{
			Source = NodesMap.FindChecked(SourceID);
		}

		if (NodesMap.Find(DestinationID) == nullptr)
		{
			Destination = NewObject<UBranchNode>();
			Destination->SetID(DestinationID);
			NodesMap.Add(DestinationID, Destination);
		}
		else
		{
			Destination = NodesMap.FindChecked(DestinationID);
		}

		UBranchSegment* Branch = NewObject<UBranchSegment>();
		Branch->Initialize(Source, Destination);

		Source->AddChildBranch(Branch);
		Destination->SetParentBranch(Branch);

		// TODO Need to check that the edge being added doesn't exist already
		Graph.Branches.Add(Branch);

		UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Module[%d]: Edge added from Parent [%d] to Child [%d]"), ID,
		       SourceID, DestinationID);
	}

	// Reshuffle IDs so start at 0 and add to graph
	int32 NextID = 0;
	for (const TPair<int32, UBranchNode*> Pair : NodesMap)
	{
		UBranchNode* Node = Pair.Value;
		Node->SetID(NextID++);
		Graph.Nodes.Add(Node);
	}

	Graph.Root = Graph.Nodes[0];
	Graph.Root->SetRoot();
	Graph.Root->Translate(InPosition);

	// We now have an array of nodes all with their edges
	// Need to check graph is connected now
	// We will do this by doing a breadth first search of the graph and then checking that all nodes are discovered
	// Taken from https://en.wikipedia.org/wiki/Breadth-first_search
	// And the depth calculation is from https://stackoverflow.com/a/43524835
	TQueue<UBranchNode*> Queue;
	TArray<int32> Discovered;
	int32 Depth = 0;

	// Add the root to discovered and the queue
	UBranchNode* Node = Graph.Nodes[0];
	Discovered.Add(Node->GetID());
	Queue.Enqueue(Node);
	Queue.Enqueue(nullptr);

	while (!Queue.IsEmpty())
	{
		if (Queue.Dequeue(Node))
		{
			if (Node == nullptr)
			{
				Queue.Enqueue(nullptr);
				if (Queue.Peek() == nullptr || *Queue.Peek() == nullptr)
				{
					break;
				}

				Depth++;
				continue;
			}
			for (UBranchSegment* ChildBranch : Node->GetChildrenBranches())
			{
				ChildBranch->SetDepth(Depth);
				UBranchNode* Child = ChildBranch->GetDestination();
				if (Discovered.Find(Child->GetID()) == INDEX_NONE)
				{
					Discovered.Add(Child->GetID());
					Queue.Enqueue(Child);
				}
			}
		}
	}

	if (Discovered.Num() != Graph.Nodes.Num())
	{
		UE_LOG(LogForestGenerator, Fatal, TEXT("Branch Module[%d]: Invalid Definition: The graph is not connected."),
		       ID);
	}

	// We now have a valid graph!
	// Minus 1 here as the graph starts with the root and its children
	AgeMature = static_cast<float>(Depth) - 1;

	// Initialize the Branch Module with the root node and its children available
	for (UBranchSegment* ChildBranch : Graph.Root->GetChildrenBranches())
	{
		ChildBranch->MakeAvailable();
		Graph.AvailableBranches.Add(ChildBranch);
	}

	Graph.Root->SetDirection(InOrientation);

	SpawnChildNodes(Graph.Root, 1.f);

	CalculateBoundingSphere();
}

void UBranchModule::SetID(const int32 InID)
{
	this->ID = InID;
}

void UBranchModule::SetVigor(const float InVigor)
{
	UE_LOG(LogForestGenerator, Log, TEXT("Branch Module[%d]: vigor set to %f."), ID, InVigor);
	this->Vigor = InVigor;
}

void UBranchModule::ResetSortMark()
{
	SortMark = ESortMark::None;
}

void UBranchModule::IncreaseLightExposure(const float InLightExposure)
{
	LightExposure += InLightExposure;
}

int32 UBranchModule::GetID() const
{
	return ID;
}

float UBranchModule::GetLightExposure() const
{
	return LightExposure;
}

float UBranchModule::GetVigor() const
{
	return Vigor;
}

const TArray<UBranchModule*>& UBranchModule::GetChildren() const
{
	return Children;
}

void UBranchModule::DrawBoundingSphere(const UWorld* WorldContext) const
{
	if (WorldContext == nullptr)
	{
		return;
	}

	DrawDebugSphere(WorldContext, BoundingSphere.Center, BoundingSphere.W, 16, FColor::Green, true, -1.f, 0, 0.5f);
}

void UBranchModule::DrawDebug(const UWorld* WorldContext)
{
	if (WorldContext == nullptr)
	{
		return;
	}

	// TODO This should call all child module and DrawDebug them as well
	// Graph.Root->DrawDebug(WorldContext);
	DrawBoundingSphere(WorldContext);
}

void UBranchModule::Visit(TArray<UBranchModule*>& SortedNodes)
{
	if (SortMark == ESortMark::Permanent)
	{
		return;
	}

	if (SortMark == ESortMark::Temporary)
	{
		// This should be impossible so very bad here
		UE_LOG(LogForestGenerator, Fatal, TEXT("Plant: Plant graph is not a DAG!"));
	}

	SortMark = ESortMark::Temporary;

	for (UBranchModule* Child : Children)
	{
		Child->Visit(SortedNodes);
	}

	SortMark = ESortMark::Permanent;
	SortedNodes.Add(this);
}

UBranchNode* UBranchModule::GetRootNode()
{
	return Graph.Root;
}

UBranchModule* UBranchModule::AttachNewBranchModule(UBranchNode* ParentNode, const float ApicalControl,
                                                    const float Determinacy)
{
	const FRotator SpawnOrientation = ParentNode->GetDirection().ToOrientationRotator() -
		FVector::UpVector.ToOrientationRotator();
	UBranchModule* ChildModule = ModuleManager->GenerateBranchModule(ApicalControl, Determinacy,
	                                                                 ParentNode->GetPosition(), SpawnOrientation);

	UBranchSegment* Branch = NewObject<UBranchSegment>();
	UBranchNode* ChildRootNode = ChildModule->GetRootNode();

	Branch->Initialize(ParentNode, ChildRootNode);
	ParentNode->AddChildBranch(Branch, true);
	ChildRootNode->SetParentBranch(Branch);
	Graph.AvailableBranches.Add(Branch);

	Children.Add(ChildModule);
	return ChildModule;
}

void UBranchModule::CalculatePerNodeVigor(const float ApicalControl)
{
	// Sort the nodes into a topological order for a basipetal pass
	TArray<UBranchNode*> SortedNodes = TopologicalSortNodes();

	// Accumulate Qu into Qtotal at uroot
	for (UBranchNode* Node : SortedNodes)
	{
		float QU = Node->GetLightExposure();

		// Sum up all Qus
		for (const UBranchNode* ChildNode : Node->GetChildren())
		{
			QU += ChildNode->GetLightExposure();
		}

		Node->SetLightExposure(QU);
	}

	// Reverse the list as redistributing in an acropetal pass
	Algo::Reverse(SortedNodes);
	ensure(SortedNodes[0] == Graph.Root);

	const float QTotal = Graph.Root->GetLightExposure();
	Graph.Root->SetVigor(QTotal);

	// Redistribute Vu through plant
	for (UBranchNode* Node : SortedNodes)
	{
		const float VU = Node->GetVigor();

		TArray<UBranchNode*> NodeChildren = Node->GetChildren();

		if (NodeChildren.Num() == 1)
		{
			// If the module only has one child, the remaining vigor goes to them
			UBranchNode* Child = NodeChildren[0];
			Child->SetVigor(VU);
		}
		else if (NodeChildren.Num() > 1)
		{
			UBranchNode* MainChild = nullptr;

			for (UBranchNode* Child : NodeChildren)
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

			NodeChildren.Remove(MainChild);

			const float QUM = MainChild->GetLightExposure();
			const float QUL = Node->GetLightExposure() - QUM;
			const float Lambda = ApicalControl;

			// Eq 2
			float VUM = 0.f;
			
			if (QUM != 0.f)
			{
				VUM = VU * (Lambda * QUM) / (Lambda * QUM + (1 - Lambda) * QUL);
			}
			const float VUL = VU - VUM;

			MainChild->SetVigor(VUM);

			for (UBranchNode* Child : NodeChildren)
			{
				Child->SetVigor(VUL);
			}
		}
	}
}

void UBranchModule::Shed()
{
	bShed = true;
}

bool UBranchModule::IsShed()
{
	return bShed;
}

void UBranchModule::Grow(const float DT, const float VMin, const float VMax, const float GP,
                         const float Phi, const float Beta, const float LMax, const float G1,
                         const float Alpha, const FVector& GDir, const float TropismStrength,
                         const float Straightness, const float ApicalControl, const float Determinacy,
                         const bool bCanSpawnChildren)
{
	UE_LOG(LogForestGenerator, Log, TEXT("Branch Module[%d]: ========== Main Grow Loop =========="), ID);
	
	if (Vigor < VMin)
	{
		UE_LOG(LogForestGenerator, Log, TEXT("Branch Module[%d]: vigor too low = %f"), ID, Vigor);
		return;
	}

	// Shuffles the array so not the same order each time
	if (Children.Num() > 0)
	{
		// const int32 LastIndex = Children.Num() - 1;
		// for (int32 i = 0; i <= LastIndex; ++i)
		// {
		// 	const int32 Index = FMath::RandRange(i, LastIndex);
		// 	if (i != Index)
		// 	{
		// 		Children.Swap(i, Index);
		// 	}
		// }

		TArray<UBranchModule*> NewChildren;

		for (UBranchModule* Child : Children)
		{
			if (!Child->IsShed())
			{
				NewChildren.Add(Child);
				Child->Grow(DT, VMin, VMax, GP, Phi, Beta, LMax, G1, Alpha, GDir, TropismStrength, Straightness,
				            ApicalControl,
				            Determinacy, bCanSpawnChildren);
			}
			else
			{
				UBranchSegment* ConnectingBranch = Child->GetRootNode()->GetParentBranch();
				Graph.AvailableBranches.Remove(ConnectingBranch);
				ConnectingBranch->GetSource()->ResetToTerminal();
			}
		}
		Children = NewChildren;
	}

	// Smoothly interpolated sigmoid function
	auto S = [](const float X) { return 3 * FMath::Square(X) - 2 * FMath::Pow(X, 3.f); };

	// Clamp Vigor to VMax
	Vigor = FMath::Min(Vigor, VMax);

	// ========== Equation 5 ==========
	// Calculate growth rate (how quickly a module is developed)
	const float GrowthRate = S((Vigor - VMin) / (VMax - VMin)) * GP;

	// ========== Equation 6 ==========
	// Calculate the DeltaAge using the rate of change of physiological age function (dau/dt = Y(u))
	const float DeltaAge = GrowthRate * static_cast<float>(DT);

	// Increase physiological age of branch module and potentially grow graph
	IncreaseAge(DeltaAge, Straightness);

	if (PhysiologicalAge > AgeMature && bCanSpawnChildren)
	{
		TArray<UBranchNode*> TerminalNodes = GetTerminalNodes();
		if (TerminalNodes.Num() != 0)
		{
			const float TerminalLightExposure = LightExposure / static_cast<float>(TerminalNodes.Num());
			for (UBranchNode* TerminalNode : TerminalNodes)
			{
				TerminalNode->SetLightExposure(TerminalLightExposure);
			}

			CalculatePerNodeVigor(ApicalControl);

			for (UBranchNode* TerminalNode : TerminalNodes)
			{
				UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Module[%d]: Terminal vigor = %f"), ID,
				       TerminalNode->GetVigor());
				if (TerminalNode->GetVigor() > VMin && TerminalNode->GetPosition().Z > BoundingSphere.Center.Z)
				{
					UBranchModule* Child = AttachNewBranchModule(TerminalNode, ApicalControl,
					                                             Vigor * Determinacy / VMax);
					// Child->Grow(DT, VMin, VMax, GP, Phi, Beta, LMax, G1, Alpha, GDir, TropismStrength, Straightness,
					//             ApicalControl, Determinacy, bCanSpawnChildren);
				}
			}
		}
	}

	TArray<UBranchSegment*> Branches(Graph.AvailableBranches);
	Algo::Reverse(Branches);

	for (UBranchSegment* Branch : Branches)
	{
		UBranchNode* Node = Branch->GetDestination();

		// In the paper, the age of a branch is defined by = module age - oldest node in the segment age
		// which doesn't make sense as the beginning branch ages will always be zero, and all other branches will
		// never age as the difference between the max age and Au will always be the same...
		// Therefore, after talking with Jian, I think the branch age should be the age of the destination node as
		// that's when the branch was added
		const float BranchAge = Node->GetAge();

		// ========== Equation 8 ==========
		TArray<UBranchSegment*> ChildrenBranches = Node->GetAvailableChildrenBranches();

		if (ChildrenBranches.Num() != 0)
		{
			// If the branch has children, set the diameter to sqrt(sum of (children diameter^2) of all children)
			float SummedChildDiameters = 0.f;

			for (const UBranchSegment* ChildrenBranch : ChildrenBranches)
			{
				SummedChildDiameters += FMath::Square(ChildrenBranch->GetDiameter());
			}

			Branch->SetDiameter(FMath::Sqrt(SummedChildDiameters));
		}
		else
		{
			// If the branch has no children the diameter is the thickening factor
			Branch->SetDiameter(Phi);
		}

		// ========== Equation 9 ==========
		const float NewBranchLength = FMath::Min(LMax, Beta * BranchAge);
		const float BranchChange = NewBranchLength - Node->GetParentBranchLength();
		FVector Growth = BranchChange * Node->GetDirection();

		Node->Translate(Growth);

		// Section 5.3.1 - Module Adaptation
		// This is where the tropism is used to effect positions of the nodes
		const float G2 = Alpha * -1.f * TropismStrength;
		FVector TropismOffset;
		const float Denominator = BranchAge + G1;

		if (Denominator == 0.f)
		{
			TropismOffset = FVector::ZeroVector;
		}
		else
		{
			TropismOffset = (G1 * GDir * G2) / Denominator;
		}

		if ((Node->GetPosition() + TropismOffset).Z < 0.f)
		{
			TropismOffset.Z = 0.1f - Node->GetPosition().Z;
		}

		if (BranchAge < 2.f)
		{
			TropismOffset = FVector::ZeroVector;
		}

		Node->Translate(TropismOffset);
		Node->RecalculateDirection();
	}


	CalculateBoundingSphere();
}

TArray<FBranch> UBranchModule::GetBranchTransforms() const
{
	return Graph.Root->GetBranchTransforms();
}

void UBranchModule::Orientate(const TArray<FSphere> Neighbors, const FRotator& InitialOrientation)
{
	/*
	TODO Module orientation when attached to parent module
	
	Section 5.2.3
	
	Natural branches tend to avoid collision naturally and exhibit tendencies to grow in certain directions
	Therefore, this needs to be simulated here
	
	Several optimisation steps of iterative gradient descent to find optimal orientation for each new module
	Orientation is represented using three Euler angles
	Default starting orientation is the parent's orientation
	
	use equation 3, 4, 12, and 13
	
	equation 3 is a distribution function = w1 * collisions(u) + w2 * tropism(u)
	w2 is the weighting of tropism and is defined as an input parameter
	w1 = 1 - w2
	collisions(u) we already defined (so may need to change how we use this)
	tropism(u) = || cos(TropismAngle) - cos(Orientation)||
	
	For eq 4, it calculates the distance (as defined by cos) from the predicted orientation to the
	tropism orientation (previous orientation plus the tropism offset to consider gravity and phototropism).
	This will offer better chance for a selected new orientation candidate if it follows the gravity influence.
	The neighbors here are all the branch modules of the current plant as it is calculating self-intersections
	*/

	Orientation = InitialOrientation;
}

void UBranchModule::CalculateLightExposure(const TArray<FSphere>& IntersectingNeighbors)
{
	UE_LOG(LogForestGenerator, Log, TEXT("Branch Module[%d]: Calculating light exposure"), ID);

	float Collisions = CalculateCollisions(BoundingSphere, IntersectingNeighbors);

	// This isn't mentioned in the paper but it gets a ratio of how much of the module is intersected
	// otherwise the LightExposure was often 0.
	// This value can be above 1 if lots of intersections.
	Collisions /= BoundingSphere.GetVolume();

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Module[%d]: Percent of Branch Module that collides: %f"), ID,
	       Collisions);

	LightExposure = FMath::Clamp(FMath::Exp(-Collisions), 0.f, 1.f);
	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Module[%d]: Calculated light exposure Qu: %f"), ID, LightExposure);
}

const FSphere& UBranchModule::GetBoundingSphere() const
{
	return BoundingSphere;
}

float UBranchModule::GetAge()
{
	return PhysiologicalAge;
}

float UBranchModule::CalculateCollisions(const FSphere& Sphere, const TArray<FSphere>& IntersectingNeighbors)
{
	float Collisions = 0.f;

	for (FSphere Neighbor : IntersectingNeighbors)
	{
		const float IntersectingVolume = CalculateIntersectingVolume(Sphere, Neighbor);
		if (IntersectingVolume < 0.f)
		{
			// This means the neighbor is fully inside the sphere
			Collisions += Neighbor.GetVolume();
		}
		else
		{
			Collisions += IntersectingVolume;
		}
	}

	return FMath::Max(Collisions, 0.f);
}

float UBranchModule::CalculateIntersectingVolume(const FSphere& Sphere, const FSphere& Neighbor)
{
	// See https://mathworld.wolfram.com/Sphere-SphereIntersection.html
	const float d = FVector::Distance(Sphere.Center, Neighbor.Center);
	const float R = Sphere.W;
	const float r = Neighbor.W;

	// Ensure the spheres are intersecting
	ensure(d < R + r);

	// PI * (R + r - d)^2 * (d^2 + 2dr - 3r^2 + 2dR + 6rR - 3R^2) / 12d
	return (PI *
			FMath::Square(R + r - d) *
			(FMath::Square(d) + 2.f * d * r - 3 * FMath::Square(r) + 2.f * d * R + 6.f * r * R - 3.f *
				FMath::Square(R))) /
		(12.f * d);
}

void UBranchModule::CalculateBoundingSphere()
{
	// This function is "good enough".
	// The paper states that the bounding sphere is "computed using center points of their [BranchModule's] geometry".
	// So in this function we calculate the midpoint by averaging all positions,
	// then we calculate the max distance of all the positions with the midpoint and use this as the radius.

	TArray<FVector> NodePositions;
	FVector Midpoint{0.f};

	for (UBranchNode* Node : GetAvailableNodes())
	{
		FVector NodePosition = Node->GetPosition();
		NodePositions.Add(NodePosition);
		Midpoint += NodePosition;
	}

	Midpoint /= NodePositions.Num();

	float RadiusSquared = 0.f;

	for (FVector NodePosition : NodePositions)
	{
		// Use DistSquared here as faster and just Sqrt at the end
		// Note also using Abs as we only want a positive value
		RadiusSquared = FMath::Max(RadiusSquared, FMath::Abs(FVector::DistSquared(Midpoint, NodePosition)));
	}

	// When the branch module is spawned it can have 0 radius so set it to at least something so it doesn't cause issues
	RadiusSquared = (RadiusSquared == 0.f) ? 10.f : RadiusSquared;

	BoundingSphere = FSphere(Midpoint, FMath::Sqrt(RadiusSquared));
}

void UBranchModule::SpawnChildNodes(UBranchNode* Parent, const float Straightness) const
{
	const FVector ParentPosition = Parent->GetPosition();
	TArray<UBranchNode*> ChildrenNodes = Parent->GetChildren();
	Algo::Reverse(ChildrenNodes);
	UBranchNode* Child;

	const FRotator ParentRotation = Parent->GetDirection().ToOrientationRotator() -
		FVector::UpVector.ToOrientationRotator();
	FVector Position = FVector::UpVector;

	// Children.Num() can be max of 5

	// If there is an odd number of children one child is always straight up
	if ((ChildrenNodes.Num() + 1) % 2 == 0)
	{
		const FRotator Rotator = FRotator{
			FMath::RandRange(-10.f, 10.f),
			0.f,
			FMath::RandRange(-10.f, 10.f)
		} * (1.f - Straightness);
		Child = ChildrenNodes.Pop();
		Position = Rotator.RotateVector(Position);
		Position = ParentRotation.RotateVector(Position);
		Child->Translate(ParentPosition + Position);
		Child->RecalculateDirection();
	}

	if (ChildrenNodes.Num() == 0)
	{
		return;
	}

	// This is used to make sure not all the branches come out the same direction for each SpawnChildren call
	const FRotator Rotator = FRotator{0.f, FMath::RandRange(0.f, 360.f), 0.f};


	if (ChildrenNodes.Num() == 4)
	{
		Child = ChildrenNodes.Pop();
		Position = FVector{1.f, 0.f, 1.f};
		Position = Rotator.RotateVector(Position);
		Position = ParentRotation.RotateVector(Position);
		Child->Translate(ParentPosition + Position);
		Child->RecalculateDirection();

		Child = ChildrenNodes.Pop();
		Position = FVector{-1.f, 0.f, 1.f};
		Position = Rotator.RotateVector(Position);
		Position = ParentRotation.RotateVector(Position);
		Child->Translate(ParentPosition + Position);
		Child->RecalculateDirection();
	}

	Child = ChildrenNodes.Pop();
	Position = FVector{0.f, 1.f, 1.f};
	Position = Rotator.RotateVector(Position);
	Position = ParentRotation.RotateVector(Position);
	Child->Translate(ParentPosition + Position);
	Child->RecalculateDirection();

	Child = ChildrenNodes.Pop();
	Position = FVector{0.f, -1.f, 1.f};
	Position = Rotator.RotateVector(Position);
	Position = ParentRotation.RotateVector(Position);
	Child->Translate(ParentPosition + Position);
	Child->RecalculateDirection();
}

void UBranchModule::GrowGraph(const float Straightness)
{
	TArray<UBranchNode*> NewParents;
	for (UBranchSegment* Branch : Graph.Branches)
	{
		if (!Branch->IsAvailable() && Branch->GetDepth() <= static_cast<int>(PhysiologicalAge))
		{
			Branch->MakeAvailable();
			Graph.AvailableBranches.Add(Branch);
			Branch->GetDestination()->IncreaseAge(PhysiologicalAge - static_cast<float>(Branch->GetDepth()));

			NewParents.AddUnique(Branch->GetSource());
		}
	}

	for (UBranchNode* NewParent : NewParents)
	{
		SpawnChildNodes(NewParent, Straightness);
	}
}

void UBranchModule::IncreaseAge(const float DeltaAge, const float Straightness)
{
	PhysiologicalAge += DeltaAge;

	UE_LOG(LogForestGenerator, Log, TEXT("Branch Module[%d]: Aging by %f, age now: %f"),
	       ID, DeltaAge, PhysiologicalAge);

	Graph.Root->IncreaseAge(DeltaAge);

	GrowGraph(Straightness);
}

TArray<UBranchNode*> UBranchModule::GetAvailableNodes()
{
	TArray<UBranchNode*> AvailableNodes;

	for (UBranchNode* Node : Graph.Nodes)
	{
		if (Node->IsRoot())
		{
			AvailableNodes.Add(Node);
		}
		else if (Node->IsParentBranchAvailable())
		{
			AvailableNodes.Add(Node);
		}
	}

	return AvailableNodes;
}

TArray<UBranchNode*> UBranchModule::GetTerminalNodes()
{
	TArray<UBranchNode*> TerminalNodes;

	TArray<UBranchNode*> SortedNodes = TopologicalSortNodes();

	for (UBranchNode* SortedNode: SortedNodes)
	{
		if (SortedNode->IsParentBranchAvailable() && SortedNode->IsTerminal())
		{
			TerminalNodes.Add(SortedNode);
		}
	}

	return TerminalNodes;
}

TArray<UBranchNode*> UBranchModule::TopologicalSortNodes() const
{
	TArray<UBranchNode*> SortedNodes;

	Graph.Root->Visit(SortedNodes);

	for (UBranchNode* SortedNode : SortedNodes)
	{
		SortedNode->ResetSortMark();
	}

	return SortedNodes;
}