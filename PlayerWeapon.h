// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerWeapon.generated.h"

class USkeletalMeshComponent;

UCLASS()
class TESTPROJECT_API APlayerWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* weaponMesh;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const { return weaponMesh; }

	UFUNCTION(BlueprintCallable)
	void WeaponRagdol(class ASwatCharacter* _player);

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	void OnEquipped(ASwatCharacter* _player);

public:
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere)
	bool bUseServerSideRewind= true;

};
