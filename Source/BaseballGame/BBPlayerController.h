#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BBPlayerController.generated.h"

class UBBMainWidget;

UCLASS()
class BASEBALLGAME_API ABBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** 채팅 입력을 끝내고 키보드 포커스를 게임 화면으로 돌려준다. */
	void EndChatInput();

	/** 클라이언트 → 서버: 채팅/숫자 입력 전달 */
	UFUNCTION(Server, Reliable)
	void ServerSendMessage(const FString& Message);

	/** 서버 → 해당 클라이언트: 채팅 로그에 한 줄 추가 */
	UFUNCTION(Client, Reliable)
	void ClientReceiveMessage(const FString& Message);

	/** 서버 → 해당 클라이언트: 결과 공지(빈 문자열이면 지움) */
	UFUNCTION(Client, Reliable)
	void ClientShowAnnouncement(const FString& Text);

private:
	/** 엔터키를 누르면 채팅 입력창으로 포커스를 옮긴다. */
	void OnEnterPressed();

	UPROPERTY()
	TObjectPtr<UBBMainWidget> MainWidget;
};
