// Fill out your copyright notice in the Description page of Project Settings.


#include "SwatCombatComponent.h"
#include "SwatAnimInstance.h"
#include "SwatCharacter.h"
#include "MainPlayerController.h"
#include "PlayerWeapon.h"
#include "ParentsPlayerState.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerBullet.h"
#include "Components/CapsuleComponent.h"
#include "TestProjectGameMode.h"
#include "Sound\SoundCue.h" 

// Sets default values for this component's properties
USwatCombatComponent::USwatCombatComponent():respawnDelayTime(6.0f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	maxRecoilPitch = 5.0f;
	minRecoilYaw = -0.6f;
	maxRecoilYaw = 0.6f;

	maxRecoilCnt = 15;

	recoilCurveLength = 0.15f;

	headShotMultiply = 5.0f;
}


// Called when the game starts
void USwatCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	check(recoilCurveFloat != nullptr);
	FOnTimelineFloat recoilCallback;
	recoilCallback.BindUFunction(this, FName("SwatRecoilCurve"));
	recoilFTimeline.AddInterpFloat(recoilCurveFloat, recoilCallback);
	recoilFTimeline.SetTimelineLength(recoilCurveLength);
}

void USwatCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	recoilFTimeline.TickTimeline(DeltaTime);
}

void USwatCombatComponent::LocalFire(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState)
{
	mainChar = mainChar == nullptr ? mainChar : Cast<ASwatCharacter>(GetOwner());

	mainChar->GetAnimInstance()->AnimMontagePlay(mainChar->GetAnimInstance()->combatMontage, 1.0f, FName("Fire"));
	SpawnMuzzleEffectAndSound();
	//UE_LOG(LogTemp, Log, TEXT("LocalFire"));

	SpawnBullet(_bulletSpawnTransform, _playerState);
	//if (mainChar->HasAuthority())
	//{
	//	if (mainChar->IsLocallyControlled())
	//	{
	//		
	//	}
	//}
}

void USwatCombatComponent::FireProjectileWeapon(FVector _startLoc, FVector _endLoc, AParentsPlayerState* _playerState)
{
	if (!mainChar)return;

	FHitResult outHit;
	FTransform bulletSpawnTransform;

	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(mainChar);
	GetWorld()->LineTraceSingleByChannel(
		outHit,
		_startLoc,
		_endLoc,
		ECC_Visibility,
		collisionParams);

	if (outHit.bBlockingHit)
	{
		bulletSpawnTransform =
			FTransform(
				UKismetMathLibrary::FindLookAtRotation(_startLoc, outHit.ImpactPoint),
				_startLoc,
				FVector(1, 1, 1)
			);
	}
	else
	{
		bulletSpawnTransform =
			FTransform(
				UKismetMathLibrary::FindLookAtRotation(_startLoc, _endLoc),
				_startLoc,
				FVector(1, 1, 1)
			);
	}
	if (!mainChar->HasAuthority()) LocalFire(bulletSpawnTransform, _playerState);
		
	Server_OnFire(bulletSpawnTransform, _playerState, false);	
}

bool USwatCombatComponent::Server_OnFire_Validate(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate)
{
	return true;
}

void USwatCombatComponent::Server_OnFire_Implementation(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate)
{	
	if (_bUltimate)
	{
		FVector launchVec = (mainChar->GetFollowCamera()->GetForwardVector() * -1000.0f) +
			(mainChar->GetFollowCamera()->GetUpVector() * 700.0f);
		mainChar->LaunchCharacter(launchVec, true, true);
	}
	Multi_OnFire(_bulletSpawnTransform, _playerState, _bUltimate);
}

bool USwatCombatComponent::Multi_OnFire_Validate(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate)
{
	return true;
}

void USwatCombatComponent::Multi_OnFire_Implementation(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate)
{
	if (mainChar && mainChar->IsLocallyControlled() && !mainChar->HasAuthority()) return;
	LocalFire(_bulletSpawnTransform, _playerState);
	//mainChar->GetAnimInstance()->AnimMontagePlay(mainChar->GetAnimInstance()->combatMontage, 1.0f, FName("Fire"));

	/*FHitResult outHit;
	FTransform bulletSpawnTransform;

	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(mainChar);
	GetWorld()->LineTraceSingleByChannel(
		outHit,
		_startLoc,
		_endLoc,
		ECC_Visibility,
		collisionParams);

	if (outHit.bBlockingHit)
	{
		bulletSpawnTransform =
			FTransform(
				UKismetMathLibrary::FindLookAtRotation(_startLoc, outHit.ImpactPoint),
				_startLoc,
				FVector(1, 1, 1)
			);
	}
	else
	{
		bulletSpawnTransform =
			FTransform(
				UKismetMathLibrary::FindLookAtRotation(_startLoc, _endLoc),
				_startLoc,
				FVector(1, 1, 1)
			);
	}*/

	/*if (_bUltimate)
	{
		SpawnUltimateMuzzleEffectAndSound();
		SpawnGrenade(bulletSpawnTransform, _playerState);
	}*/
	/*else
	{
		SpawnMuzzleEffectAndSound();
		SpawnBullet(bulletSpawnTransform, _playerState);
	}*/
}

