#include "Core/MovementTuningHUD.h"

#include "Characters/Phase1Character.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

namespace
{
    FString FormatTuningValue(float Value)
    {
        const float AbsValue = FMath::Abs(Value);
        if (AbsValue >= 100.0f)
        {
            return FString::Printf(TEXT("%.0f"), Value);
        }
        if (AbsValue >= 10.0f)
        {
            return FString::Printf(TEXT("%.1f"), Value);
        }
        return FString::Printf(TEXT("%.3f"), Value);
    }
}

void AMovementTuningHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !PlayerOwner || !GEngine)
    {
        return;
    }

    const APhase1Character* Character = Cast<APhase1Character>(PlayerOwner->GetPawn());
    UFont* Font = GEngine->GetSmallFont();
    if (!Character || !Font)
    {
        return;
    }

    const float HorizontalSpeed = Character->GetVelocity().Size2D();
    DrawText(
        FString::Printf(TEXT("Speed: %.0f cm/s  (%.2f m/s)"), HorizontalSpeed, HorizontalSpeed / 100.0f),
        FLinearColor::White,
        22.0f,
        20.0f,
        Font,
        1.05f,
        false);

    DrawText(
        TEXT("F1: Movement Tuning"),
        FLinearColor(0.65f, 0.85f, 1.0f, 1.0f),
        22.0f,
        43.0f,
        Font,
        1.0f,
        false);

    if (!Character->IsMovementTuningVisible())
    {
        return;
    }

    const int32 Count = Character->GetMovementTuningCount();
    const int32 Selected = Character->GetMovementTuningSelection();

    const float PanelX = 18.0f;
    const float PanelY = 70.0f;
    const float PanelWidth = 520.0f;
    const float RowHeight = 21.0f;
    const float PanelHeight = 86.0f + Count * RowHeight;

    DrawRect(
        FLinearColor(0.02f, 0.02f, 0.025f, 0.88f),
        PanelX,
        PanelY,
        PanelWidth,
        PanelHeight);

    DrawText(
        TEXT("MOVEMENT TUNING - live runtime values"),
        FLinearColor::White,
        PanelX + 12.0f,
        PanelY + 10.0f,
        Font,
        1.05f,
        false);

    DrawText(
        TEXT("Up/Down select | Left/Right fine | PgUp/PgDn coarse | R reset selected | F2 reset all"),
        FLinearColor(0.72f, 0.72f, 0.72f, 1.0f),
        PanelX + 12.0f,
        PanelY + 33.0f,
        Font,
        0.88f,
        false);

    DrawText(
        TEXT("Changes apply immediately. Values reset when Play session restarts."),
        FLinearColor(0.72f, 0.72f, 0.72f, 1.0f),
        PanelX + 12.0f,
        PanelY + 51.0f,
        Font,
        0.88f,
        false);

    float Y = PanelY + 76.0f;
    for (int32 Index = 0; Index < Count; ++Index)
    {
        const bool bSelected = Index == Selected;
        const FLinearColor Color =
            bSelected ? FLinearColor(1.0f, 0.82f, 0.18f, 1.0f) : FLinearColor::White;

        const FString Prefix = bSelected ? TEXT("> ") : TEXT("  ");
        const FString Line = FString::Printf(
            TEXT("%s%-28s %s"),
            *Prefix,
            *Character->GetMovementTuningName(Index),
            *FormatTuningValue(Character->GetMovementTuningValue(Index)));

        DrawText(
            Line,
            Color,
            PanelX + 12.0f,
            Y,
            Font,
            0.96f,
            false);

        Y += RowHeight;
    }
}
