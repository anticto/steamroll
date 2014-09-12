

#pragma once

#include "Object.h"
#include "ExplosionDestructibleInterface.generated.h"

/** Class needed to support InterfaceCast<IExplosionDestructibleInterface>(Object) */
UINTERFACE(MinimalAPI)
class UExplosionDestructibleInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


class IExplosionDestructibleInterface
{
	GENERATED_IINTERFACE_BODY()

	//virtual bool IsDestructible() const = 0;
};
