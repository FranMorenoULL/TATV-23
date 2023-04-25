// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponFirePacket
{
	GENERATED_BODY();

	UPROPERTY()
	float ServerFireTime;

	UPROPERTY()
	bool bCausedDamage;

	UPROPERTY()
	FVector_NetQuantize ImpactPoint;

	UPROPERTY()
	FVector_NetQuantizeNormal ImpactNormal;
};

UCLASS()
class MPSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* MuzzleHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	float AimInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	float DropInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FRotator DropRotation;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Aiming|State")
	FVector AimLocation;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Aiming|State")
	bool bAimLocationIsValid;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float FireCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float LastFireTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	class UNiagaraSystem* FireEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	class UNiagaraSystem* ImpactEffect;

	UPROPERTY(ReplicatedUsing = OnRep_LastFirePacket, VisibleAnywhere, BlueprintReadOnly, Category = "Firing|State")
	FWeaponFirePacket LastFirePacket;
	
public:
	AWeapon(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation);
	void HandleFireInput();

	UFUNCTION(Server, Reliable)
	void Server_TryFire(const FVector& MuzzleLocation, const FVector& Direction);

	UFUNCTION()
	void OnRep_LastFirePacket() const;
	
protected:
	virtual void BeginPlay() override;

private:
	void PlayFireEffects() const;
	void PlayImpactEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bCausedDamage) const;
	bool RunFireTrace(FHitResult& OutHit) const;
};