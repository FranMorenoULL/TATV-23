#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterPawn.generated.h"

UCLASS()
class MPSHOOTER_API AShooterPawn : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	class UMaterialInstanceDynamic* MeshMID;
	
	UPROPERTY(ReplicatedUsing = OnRep_Color, Transient, BlueprintReadOnly, Category = "Player")
	FLinearColor Color;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* WeaponHandle;

	UPROPERTY(ReplicatedUsing = OnRep_Weapon, Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	class AWeapon* Weapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AimTraceDistance;

public:
	AShooterPawn(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void AuthSetColor(const FLinearColor& InColor);

protected:
	
	UFUNCTION() void OnFire();
	UFUNCTION() void OnMoveForward(float Axis);
	UFUNCTION() void OnMoveRight(float Axis);
	UFUNCTION() void OnMoveUp(float Axis);
	UFUNCTION() void OnLookRight(float Axis);
	UFUNCTION() void OnLookUp(float Axis);
	
	UFUNCTION()	void OnRep_Color() const;
	UFUNCTION() void OnRep_Weapon() const;
};