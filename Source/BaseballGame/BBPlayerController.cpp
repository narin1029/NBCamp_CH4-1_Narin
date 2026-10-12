#include "BBPlayerController.h"
#include "BBGameMode.h"
#include "BBMainWidget.h"
#include "Engine/World.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void ABBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	MainWidget = CreateWidget<UBBMainWidget>(this, UBBMainWidget::StaticClass());
	if (MainWidget)
	{
		MainWidget->AddToViewport();

		bShowMouseCursor = true;
		EndChatInput(); // 시작은 '입력 대기' 상태. 엔터를 눌러야 채팅 시작
	}
}

void ABBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ABBPlayerController::OnEnterPressed);
	}
}

void ABBPlayerController::OnEnterPressed()
{
	if (MainWidget)
	{
		MainWidget->FocusChatInput();
	}
}

void ABBPlayerController::EndChatInput()
{
	// 게임 + UI 모드: 입력창에 포커스가 없을 때는 키 입력이 컨트롤러(InputComponent)로 들어온다.
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);

	UWidgetBlueprintLibrary::SetFocusToGameViewport();
}

void ABBPlayerController::ServerSendMessage_Implementation(const FString& Message)
{
	// 서버에서만 실행된다. 너무 긴 입력은 잘라서 방어.
	const FString Safe = Message.Left(200);
	if (Safe.IsEmpty())
	{
		return;
	}

	if (ABBGameMode* GM = GetWorld()->GetAuthGameMode<ABBGameMode>())
	{
		GM->HandlePlayerMessage(this, Safe);
	}
}

void ABBPlayerController::ClientReceiveMessage_Implementation(const FString& Message)
{
	if (MainWidget)
	{
		MainWidget->AddChatMessage(Message);
	}
}

void ABBPlayerController::ClientShowAnnouncement_Implementation(const FString& Text)
{
	if (MainWidget)
	{
		MainWidget->ShowAnnouncement(Text);
	}
}
