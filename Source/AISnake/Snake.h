#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SnakeSegment.h"
#include "Snake.generated.h"

// Broadcast after each grid move so SnakeManager can check collisions
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSnakeMoved);

UCLASS()
class AISNAKE_API ASnake : public APawn
{
	GENERATED_BODY()

public:
	ASnake();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ---- Mesh & Materials ----
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* HeadMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* NormalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* InvisibleMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* InvincibleMaterial;

	// ---- Initialization ----
	void Initialize(FIntPoint StartPos, FIntPoint StartDir,
	                int32 InGridCount, float InCellSize,
	                TSubclassOf<ASnakeSegment> InSegmentClass);

	// ---- Input ----
	void RequestDirection(FIntPoint NewDir);
	void SetAccelerating(bool bAccel);
	bool IsAccelerating() const { return bAccelerating; }

	// ---- Body ----
	void AddSegment();
	void Cleanup(); // destroys all segment actors

	// ---- Effects ----
	void ApplyInvisibleEffect(float Duration);
	void ApplyInvincibleEffect(float Duration);
	bool IsInvisible()  const { return bIsInvisible; }
	bool IsInvincible() const { return bIsInvincible; }

	// ---- Queries ----
	FIntPoint GetHeadGridPosition() const;
	TArray<FIntPoint> GetBodyGridPositions() const;

	// ---- Delegate ----
	UPROPERTY(BlueprintAssignable)
	FOnSnakeMoved OnSnakeMoved;

private:
	void DoMove();
	void ApplyCurrentEffectMaterial();
	FVector GridToWorld(FIntPoint GridPos) const;

	// Grid config (set by Initialize)
	int32 GridCount      = 20;
	float GridCellSize   = 100.0f;

	// Positions: index 0 = head, 1..N = body segments
	TArray<FIntPoint> BodyPositions;
	TArray<ASnakeSegment*> Segments;
	TSubclassOf<ASnakeSegment> SegmentClass;

	FIntPoint CurrentDirection  = FIntPoint(0, 1); // start moving right (screen)
	bool      bHasPendingDir    = false;
	FIntPoint PendingDirection;

	// Movement timing
	float MoveTimer         = 0.0f;
	float NormalInterval    = 0.3f;  // seconds per cell, normal speed
	float FastInterval      = 0.1f;  // accelerated
	bool  bAccelerating     = false;

	// Effects
	bool  bIsInvisible      = false;
	float InvisibleTimer    = 0.0f;
	bool  bIsInvincible     = false;
	float InvincibleTimer   = 0.0f;
};
