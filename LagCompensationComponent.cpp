// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "SwatCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerWeapon.h"

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (mainChar == nullptr || !mainChar->HasAuthority()) return;

	//frameHistory의 크기가 0,1일때 헤드에 현재 프레임 추가
	if (frameHistory.Num() <= 1)
	{
		FFramePackage thisFrame;
		SaveFramePackage(thisFrame);
		frameHistory.AddHead(thisFrame);
	}
	else
	{
		//frameHistory의 처음과 끝 시간의 차이
		float historyLength = frameHistory.GetHead()->GetValue().time - frameHistory.GetTail()->GetValue().time;

		//historyLength가 지정한 최대 시간을 넘어가면 꼬리 삭제 후 historyLength 다시계산
		while (historyLength > maxRecordTime)
		{
			frameHistory.RemoveNode(frameHistory.GetTail());
			historyLength = frameHistory.GetHead()->GetValue().time - frameHistory.GetTail()->GetValue().time;
		}
		//이후 현재 프레임 추가
		FFramePackage thisFrame;
		SaveFramePackage(thisFrame);
		frameHistory.AddHead(thisFrame);

		//ShowFramePackage(thisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	mainChar = mainChar == nullptr ? Cast<ASwatCharacter>(GetOwner()) : mainChar;

	Package.time = GetWorld()->GetTimeSeconds();
	Package.hitCapsuleInfo = mainChar->GetCapsuleComponent();
	Package.hitCapsuleLocation = mainChar->GetCapsuleComponent()->GetComponentLocation();
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	DrawDebugCapsule(
		GetWorld(),
		Package.hitCapsuleLocation,
		Package.hitCapsuleInfo->GetScaledCapsuleHalfHeight(),
		Package.hitCapsuleInfo->GetScaledCapsuleRadius(),
		FQuat(Package.hitCapsuleInfo->GetComponentRotation()),
		Color,
		false,
		4.f);
}

void ULagCompensationComponent::SaveCapsulePosition(ASwatCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	if (HitCharacter->GetCapsuleComponent() != nullptr)
	{
		OutFramePackage.hitCapsuleLocation = HitCharacter->GetCapsuleComponent()->GetComponentLocation();
	}
}

