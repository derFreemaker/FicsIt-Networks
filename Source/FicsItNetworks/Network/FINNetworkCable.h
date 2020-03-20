#pragma once

#include "CoreMinimal.h"
#include "FGBuildable.h"
#include "Components/SplineMeshComponent.h"
#include "FINNetworkCable.generated.h"

class UFINNetworkConnector;

UCLASS()
class FICSITNETWORKS_API AFINNetworkCable : public AFGBuildable {
	GENERATED_BODY()

public:
	UPROPERTY()
	UFINNetworkConnector* Connector1;

	UPROPERTY()
	UFINNetworkConnector* Connector2;

	UPROPERTY(EditDefaultsOnly)
	USplineMeshComponent* CableSpline;

	AFINNetworkCable();
	~AFINNetworkCable();

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGDismantleInterface
	virtual void Dismantle_Implementation() override;
	virtual void GetDismantleRefund_Implementation(TArray<FInventoryStack>& out_refund) const override;
	// End IFGDismantleInterface
};