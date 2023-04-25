// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "DamageType_WeaponFire.h"
#include "Engine/CollisionProfile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")
	);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
		TEXT("Material'/Game/MPShooter/Materials/Weapon/M_Weapon.M_Weapon'")
	);

	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));
	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetStaticMesh(MeshFinder.Object);
	MeshComponent->SetMaterial(0, MaterialFinder.Object);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComponent->SetRelativeLocation(FVector(20.0f, 0.0f, 0.0f));
	MeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 45.0f));
	MeshComponent->SetRelativeScale3D(FVector(0.5f, 0.15f, 0.15f));
	
	AimInterpSpeed = 8.0f;
	DropInterpSpeed = 10.0f;
	DropRotation = FRotator(-30.0f, -80.0f, 0.0f);

	MuzzleHandle = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("MuzzleHandle"));
	MuzzleHandle->SetupAttachment(RootComponent);
	MuzzleHandle->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
	
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FireEffectFinder(
		TEXT("/Script/Niagara.NiagaraSystem'/Game/MPShooter/FX/N_Fire_System.N_Fire_System'")
	);
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ImpactEffectFinder(
		TEXT("/Script/Niagara.NiagaraSystem'/Game/MPShooter/FX/N_Impact_System.N_Impact_System'")
	);

	FireCooldown = 0.4f;
	LastFireTime = TNumericLimits<float>::Lowest();
	FireEffect = FireEffectFinder.Object;
	ImpactEffect = ImpactEffectFinder.Object;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAimLocationIsValid)
	{
		const FVector AimDisplacement = AimLocation - GetActorLocation();
		const FVector AimDirection = AimDisplacement.GetSafeNormal();

		const FQuat TargetRotation = AimDirection.ToOrientationQuat();
		const FQuat NewRotation = FMath::QInterpTo(GetActorQuat(), TargetRotation, DeltaTime, AimInterpSpeed);
		SetActorRotation(NewRotation);
	}
	else
	{
		const AActor* AttachParent = GetAttachParentActor();
		const FQuat TargetRotation = AttachParent ? AttachParent->GetActorTransform().TransformRotation(FQuat(DropRotation)) : FQuat(DropRotation);
		const FQuat NewRotation = FMath::QInterpTo(GetActorQuat(), TargetRotation, DeltaTime, DropInterpSpeed);
		SetActorRotation(NewRotation);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, LastFirePacket, COND_SkipOwner);
}

void AWeapon::UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation)
{
	AimLocation = InWorldAimLocation;
	bAimLocationIsValid = InViewAimLocation.X > MuzzleHandle->GetRelativeLocation().X;
}

void AWeapon::HandleFireInput()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;
	if(ElapsedSinceLastFire>=FireCooldown)
	{
		const FVector MuzzleLocation = MuzzleHandle->GetComponentLocation();
		const FVector Direction = MuzzleHandle->GetComponentQuat().Vector();
		Server_TryFire(MuzzleLocation, Direction);
		LastFireTime = CurrentTime;
	
		if (!HasAuthority())
		{
			PlayFireEffects();
			FHitResult Hit;
			if (RunFireTrace(Hit))
			{
				const bool bWillProbablyCauseDamage = Hit.GetActor()->IsValidLowLevel() && Hit.GetActor()->CanBeDamaged();
				PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal, bWillProbablyCauseDamage);
			}
		}
	}
}

void AWeapon::Server_TryFire_Implementation(const FVector& MuzzleLocation, const FVector& Direction)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;

	if (ElapsedSinceLastFire >= FireCooldown)
	{
		LastFireTime = CurrentTime;
		LastFirePacket.ServerFireTime = CurrentTime;
		if (FHitResult Hit; RunFireTrace(Hit))
		{
			float DamageCaused = 0.0f;
			if (Hit.GetActor()->IsValidLowLevel() && Hit.GetActor()->CanBeDamaged())
			{
				constexpr float BaseDamage = 1.0f;
				const FPointDamageEvent DamageEvent(BaseDamage, Hit, Direction, UDamageType_WeaponFire::StaticClass());
				DamageCaused = Hit.GetActor()->TakeDamage(BaseDamage, DamageEvent, GetInstigatorController(), this);
			}
			PlayFireEffects();
			PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal, DamageCaused > 0.0f);

			LastFirePacket.bCausedDamage = DamageCaused > 0.0f;
			LastFirePacket.ImpactPoint = Hit.ImpactPoint;
			LastFirePacket.ImpactNormal = Hit.ImpactNormal;
		}
		else
		{
			LastFirePacket.ImpactNormal = FVector::ZeroVector;
		}
	}
}

void AWeapon::OnRep_LastFirePacket() const
{
	PlayFireEffects();

	if (!LastFirePacket.ImpactNormal.IsZero())
	{
		PlayImpactEffects(LastFirePacket.ImpactPoint, LastFirePacket.ImpactNormal, LastFirePacket.bCausedDamage);
	}
}

void AWeapon::PlayFireEffects() const
{
	if (FireEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(FireEffect, MuzzleHandle, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
	}
}

void AWeapon::PlayImpactEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bCausedDamage) const
{
	const FRotator ImpactRotation = ImpactNormal.ToOrientationRotator();
	if (ImpactEffect)
	{
		FVector scale(1.0, 1.0, 1.0);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactEffect, ImpactPoint, ImpactRotation,scale,false,true,ENCPoolMethod::None,true);
	}
}

bool AWeapon::RunFireTrace(FHitResult& OutHit) const
{
	const FVector& TraceStart = MuzzleHandle->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (MuzzleHandle->GetForwardVector() * 5000.0f);
	const FName ProfileName = UCollisionProfile::BlockAllDynamic_ProfileName;
	const FCollisionQueryParams QueryParams(TEXT("WeaponFire"), false, GetOwner());
	return GetWorld()->LineTraceSingleByProfile(OutHit, TraceStart, TraceEnd, ProfileName, QueryParams);
}
