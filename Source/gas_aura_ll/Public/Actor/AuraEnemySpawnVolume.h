// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SaveInterface.h"
#include "AuraEnemySpawnVolume.generated.h"

class UBoxComponent;
class AAuraEnemySpawnPoint;

UCLASS()
class GAS_AURA_LL_API AAuraEnemySpawnVolume : public AActor, public ISaveInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAuraEnemySpawnVolume();

	/* Save Interface */
	virtual void LoadActor_Implementation() override;
	/* end Save Interface */

	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bReached = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	TArray<AAuraEnemySpawnPoint*> SpawnPoints;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;
};
