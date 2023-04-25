// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MPShooterGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTER_API AMPShooterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Players")
	TArray<FLinearColor> PlayerColors;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Players")
	int32 LastPlayerColorIndex;

public:
	AMPShooterGameModeBase(const FObjectInitializer& ObjectInitializer);
	virtual void SetPlayerDefaults(class APawn* PlayerPawn) override;
	
};
