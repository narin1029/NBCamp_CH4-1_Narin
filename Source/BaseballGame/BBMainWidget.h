#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BBMainWidget.generated.h"

class UEditableTextBox;
class UScrollBox;
class UTextBlock;

/** 에디터 작업 없이 C++만으로 UI를 구성하는 메인 위젯 (시도 횟수 / 공지 / 채팅 로그 / 입력창). */
UCLASS()
class BASEBALLGAME_API UBBMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void AddChatMessage(const FString& Message);
	void ShowAnnouncement(const FString& Text);
	void FocusChatInput();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> AttemptText;

	UPROPERTY()
	TObjectPtr<UTextBlock> AnnounceText;

	UPROPERTY()
	TObjectPtr<UScrollBox> ChatLog;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> ChatInput;
};
