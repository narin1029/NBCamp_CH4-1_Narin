#include "BBGameMode.h"
#include "BBPlayerController.h"
#include "BBPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABBGameMode::ABBGameMode()
{
	PlayerControllerClass = ABBPlayerController::StaticClass();
	PlayerStateClass = ABBPlayerState::StaticClass();
	DefaultPawnClass = nullptr; // 채팅 게임이라 Pawn 불필요
}

void ABBGameMode::BeginPlay()
{
	Super::BeginPlay();

	SecretNumber = GenerateRandomNumbers();
	UE_LOG(LogTemp, Warning, TEXT("[Baseball] 정답: %s"), *SecretNumber);
}

FString ABBGameMode::GenerateRandomNumbers() const
{
	TArray<int32> Pool = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	FString Result;

	// 부분 셔플: 앞에서 3개만 뽑는다 (중복 불가)
	for (int32 i = 0; i < 3; ++i)
	{
		const int32 Pick = FMath::RandRange(i, Pool.Num() - 1);
		Pool.Swap(i, Pick);
		Result.AppendInt(Pool[i]);
	}
	return Result;
}

bool ABBGameMode::IsValidInput(const FString& Input, FString& OutError) const
{
	if (Input.Len() != 3)
	{
		OutError = TEXT("3자리 숫자를 입력해주세요.");
		return false;
	}

	TSet<TCHAR> Seen;
	for (const TCHAR Ch : Input)
	{
		if (Ch < TEXT('1') || Ch > TEXT('9'))
		{
			OutError = TEXT("1~9 사이의 숫자만 입력해주세요.");
			return false;
		}
		if (Seen.Contains(Ch))
		{
			OutError = FString::Printf(TEXT("중복된 숫자입니다. (%s) 서로 다른 숫자 3개를 입력해주세요."), *FString::Chr(Ch));
			return false;
		}
		Seen.Add(Ch);
	}
	return true;
}

FString ABBGameMode::CheckAnswer(const FString& Input) const
{
	int32 Strike = 0;
	int32 Ball = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (Input[i] == SecretNumber[i])
		{
			++Strike;
		}
		else if (SecretNumber.Contains(FString::Chr(Input[i])))
		{
			++Ball;
		}
	}

	if (Strike == 0 && Ball == 0)
	{
		return TEXT("OUT");
	}
	return FString::Printf(TEXT("%dS%dB"), Strike, Ball);
}

void ABBGameMode::HandlePlayerMessage(ABBPlayerController* Sender, const FString& Message)
{
	if (!Sender)
	{
		return;
	}

	ABBPlayerState* PS = Sender->GetPlayerState<ABBPlayerState>();
	if (!PS)
	{
		return;
	}

	// 1) '/'로 시작하지 않으면 일반 채팅
	if (!Message.StartsWith(TEXT("/")))
	{
		BroadcastMessage(FString::Printf(TEXT("%s: %s"), *PS->GetPlayerName(), *Message));
		return;
	}

	// 2) 숫자 야구 입력
	const FString Guess = Message.Mid(1).TrimStartAndEnd();

	if (bGameOver)
	{
		Sender->ClientReceiveMessage(TEXT("[시스템] 결과 발표 중입니다. 잠시 후 새 게임이 시작됩니다."));
		return;
	}

	if (!PS->HasChancesLeft())
	{
		Sender->ClientReceiveMessage(TEXT("[시스템] 기회를 모두 사용했습니다."));
		return;
	}

	FString Error;
	if (!IsValidInput(Guess, Error))
	{
		// 기회는 소진하지 않는다
		Sender->ClientReceiveMessage(FString::Printf(TEXT("[시스템] %s"), *Error));
		return;
	}

	const FString Result = CheckAnswer(Guess);
	PS->AddTryCount();

	BroadcastMessage(FString::Printf(TEXT("[%s] %s → %s %s"),
		*PS->GetPlayerName(), *Guess, *Result, *PS->GetTryCountString()));

	CheckGameEnd(PS, Result);
}

void ABBGameMode::CheckGameEnd(ABBPlayerState* Guesser, const FString& Result)
{
	// 승리: 3S
	if (Result == TEXT("3S0B"))
	{
		bGameOver = true;
		BroadcastAnnouncement(FString::Printf(TEXT("%s 승리!"), *Guesser->GetPlayerName()));
		GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ABBGameMode::ResetGame, ResetDelay, false);
		return;
	}

	// 무승부: 모든 플레이어가 기회를 소진
	bool bAllOut = true;
	for (APlayerState* Raw : GameState->PlayerArray)
	{
		const ABBPlayerState* PS = Cast<ABBPlayerState>(Raw);
		if (PS && PS->HasChancesLeft())
		{
			bAllOut = false;
			break;
		}
	}

	if (bAllOut)
	{
		bGameOver = true;
		BroadcastAnnouncement(TEXT("무승부!"));
		GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ABBGameMode::ResetGame, ResetDelay, false);
	}
}

void ABBGameMode::ResetGame()
{
	SecretNumber = GenerateRandomNumbers();
	bGameOver = false;
	UE_LOG(LogTemp, Warning, TEXT("[Baseball] 새 정답: %s"), *SecretNumber);

	for (APlayerState* Raw : GameState->PlayerArray)
	{
		if (ABBPlayerState* PS = Cast<ABBPlayerState>(Raw))
		{
			PS->ResetTryCount();
		}
	}

	BroadcastAnnouncement(TEXT("")); // 공지 지우기
	BroadcastMessage(TEXT("[시스템] 새 게임이 시작되었습니다!"));
}

void ABBGameMode::BroadcastMessage(const FString& Message) const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABBPlayerController* PC = Cast<ABBPlayerController>(It->Get()))
		{
			PC->ClientReceiveMessage(Message);
		}
	}
}

void ABBGameMode::BroadcastAnnouncement(const FString& Text) const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABBPlayerController* PC = Cast<ABBPlayerController>(It->Get()))
		{
			PC->ClientShowAnnouncement(Text);
		}
	}
}
