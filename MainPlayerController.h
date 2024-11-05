// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class TESTPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMainPlayerController();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual float GetServerTime();

private:
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	float HighPingRunningTime = 0.f;
	float CurPing = 0.f;
	float CheckPingFrequency = 1.f;
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float HighPingThreshold = 100.f;

	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float SingleTripTime = 0.f;

	float ClientServerDelta = 0.f; // difference between client and server time

public:
	FHighPingDelegate highPingDelegate;

public:
	void CheckPing(float _deltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HighPingWarning();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StopHighPingWarning();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawCurPing();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowChat(bool _bFofused);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void AddMessage(const FString& _sender, const FString& _message);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void AddPlayerScore(const TArray<AParentsPlayerState*>& playerRankArr);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawEndPlayerScore();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawKillDeathPlayer(AParentsPlayerState* _damagedPlayer, AParentsPlayerState* _damageCauserPlayer, bool _bHead);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SendMessage(const FString& _sender, const FString& _message);
	bool Server_SendMessage_Validate(const FString& _sender, const FString& _message);
	void Server_SendMessage_Implementation(const FString& _sender, const FString& _message);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_PossessPlayer(APawn* _playerParents);
	bool Server_PossessPlayer_Validate(APawn* _playerParents);
	void Server_PossessPlayer_Implementation(APawn* _playerParents);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SendPlayerScore();
	bool Server_SendPlayerScore_Validate();
	void Server_SendPlayerScore_Implementation();

	UFUNCTION(Client, Reliable, WithValidation, BlueprintCallable)
	void Client_SetPlayerName();
	bool Client_SetPlayerName_Validate();
	void Client_SetPlayerName_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_SetPlayerName(const FString& _val);
	bool Server_SetPlayerName_Validate(const FString& _val);
	void Server_SetPlayerName_Implementation(const FString& _val);

	UFUNCTION(BlueprintCallable)
	void PossessPlayer(APawn* _playerParents);

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurPing()const { return CurPing; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetHighPingThreshold()const { return HighPingThreshold; }
};
