// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicCircle.generated.h"

UCLASS()
class GAS_AURA_LL_API AMagicCircle : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMagicCircle();
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDecalComponent> MagicCircleDecal;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
