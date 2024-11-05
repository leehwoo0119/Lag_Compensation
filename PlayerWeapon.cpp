// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "MainPlayerController.h"
#include "SwatCharacter.h"

// Sets default values
APlayerWeapon::APlayerWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("weaponMesh"));
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called when the game starts or when spawned
void APlayerWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerWeapon, bUseServerSideRewind);
}

void APlayerWeapon::WeaponRagdol(ASwatCharacter* _player)
{
	if (!_player)return;
	AMainPlayerController* mainPlayerController = Cast<AMainPlayerController>(_player->Controller);

	if (mainPlayerController && HasAuthority() && !mainPlayerController->highPingDelegate.IsBound() && bUseServerSideRewind)
	{
		mainPlayerController->highPingDelegate.RemoveDynamic(this, &APlayerWeapon::OnPingTooHigh);
	}

	weaponMesh->SetSimulatePhysics(true);
	weaponMesh->SetCollisionProfileName(FName("ragdoll"));
	weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void APlayerWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
	//UE_LOG(LogTemp, Log, TEXT("bHighPing: %d || bUseServerSideRewind: %d"), bPingTooHigh, bUseServerSideRewind);
}

void APlayerWeapon::OnEquipped(ASwatCharacter* _player)
{
	if (!_player)return;
	AMainPlayerController* mainPlayerController = Cast<AMainPlayerController>(_player->Controller);

	if (mainPlayerController && HasAuthority() && !mainPlayerController->highPingDelegate.IsBound() && bUseServerSideRewind)
	{
		mainPlayerController->highPingDelegate.AddDynamic(this, &APlayerWeapon::OnPingTooHigh);
	}
}
