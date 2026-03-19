#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Food.h"
#include "Materials/MaterialInterface.h"
#include "Snake.generated.h"

class ASnakeManager;
class ASnakeSegment;

UCLASS()
class AISNAKE_API ASnake : public APawn
{
	GENERATED_BODY()

public:
	ASnake();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override {}

	// Called by SnakeManager after spawning
	void Initialize(ASnakeManager* Manager, FVector2D StartPos);

	/** Enqueue a new desired direction (validated immediately for 180-turn) */
	UFUNCTION(BlueprintCallable, Category="Snake")
	void SetDesiredDirection(FVector2D NewDir);

	UFUNCTION(BlueprintCallable, Category="Snake")
	void StartAcceleration();

	UFUNCTION(BlueprintCallable, Category="Snake")
	void StopAcceleration();

	// Class for spawning segment actors (set on Blueprint child)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<ASnakeSegment> SegmentClass;

	/** State materials – assign in BP_Snake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* NormalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* InvisibleMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	UMaterialInterface* InvincibleMaterial = nullptr;

	// Visual root mesh (head)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* HeadMesh;

private:
	UPROPERTY()
	ASnakeManager* ManagerRef = nullptr;

	// Body (index 0 = head position, 1… = tail)
	TArray<FVector2D> BodyPositions;

	UPROPERTY()
	TArray<ASnakeSegment*> Segments; // index 0 is unused (head uses HeadMesh)

	FVector2D CurrentDirection   = FVector2D(1.f, 0.f);
	TArray<FVector2D> DirQueue;  // buffered direction changes

	// Move timers
	FTimerHandle MoveTimerHandle;
	float NormalMoveInterval     = 0.2f;
	float AccelMoveInterval      = 0.08f;
	bool  bAccelerating          = false;

	// Status effects
	bool  bInvisible             = false;
	bool  bInvincible            = false;
	FTimerHandle InvisTimerHandle;
	FTimerHandle InvincTimerHandle;

	// ─── Internal methods ───
	void ScheduleMoveTimer(float Interval);
	void MoveStep();

	FVector2D WrapPosition(FVector2D Pos) const;
	void GrowBody();
	void ApplyFoodEffect(EFoodType Type);

	/** Apply Mat to the head mesh and every body segment. */
	void ApplyMaterialToAll(UMaterialInterface* Mat);

	/** Returns the material that matches the current status effect. */
	UMaterialInterface* GetCurrentMaterial() const;

	void EndInvisible()  { bInvisible  = false; ApplyMaterialToAll(GetCurrentMaterial()); }
	void EndInvincible() { bInvincible = false; ApplyMaterialToAll(GetCurrentMaterial()); }

	FVector GridToWorld(FVector2D GridPos) const;
};
