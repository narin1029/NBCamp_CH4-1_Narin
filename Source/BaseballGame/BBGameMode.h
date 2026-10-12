#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BBGameMode.generated.h"

class ABBPlayerController;
class ABBPlayerState;

/** 서버에서만 존재. 정답 생성 / 입력 검사 / 판정 / 승리·무승부 / 리셋을 담당한다. */
UCLASS()
class BASEBALLGAME_API ABBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABBGameMode();

	virtual void BeginPlay() override;

	/** 컨트롤러가 서버 RPC로 보낸 채팅을 처리한다. "/123" 처럼 '/'로 시작하면 숫자 야구 입력. */
	void HandlePlayerMessage(ABBPlayerController* Sender, const FString& Message);

private:
	// --- 필수 함수 ---
	FString GenerateRandomNumbers() const;                           // 1~9 중복 없는 3자리
	bool IsValidInput(const FString& Input, FString& OutError) const; // 깐깐한 입력 검사
	FString CheckAnswer(const FString& Input) const;                 // "1S1B", "OUT" ...
	void ResetGame();                                                // 전체 초기화 + 새 정답

	void CheckGameEnd(ABBPlayerState* Guesser, const FString& Result);
	void BroadcastMessage(const FString& Message) const;
	void BroadcastAnnouncement(const FString& Text) const;

private:
	FString SecretNumber;
	bool bGameOver = false;

	FTimerHandle ResetTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Baseball")
	float ResetDelay = 5.f;
};
