// Ollie Nicholls, 2021

#include "BranchSegment.h"

#include "BranchNode.h"
#include "ForestGeneratorLog.h"


void UBranchSegment::Initialize(UBranchNode* InSource, UBranchNode* InDestination)
{
	Source = InSource;
	Destination = InDestination;

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Segment[%s]: Initialising."), *ToString());
}

void UBranchSegment::SetDiameter(const float InDiameter)
{
	Diameter = InDiameter;

	UE_LOG(LogForestGenerator, Verbose, TEXT("Branch Segment[%s]: Diameter set to %f."), *ToString(), Diameter);
}

void UBranchSegment::MakeAvailable()
{
	bAvailable = true;
}

void UBranchSegment::SetDepth(const int32 InDepth)
{
	Depth = InDepth;
}

UBranchNode* UBranchSegment::GetSource() const
{
	return Source;
}

UBranchNode* UBranchSegment::GetDestination() const
{
	return Destination;
}

float UBranchSegment::GetDiameter() const
{
	return Diameter;
}

bool UBranchSegment::IsAvailable() const
{
	return bAvailable;
}

int32 UBranchSegment::GetDepth() const
{
	return Depth;
}

FString UBranchSegment::ToString() const
{
	return FString::Printf(TEXT("%d, %d"), Source->GetID(), Destination->GetID());
}
