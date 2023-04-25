// Copyright Epic Games, Inc. All Rights Reserved.


#include "MPShooterGameModeBase.h"

#include "ShooterPawn.h"

AMPShooterGameModeBase::AMPShooterGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	PlayerColors.Add(FLinearColor(0.30f, 0.02f, 0.02f));
	PlayerColors.Add(FLinearColor(0.02f, 0.30f, 0.02f));
	PlayerColors.Add(FLinearColor(0.02f, 0.02f, 0.30f));
	LastPlayerColorIndex = -1;
	
	DefaultPawnClass = AShooterPawn::StaticClass();
}

void AMPShooterGameModeBase::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (AShooterPawn* ShooterPawn = Cast<AShooterPawn>(PlayerPawn))
	{
		if (const int32 PlayerColorIndex = (LastPlayerColorIndex + 1) % PlayerColors.Num(); PlayerColors.IsValidIndex(PlayerColorIndex))
		{
			ShooterPawn->AuthSetColor(PlayerColors[PlayerColorIndex]);
			LastPlayerColorIndex = PlayerColorIndex;
		}
	}
}