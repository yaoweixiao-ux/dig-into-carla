// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaWheeledVehicleNW.h"
#include "WheeledVehicleMovementComponentNW.h"

ACarlaWheeledVehicleNW::ACarlaWheeledVehicleNW(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer.SetDefaultSubobjectClass<UWheeledVehicleMovementComponentNW>(AWheeledVehicle::VehicleMovementComponentName))
{
  bIsNWVehicle = true;
}

ACarlaWheeledVehicleNW::~ACarlaWheeledVehicleNW() {}
