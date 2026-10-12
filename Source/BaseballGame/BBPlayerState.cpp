#include "BBPlayerState.h"
#include "Net/UnrealNetwork.h"

ABBPlayerState::ABBPlayerState()
	: CurrentTryCount(0)
	, MaxTryCount(3)
{
	bReplicates = true;
}

void ABBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABBPlayerState, CurrentTryCount);
	DOREPLIFETIME(ABBPlayerState, MaxTryCount);
}

FString ABBPlayerState::GetTryCountString() const
{
	return FString::Printf(TEXT("[%d / %d]"), CurrentTryCount, MaxTryCount);
}

void ABBPlayerState::AddTryCount()
{
	if (HasAuthority())
	{
		CurrentTryCount = FMath::Min(CurrentTryCount + 1, MaxTryCount);
	}
}

void ABBPlayerState::ResetTryCount()
{
	if (HasAuthority())
	{
		CurrentTryCount = 0;
	}
}
