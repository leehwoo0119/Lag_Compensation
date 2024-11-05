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

	//frameHistory�� ũ�Ⱑ 0,1�϶� ��忡 ���� ������ �߰�
	if (frameHistory.Num() <= 1)
	{
		FFramePackage thisFrame;
		SaveFramePackage(thisFrame);
		frameHistory.AddHead(thisFrame);
	}
	else
	{
		//frameHistory�� ó���� �� �ð��� ����
		float historyLength = frameHistory.GetHead()->GetValue().time - frameHistory.GetTail()->GetValue().time;

		//historyLength�� ������ �ִ� �ð��� �Ѿ�� ���� ���� �� historyLength �ٽð��
		while (historyLength > maxRecordTime)
		{
			frameHistory.RemoveNode(frameHistory.GetTail());
			historyLength = frameHistory.GetHead()->GetValue().time - frameHistory.GetTail()->GetValue().time;
		}
		//���� ���� ������ �߰�
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
	//YoungerFrame�� OlderFrame ���̿� �ִ� HitTime�� ��ġ�� �м��� ���� �� ����
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
	//������ �� ������ ���� �����ϱ� ���� FFramePackage ����
	FFramePackage currentFrame;

	//���� FFramePackage ����
	SaveCapsulePosition(HitCharacter, currentFrame);

	//ServerRewind�� ��ġ�� Capsule �̵�
	MoveCapsulePosition(HitCharacter, Package);

	//Projectile�� �̵���θ� ������ �浹 Ȯ��
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

	//�浹 �� (���� ����)
	if (pathResult.HitResult.bBlockingHit)
	{
		//CapsulePosition ���󺹱�
		ResetCapsulePosition(HitCharacter, currentFrame);

		//���� ����
		return true;
	}

	ResetCapsulePosition(HitCharacter, currentFrame);

	//���� ����
	return false;
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ASwatCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	bool bAttackSuccess = ServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	if (mainChar && HitCharacter && bAttackSuccess)
	{
		//UE_LOG(LogTemp, Log, TEXT("bAttackSuccess"));

		//��� ������ �ӽ�
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
	//������ �� ������ �ʿ��� ���
	bool bShouldInterpolate = true;

	//���� �߻� ��Ű�� �ʱ����� ������ ����
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->frameHistory;

	//���� �ֱٰ� ������ frameHistory ����
	const float oldestHistoryTime = History.GetTail()->GetValue().time;
	const float newestHistoryTime = History.GetHead()->GetValue().time;

	//������ �ʹ� ���� �߻��ϴ� ���
	if (oldestHistoryTime > HitTime)
	{
		return false;
	}
	//HitTime�� oldestHistoryTime�� �Ȱ��� ���(���ɼ� ���)
	if (oldestHistoryTime == HitTime)
	{
		frameToCheck = History.GetTail()->GetValue();
		//oldestHistoryTime == HitTime �̹Ƿ� ������ �ʿ䰡 ����
		bShouldInterpolate = false;
	}

	//HitTime�� newestHistoryTime���� �Ȱ��ų� ū ���(���ɼ� ���)
	if (newestHistoryTime <= HitTime)
	{
		frameToCheck = History.GetHead()->GetValue();
		//�� ���� frameToCheck�� frameHistory�� �ֽ� time���� �����ϹǷ� ������ �ʿ䰡 ����
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* older = younger;

	while (older->GetValue().time > HitTime) //HitTime�� Older���� ������� üũ
	{
		//�ش� while���� olderTime < HitTime < youngerTime �� ���� Ȯ����

		if (older->GetNextNode() == nullptr) break;	//����� �� ����

		//���� ��� Ž������ ���� ���� ����
		older = older->GetNextNode();

		if (older->GetValue().time > HitTime)
		{
			younger = older;
		}
	}
	//HitTime�� olderTime�� ��Ȯ�� ������ ���(���ɼ� ���)
	if (older->GetValue().time == HitTime)
	{
		frameToCheck = older->GetValue();
		//HitTime�� olderTime�� ��Ȯ�� �����ϹǷ� ������ �ʿ䰡 ����
		bShouldInterpolate = false;
	}
	//������ �ʿ��� ���
	if (bShouldInterpolate)
	{
		//older�� younger ���̸� ����
		frameToCheck = InterpBetweenFrames(older->GetValue(), younger->GetValue(), HitTime, HitCharacter);
	}

	ShowFramePackage(frameToCheck, FColor::Red);

	return ProjectileHitCheck(frameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}



