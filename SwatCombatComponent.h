// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimeLineComponent.h"
#include "SwatCombatComponent.generated.h"

class ASwatCharacter;
class APlayerBullet;
class AParentsPlayerState;
class AMainPlayerController;
class USoundCue;
class USoundBase;

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESTPROJECT_API USwatCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USwatCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	ASwatCharacter* mainChar;
	AMainPlayerController* mainPlayerController;
	float respawnDelayTime;

	float maxRecoilPitch;
	float totalRecoilPitch;

	float minRecoilYaw;
	float maxRecoilYaw;

	int recoilCnt;
	int maxRecoilCnt;

	float pitchRecoilVal;
	float yawRecoilVal;

	float headShotMultiply;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* recoilCurveFloat;

	FTimeline recoilFTimeline;

	float recoilCurveLength;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* muzzleParticle;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USoundCue* fire_Sound;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ultimateMuzzleParticle;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USoundBase* ultimateFire_Sound;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE AMainPlayerController* Getsex()const { return mainPlayerController; }

	void LocalFire(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState);

	void FireProjectileWeapon(FVector _startLoc, FVector _endLoc, AParentsPlayerState* _playerState);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_OnFire(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate);
	bool Server_OnFire_Validate(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate);
	void Server_OnFire_Implementation(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_OnFire(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate);
	bool Multi_OnFire_Validate(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate);
	void Multi_OnFire_Implementation(FTransform _bulletSpawnTransform, AParentsPlayerState* _playerState, bool _bUltimate);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();
	bool Server_Reload_Validate();
	void Server_Reload_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_Reload();
	bool Multi_Reload_Validate();
	void Multi_Reload_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_Death(FVector _causerLoc, AParentsPlayerState* _damagedPlayer, AParentsPlayerState* _damageCauserPlayer, bool _bHead, bool _bExplosion);
	bool Server_Death_Validate(FVector _causerLoc, AParentsPlayerState* _damagedPlayer, AParentsPlayerState* _damageCauserPlayer, bool _bHead, bool _bExplosion);
	void Server_Death_Implementation(FVector _causerLoc, AParentsPlayerState* _damagedPlayer, AParentsPlayerState* _damageCauserPlayer, bool _bHead, bool _bExplosion);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_Death(FVector _causerLoc, bool _bExplosion);
	bool Multi_Death_Validate(FVector _causerLoc, bool _bExplosion);
	void Multi_Death_Implementation(FVector _causerLoc, bool _bExplosion);

	UFUNCTION(Client, Reliable, WithValidation,BlueprintCallable)
	void Damaged(ASwatCharacter* _damagedActor, float _damage, AParentsPlayerState* _damageCauserState, FVector _causerLoc, FVector _hitLoc, bool _bHeadShot, bool _bExplosion);
	bool Damaged_Validate(ASwatCharacter* _damagedActor, float _damage, AParentsPlayerState* _damageCauserState, FVector _causerLoc, FVector _hitLoc, bool _bHeadShot, bool _bExplosion);
	void Damaged_Implementation(ASwatCharacter* _damagedActor, float _damage, AParentsPlayerState* _damageCauserState, FVector _causerLoc, FVector _hitLoc, bool _bHeadShot, bool _bExplosion);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnWeapon(bool _bUltimate);
	bool Server_SpawnWeapon_Validate(bool _bUltimate);
	void Server_SpawnWeapon_Implementation(bool _bUltimate);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_SpawnWeapon(bool _bUltimate);
	bool Multi_SpawnWeapon_Validate(bool _bUltimate);
	void Multi_SpawnWeapon_Implementation(bool _bUltimate);

	UFUNCTION(BlueprintCallable)
	void SetMainChar(ASwatCharacter* _mainChar);

	UFUNCTION(BlueprintCallable)
	void SetCtrl(AMainPlayerController* _ctrl);

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnBullet(FTransform _transform, AParentsPlayerState* _playerState);

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnGrenade(FTransform _transform, AParentsPlayerState* _playerState);

	bool CheckDeath(AParentsPlayerState* _damagedSwatPlayerState, float _damage);

	void RespawnDelay(float _delay);

	void SwatRecoil(bool _reset = false);

	UFUNCTION()
	void SwatRecoilCurve(float _val);

	void SpawnMuzzleEffectAndSound();

	void SpawnUltimateMuzzleEffectAndSound();

};
