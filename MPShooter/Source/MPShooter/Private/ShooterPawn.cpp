
#include "ShooterPawn.h"

#include "Components/CapsuleComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon.h"

// Sets default values
AShooterPawn::AShooterPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("SkeletalMesh'/Engine/EngineMeshes/SkeletalCube.SkeletalCube'")
	);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
		TEXT("Material'/Game/MPShooter/Materials/Pawn/M_Pawn.M_Pawn'")
	);

	BaseEyeHeight = 18.0f;
	bUseControllerRotationPitch = true;

	if (UCapsuleComponent* CollisionComponent = GetCapsuleComponent())
	{
		CollisionComponent->SetCapsuleSize(32.0f, 40.0f, false);
	}
	
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent)
	{
		MeshComponent->SetSkeletalMesh(MeshFinder.Object);
		MeshComponent->SetMaterial(0, MaterialFinder.Object);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -22.0f));
		MeshComponent->SetRelativeScale3D(FVector(2.0f));
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DefaultLandMovementMode = MOVE_Flying;
		MovementComponent->MaxAcceleration = 5000.0f;
		MovementComponent->MaxFlySpeed = 800.0f;
		MovementComponent->BrakingDecelerationFlying = 5000.0f;
	}
	
	WeaponHandle = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("WeaponHandle"));
	WeaponHandle->SetUsingAbsoluteScale(true);
	WeaponHandle->SetupAttachment(MeshComponent ? MeshComponent : RootComponent);
	WeaponHandle->SetRelativeLocation(FVector(15.0f, 8.0f, 6.0f));
	
	AimTraceDistance = 3000.0f;
}

void AShooterPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Weapon)
	{
		const FVector ViewLocation = GetPawnViewLocation();
		const FTransform ViewTransform = GetMesh() ? GetMesh()->GetComponentTransform() : GetActorTransform();
		const FVector ViewForward = ViewTransform.GetUnitAxis(EAxis::X);

		const FVector& TraceStart = ViewLocation;
		const FVector TraceEnd = TraceStart + (ViewForward * AimTraceDistance);
		const FName ProfileName = UCollisionProfile::BlockAllDynamic_ProfileName;
		const FCollisionQueryParams QueryParams(TEXT("PlayerAim"), false, this);

		FHitResult Hit;
		FVector WorldAimLocation;
		if (GetWorld()->LineTraceSingleByProfile(Hit, TraceStart, TraceEnd, ProfileName, QueryParams)) {

			WorldAimLocation = Hit.ImpactPoint;
		}
		else {
			WorldAimLocation = TraceEnd;
		}

		const FVector ViewAimLocation = ViewTransform.InverseTransformPosition(WorldAimLocation);

		Weapon->UpdateAimLocation(WorldAimLocation, ViewAimLocation);
	}
}

// Called to bind functionality to input
void AShooterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AShooterPawn::OnFire);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterPawn::OnMoveForward);

	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterPawn::OnMoveRight);

	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &AShooterPawn::OnMoveUp);

	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &AShooterPawn::OnLookRight);

	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AShooterPawn::OnLookUp);
}

void AShooterPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPawn, Color);
	DOREPLIFETIME(AShooterPawn, Weapon);
}

void AShooterPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshMID = MeshComponent->CreateDynamicMaterialInstance(0);
	}
	if (HasAuthority())
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		const FVector SpawnLocation = WeaponHandle->GetComponentLocation();
		const FRotator SpawnRotation = WeaponHandle->GetComponentRotation();

		Weapon = GetWorld()->SpawnActor<AWeapon>(SpawnLocation, SpawnRotation, SpawnInfo);
		OnRep_Weapon();
	}
}

void AShooterPawn::AuthSetColor(const FLinearColor& InColor)
{
	Color = InColor;
	OnRep_Color();
}

void AShooterPawn::OnFire()
{
	if (Weapon)
	{
		Weapon->HandleFireInput();
	}
}

void AShooterPawn::OnMoveForward(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ViewForward = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::X);
		AddMovementInput(ViewForward, AxisValue);
	}
}

void AShooterPawn::OnMoveRight(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ViewRight = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(ViewRight, AxisValue);
	}
}

void AShooterPawn::OnMoveUp(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		AddMovementInput(FVector::UpVector, AxisValue);
	}
}

void AShooterPawn::OnLookRight(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

void AShooterPawn::OnLookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

void AShooterPawn::OnRep_Color() const
{
	if (MeshMID)
	{
		MeshMID->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void AShooterPawn::OnRep_Weapon() const
{
	if (Weapon)
	{
		Weapon->AttachToComponent(WeaponHandle, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}