void ULagCompensationComponent::MoveCapsulePosition(ASwatCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	if (HitCharacter->GetCapsuleComponent() != nullptr && HitCharacter->GetMesh())
	{
		HitCharacter->GetCapsuleComponent()->SetWorldLocation(Package.hitCapsuleLocation);
		HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ULagCompensationComponent::ResetCapsulePosition(ASwatCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	if (HitCharacter->GetCapsuleComponent() != nullptr && HitCharacter->GetMesh())
	{
		HitCharacter->GetCapsuleComponent()->SetWorldLocation(Package.hitCapsuleLocation);
		//HitCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime, ASwatCharacter* HitCharacter)
{
	//YoungerFrame과 OlderFrame 사이에 있는 HitTime의 위치를 분수로 구한 뒤 보간
	const float distance = YoungerFrame.time - OlderFrame.time;
	const float interpFraction = FMath::Clamp((HitTime - OlderFrame.time) / distance, 0.f, 1.f);

	FFramePackage interpFramePackage;
	interpFramePackage.time = HitTime;
	interpFramePackage.hitCapsuleInfo = HitCharacter->GetCapsuleComponent();
	interpFramePackage.hitCapsuleLocation = FMath::VInterpTo(OlderFrame.hitCapsuleLocation, YoungerFrame.hitCapsuleLocation, 1.f, interpFraction);

	return interpFramePackage;
}

bool ULagCompensationComponent::ProjectileHitCheck(const FFramePackage& Package, ASwatCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	//스왑할 때 기존의 값을 저장하기 위한 FFramePackage 생성
	FFramePackage currentFrame;

	//기존 FFramePackage 저장
	SaveCapsulePosition(HitCharacter, currentFrame);

	//ServerRewind한 위치로 Capsule 이동
	MoveCapsulePosition(HitCharacter, Package);

	//Projectile을 이동경로를 예측해 충돌 확인
	FPredictProjectilePathParams pathParams;
	pathParams.bTraceWithCollision = true;
	pathParams.MaxSimTime = maxRecordTime;
	pathParams.LaunchVelocity = InitialVelocity;
	pathParams.StartLocation = TraceStart;
	pathParams.SimFrequency = 15.f;
	pathParams.ProjectileRadius = 5.f;
	//PathParams.TraceChannel = ECollisionChannel::ECC_GameTraceChannel2;
	pathParams.ActorsToIgnore.Add(GetOwner());

	FPredictProjectilePathResult pathResult;
	UGameplayStatics::PredictProjectilePath(this, pathParams, pathResult);

	//충돌 시 (공격 성공)
	if (pathResult.HitResult.bBlockingHit)
	{
		//CapsulePosition 원상복귀
		ResetCapsulePosition(HitCharacter, currentFrame);

		//공격 성공
		return true;
	}

	ResetCapsulePosition(HitCharacter, currentFrame);

	//공격 실패
	return false;
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ASwatCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	bool bAttackSuccess = ServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	if (mainChar && HitCharacter && bAttackSuccess)
	{
		//UE_LOG(LogTemp, Log, TEXT("bAttackSuccess"));

		//즉사 데미지 임시
		const float damage = 200.0f;

		//PlayerBullet->TakeProjectileDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			damage,
			mainChar->Controller,
			mainChar,
			UDamageType::StaticClass()
		);
	}
}

bool ULagCompensationComponent::ServerSideRewind(ASwatCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->frameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->frameHistory.GetTail() == nullptr)
		return false;

	FFramePackage frameToCheck;
	//프레임 간 보간이 필요한 경우
	bool bShouldInterpolate = true;

	//복제 발생 시키지 않기위해 참조로 진행
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->frameHistory;

	//가장 최근과 과거의 frameHistory 저장
	const float oldestHistoryTime = History.GetTail()->GetValue().time;
	const float newestHistoryTime = History.GetHead()->GetValue().time;

	//지연이 너무 많이 발생하는 경우
	if (oldestHistoryTime > HitTime)
	{
		return false;
	}
	//HitTime이 oldestHistoryTime과 똑같은 경우(가능성 희박)
	if (oldestHistoryTime == HitTime)
	{
		frameToCheck = History.GetTail()->GetValue();
		//oldestHistoryTime == HitTime 이므로 보간할 필요가 없음
		bShouldInterpolate = false;
	}

	//HitTime이 newestHistoryTime보다 똑같거나 큰 경우(가능성 희박)
	if (newestHistoryTime <= HitTime)
	{
		frameToCheck = History.GetHead()->GetValue();
		//이 역시 frameToCheck를 frameHistory의 최신 time으로 갱신하므로 보간할 필요가 없음
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* older = younger;

	while (older->GetValue().time > HitTime) //HitTime이 Older보다 작은경우 체크
	{
		//해당 while문은 olderTime < HitTime < youngerTime 일 때를 확인함

		if (older->GetNextNode() == nullptr) break;	//노드의 끝 도달

		//이전 노드 탐색이후 다음 노드로 변경
		older = older->GetNextNode();

		if (older->GetValue().time > HitTime)
		{
			younger = older;
		}
	}
	//HitTime과 olderTime이 정확히 유사한 경우(가능성 희박)
	if (older->GetValue().time == HitTime)
	{
		frameToCheck = older->GetValue();
		//HitTime과 olderTime이 정확히 유사하므로 보간할 필요가 없음
		bShouldInterpolate = false;
	}
	//보간이 필요한 경우
	if (bShouldInterpolate)
	{
		//older과 younger 사이를 보간
		frameToCheck = InterpBetweenFrames(older->GetValue(), younger->GetValue(), HitTime, HitCharacter);
	}

	ShowFramePackage(frameToCheck, FColor::Red);

	return ProjectileHitCheck(frameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}



