#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BBPlayerState.generated.h"

/** 플레이어별 시도 횟수를 관리하고 모든 클라이언트에 복제한다. */
UCLASS()
class BASEBALLGAME_API ABBPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABBPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Getter
	UFUNCTION(BlueprintPure)
	int32 GetCurrentTryCount() const { return CurrentTryCount; }

	UFUNCTION(BlueprintPure)
	int32 GetMaxTryCount() const { return MaxTryCount; }

	/** "[현재 시도 횟수 / 최대 시도 횟수]" 형태 문자열 */
	FString GetTryCountString() const;

	bool HasChancesLeft() const { return CurrentTryCount < MaxTryCount; }

	// 서버에서만 호출
	void AddTryCount();
	void ResetTryCount();

protected:
	UPROPERTY(Replicated)
	int32 CurrentTryCount;

	UPROPERTY(Replicated)
	int32 MaxTryCount;
};
