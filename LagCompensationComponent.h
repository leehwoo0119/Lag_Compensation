// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float time;

	class UCapsuleComponent* hitCapsuleInfo;

	UPROPERTY()
	FVector hitCapsuleLocation;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();
	friend class ASwatCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	void SaveFramePackage(FFramePackage& Package);

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	void SaveCapsulePosition(ASwatCharacter* HitCharacter, FFramePackage& OutFramePackage);

	void MoveCapsulePosition(ASwatCharacter* HitCharacter, const FFramePackage& Package);

	void ResetCapsulePosition(ASwatCharacter* HitCharacter, const FFramePackage& Package);
	//많은 연산을 하기 때문에 최대한 FVector를 압축하기 위해서 FVector_NetQuantize를 활용
	bool ServerSideRewind(
		ASwatCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	bool ProjectileHitCheck(
		const FFramePackage& Package,
		ASwatCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ProjectileServerScoreRequest(
		ASwatCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime, ASwatCharacter* HitCharacter);

private:
	UPROPERTY()
	ASwatCharacter* mainChar;

	UPROPERTY()
	class AMainPlayerController* ctrl;
		
	TDoubleLinkedList<FFramePackage> frameHistory;

	UPROPERTY(EditAnywhere)
	float maxRecordTime = 4.f;

};
