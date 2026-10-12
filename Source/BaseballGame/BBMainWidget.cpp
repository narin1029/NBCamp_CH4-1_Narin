#include "BBMainWidget.h"
#include "BBPlayerController.h"
#include "BBPlayerState.h"

#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "Brushes/SlateColorBrush.h"

void UBBMainWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree)
	{
		return;
	}

	// 루트: Overlay (맨 아래 배경 + 그 위에 내용)
	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Overlay"));
	WidgetTree->RootWidget = Overlay;

	// 배경 - 색을 바꾸려면 아래 FLinearColor 값을 수정 (R,G,B,A)
	UImage* Background = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Background"));
	Background->SetColorAndOpacity(FLinearColor(0.03f, 0.05f, 0.12f, 1.f));
	// 이미지를 쓰고 싶다면:
	// if (UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/T_Background.T_Background")))
	// {
	//     Background->SetBrushFromTexture(Tex);
	//     Background->SetColorAndOpacity(FLinearColor::White);
	// }
	UOverlaySlot* BgSlot = Overlay->AddChildToOverlay(Background);
	BgSlot->SetHorizontalAlignment(HAlign_Fill);
	BgSlot->SetVerticalAlignment(VAlign_Fill);

	// 내용
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	UOverlaySlot* ContentSlot = Overlay->AddChildToOverlay(Root);
	ContentSlot->SetHorizontalAlignment(HAlign_Fill);
	ContentSlot->SetVerticalAlignment(VAlign_Fill);

	// 시도 횟수
	AttemptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AttemptText"));
	AttemptText->SetText(FText::FromString(TEXT("[0 / 3]")));
	{
		FSlateFontInfo Font = AttemptText->GetFont();
		Font.Size = 20;
		AttemptText->SetFont(Font);
	}
	Root->AddChildToVerticalBox(AttemptText)->SetPadding(FMargin(10.f));

	// 결과 공지 (승리 / 무승부)
	AnnounceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AnnounceText"));
	AnnounceText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
	AnnounceText->SetJustification(ETextJustify::Center);
	{
		FSlateFontInfo Font = AnnounceText->GetFont();
		Font.Size = 40;
		AnnounceText->SetFont(Font);
	}
	Root->AddChildToVerticalBox(AnnounceText)->SetPadding(FMargin(10.f));

	// 채팅 로그 (남는 공간을 모두 차지)
	ChatLog = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ChatLog"));
	UVerticalBoxSlot* LogSlot = Root->AddChildToVerticalBox(ChatLog);
	LogSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	LogSlot->SetPadding(FMargin(10.f));

	// 입력창
	ChatInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("ChatInput"));
	// 입력창 스타일: 글자는 검정, 배경은 밝은 회색 (검정 글자가 보이도록)
	{
		const FSlateColorBrush LightBrush(FLinearColor(0.9f, 0.9f, 0.9f, 1.f));

		FEditableTextBoxStyle Style = ChatInput->WidgetStyle;
		Style.SetForegroundColor(FSlateColor(FLinearColor::Black));        // 평소 글자색
		Style.SetFocusedForegroundColor(FSlateColor(FLinearColor::Black)); // 입력 중 글자색
		Style.SetReadOnlyForegroundColor(FSlateColor(FLinearColor::Black));
		Style.SetBackgroundColor(FSlateColor(FLinearColor::White));
		Style.SetBackgroundImageNormal(LightBrush);
		Style.SetBackgroundImageHovered(LightBrush);
		Style.SetBackgroundImageFocused(LightBrush);
		Style.SetBackgroundImageReadOnly(LightBrush);
		ChatInput->WidgetStyle = Style;
	}
	ChatInput->SetHintText(FText::FromString(TEXT("엔터를 눌러 채팅 시작 (숫자 야구는 /123)")));
	ChatInput->OnTextCommitted.AddUniqueDynamic(this, &UBBMainWidget::OnChatCommitted);
	Root->AddChildToVerticalBox(ChatInput)->SetPadding(FMargin(10.f));

	AddChatMessage(TEXT("[안내] 1~9 중복 없는 3자리 숫자를 맞혀보세요. 기회는 3번! 예) /123"));
	AddChatMessage(TEXT("[안내] 엔터 키를 누르면 입력창이 활성화되고, 다시 엔터를 누르면 전송됩니다. (ESC: 취소)"));
}

void UBBMainWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// PlayerState의 복제된 값을 표시
	if (AttemptText)
	{
		if (const ABBPlayerState* PS = GetOwningPlayerState<ABBPlayerState>())
		{
			AttemptText->SetText(FText::FromString(PS->GetTryCountString()));
		}
	}
}

void UBBMainWidget::AddChatMessage(const FString& Message)
{
	if (!ChatLog || !WidgetTree)
	{
		return;
	}

	UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Line->SetText(FText::FromString(Message));
	Line->SetAutoWrapText(true);
	ChatLog->AddChild(Line);

	if (ChatLog->GetChildrenCount() > 100)
	{
		ChatLog->RemoveChildAt(0);
	}
	ChatLog->ScrollToEnd();
}

void UBBMainWidget::ShowAnnouncement(const FString& Text)
{
	if (AnnounceText)
	{
		AnnounceText->SetText(FText::FromString(Text));
	}
}

void UBBMainWidget::FocusChatInput()
{
	if (ChatInput)
	{
		ChatInput->SetKeyboardFocus();
	}
}

void UBBMainWidget::OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	ABBPlayerController* PC = GetOwningPlayer<ABBPlayerController>();

	// ESC: 입력 취소
	if (CommitMethod == ETextCommit::OnCleared)
	{
		ChatInput->SetText(FText::GetEmpty());
		if (PC)
		{
			PC->EndChatInput();
		}
		return;
	}

	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString Message = Text.ToString().TrimStartAndEnd();
	ChatInput->SetText(FText::GetEmpty());

	if (PC)
	{
		if (!Message.IsEmpty())
		{
			PC->ServerSendMessage(Message);
		}
		PC->EndChatInput(); // 전송 후 입력창에서 빠져나와 다시 '엔터 대기' 상태로
	}
}
