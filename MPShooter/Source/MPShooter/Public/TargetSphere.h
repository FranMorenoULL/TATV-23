// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetSphere.generated.h"

UCLASS()
class MPSHOOTER_API ATargetSphere : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(ReplicatedUsing = OnRep_Color, EditAnywhere, BlueprintReadWrite, Category = "Target")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	float ColorChangeDuration;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	class UMaterialInstanceDynamic* MeshMID;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	FLinearColor PreviousColor;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	float LastColorChangeTime;

public:
	ATargetSphere(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitializeComponents() override;
	virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void OnRep_Color();
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

};