bool USwatCombatComponent::Server_Reload_Validate()
{
	return true;
}

void USwatCombatComponent::Server_Reload_Implementation()
{
	Multi_Reload();
}

bool USwatCombatComponent::Multi_Reload_Validate()
{
	return true;
}

void USwatCombatComponent::Multi_Reload_Implementation()
{
	mainChar->GetAnimInstance()->AnimMontagePlay(mainChar->GetAnimInstance()->reloadMontage, 1.0f, FName("Reload"));
}

bool USwatCombatComponent::Server_Death_Validate(FVector _causerLoc, AParentsPlayerState* _damagedPlayer, AParentsPlayerState* _damageCauserPlaye, bool _bHead, bool _bExplosion)
{
	return true;
}

void USwatCombatComponent::Server_Death_Implementation(FVector _causerLoc, AParentsPlayerState* _damagedPlayer, AParentsPlayerState* _damageCauserPlayer, bool _bHead, bool _bExplosion)
{
	ATestProjectGameMode* testProjectGameMode = Cast<ATestProjectGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	check(testProjectGameMode != nullptr);

	_damagedPlayer->PlusDeathCnt();
	_damageCauserPlayer->PlusKillCnt();

	testProjectGameMode->SendKillDeathToEveryone(_damagedPlayer, _damageCauserPlayer, _bHead);

	Multi_Death(_causerLoc, _bExplosion);
	RespawnDelay(_damagedPlayer->GetRespawnDelay());
}

bool USwatCombatComponent::Multi_Death_Validate(FVector _causerLoc, bool _bExplosion)
{
	return true;
}

void USwatCombatComponent::Multi_Death_Implementation(FVector _causerLoc, bool _bExplosion)
{
	UE_LOG(LogTemp, Log, TEXT("Multi_Death_Implementation"));
	float impulseVal = 500.0f;
	if (_bExplosion)impulseVal *= 2;

	mainChar->GetPlayerWeapon()->WeaponRagdol(mainChar);
	mainChar->GetMesh()->SetSimulatePhysics(true);
	mainChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	mainChar->GetMesh()->SetCollisionProfileName(FName("ragdoll"), true);
	mainChar->GetMesh()->AddImpulse(_causerLoc * impulseVal * mainChar->GetMesh()->GetMass());
	mainChar->GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("root"), true);
	mainChar->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

bool USwatCombatComponent::Server_SpawnWeapon_Validate(bool _bUltimate)
{
	return true;
}

void USwatCombatComponent::Server_SpawnWeapon_Implementation(bool _bUltimate)
{
	Multi_SpawnWeapon(_bUltimate);
}

bool USwatCombatComponent::Multi_SpawnWeapon_Validate(bool _bUltimate)
{
	return true;
}

void USwatCombatComponent::Multi_SpawnWeapon_Implementation(bool _bUltimate)
{
	if (!mainChar->HasAuthority())
	{
		if (_bUltimate)
		{
			mainChar->GetPlayerWeapon()->GetWeaponMesh()->SetVisibility(false);
			mainChar->GetUltimateWeapon()->GetWeaponMesh()->SetVisibility(true);
		}
		else
		{
			mainChar->GetPlayerWeapon()->GetWeaponMesh()->SetVisibility(true);
			mainChar->GetUltimateWeapon()->GetWeaponMesh()->SetVisibility(false);
		}		
	}
}

void USwatCombatComponent::SetMainChar(ASwatCharacter* _mainChar)
{
	mainChar = _mainChar;	
	check(mainChar != nullptr);	
}

void USwatCombatComponent::SetCtrl(AMainPlayerController* _ctrl)
{
	mainPlayerController = _ctrl;
	check(mainPlayerController != nullptr);
}

void USwatCombatComponent::SwatRecoil(bool _reset)
{
	if (_reset || totalRecoilPitch >= maxRecoilPitch)
	{
		recoilCnt = 0;
		totalRecoilPitch = 0.0f;
		pitchRecoilVal = 0.0f;
		yawRecoilVal = 0.0f;
		return;
	}
	recoilCnt++;	

	if (recoilCnt < 3)
	{
		pitchRecoilVal = FMath::RandRange(0.1f, 0.2f);
		yawRecoilVal = FMath::RandRange(-0.1f, 0.2f);
	}
	else if (recoilCnt < 10)
	{
		pitchRecoilVal = FMath::RandRange(-0.1f, 0.4f);
		yawRecoilVal = FMath::RandRange(-0.3f, 0.5f);
	}
	else
	{		
		pitchRecoilVal = FMath::RandRange(-0.3f, 0.1f);
		yawRecoilVal = FMath::RandRange(-0.5f, 0.2f);
	}

	recoilFTimeline.PlayFromStart();
}

void USwatCombatComponent::SwatRecoilCurve(float _val)
{
	float movePitchVal = FMath::Lerp<float, float>(0.0f, pitchRecoilVal, _val);
	float moveYawVal = FMath::Lerp<float, float>(0.0f, yawRecoilVal, _val);

	totalRecoilPitch += (movePitchVal / 5);

	mainChar->AddControllerPitchInput(-movePitchVal / 5);
	mainChar->AddControllerYawInput(moveYawVal / 5);
}


bool USwatCombatComponent::Damaged_Validate(ASwatCharacter* _damagedActor, float _damage, AParentsPlayerState* _damageCauserState, FVector _causerLoc, FVector _hitLoc, bool _bHeadShot, bool _bExplosion)
{
	return true;
}

void USwatCombatComponent::Damaged_Implementation(ASwatCharacter* _damagedActor, float _damage, AParentsPlayerState* _damageCauserState, FVector _causerLoc, FVector _hitLoc, bool _bHeadShot, bool _bExplosion)
{
	AParentsPlayerState* damagedSwatPlayerState = Cast<AParentsPlayerState>(mainChar->GetPlayerState());

	check(_damageCauserState != nullptr);
	
	if (damagedSwatPlayerState)
	{
		float damage = _damage;

		if (_bHeadShot)damage *= headShotMultiply;

		if (damagedSwatPlayerState->GetDeath())return;

		if (CheckDeath(damagedSwatPlayerState, damage))
		{
			damagedSwatPlayerState->SetDeath(true);
			_damageCauserState->SetCurUltimateGauge(_damageCauserState->GetCurUltimateGauge() + 20.0f);
			Server_Death(_causerLoc, damagedSwatPlayerState, _damageCauserState, _bHeadShot, _bExplosion);
		}
		_damageCauserState->SetCurUltimateGauge(_damageCauserState->GetCurUltimateGauge() + 5.0f);
		damagedSwatPlayerState->SetSwatCurHP(damagedSwatPlayerState->GetSwatCurHP() - damage);
		
		damagedSwatPlayerState->DamagePostProcess(_hitLoc);
	}

}

bool USwatCombatComponent::CheckDeath(AParentsPlayerState* _damagedSwatPlayerState, float _damage)
{	
	return _damagedSwatPlayerState->GetSwatCurHP() - _damage <= 0;
}

void USwatCombatComponent::RespawnDelay(float _delay)
{
	if (!mainPlayerController && mainChar)mainPlayerController = Cast<AMainPlayerController>(mainChar->GetController());

	mainPlayerController->UnPossess();
	FTimerHandle WaitHandle;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			
			mainPlayerController->PossessPlayer(mainChar);

		}), _delay, false);

	//GetWorld()->GetTimerManager().ClearTimer(WaitHandle);
}

void USwatCombatComponent::SpawnMuzzleEffectAndSound()
{
	UGameplayStatics::PlaySoundAtLocation(this, fire_Sound, mainChar->GetPlayerWeapon()->GetActorLocation());
	UGameplayStatics::SpawnEmitterAttached(
		muzzleParticle,
		mainChar->GetPlayerWeapon()->GetWeaponMesh(),
		FName("MuzzleFlash"));
}

void USwatCombatComponent::SpawnUltimateMuzzleEffectAndSound()
{
	UGameplayStatics::SpawnSoundAttached(ultimateFire_Sound, mainChar->GetUltimateWeapon()->GetWeaponMesh());
	UGameplayStatics::SpawnEmitterAttached(
		ultimateMuzzleParticle,
		mainChar->GetUltimateWeapon()->GetWeaponMesh(),
		FName("MuzzleFlash"));
}


