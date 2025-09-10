// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "Carla/Sensor/FisheyeSensor.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"

#include "Engine/TextureRenderTargetCube.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Runtime/RenderCore/Public/RenderingThread.h"

#include "CoreGlobals.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"
#include "CubemapUnwrapUtils.h"
#include "Kismet/KismetSystemLibrary.h"


#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>


static auto SCENE_CAPTURE_COUNTER_CUBE = 0u;

AFisheyeSensor::AFisheyeSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTargetCube>(FName(*FString::Printf(TEXT("CaptureRenderTarget_d%d"), SCENE_CAPTURE_COUNTER_CUBE)));
  CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
  CaptureRenderTarget->SRGB = false;  

  Fisheye = CreateDefaultSubobject<USceneCaptureComponentCube>(
      FName(*FString::Printf(TEXT("SceneCaptureComponentCube_%d"), SCENE_CAPTURE_COUNTER_CUBE)));
  Fisheye-> bCaptureRotation = true;
  Fisheye->SetupAttachment(RootComponent);
  Fisheye->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
  ++SCENE_CAPTURE_COUNTER_CUBE;
}

FActorDefinition AFisheyeSensor::GetSensorDefinition()
{
  auto Definition = UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(
      TEXT("camera"),
      TEXT("fisheye"));

  // Resolution (standard attribute names)
  FActorVariation ResX;
  ResX.Id = TEXT("image_size_x");
  ResX.Type = EActorAttributeType::Int;
  ResX.RecommendedValues = { TEXT("800") };
  ResX.bRestrictToRecommended = false;

  FActorVariation ResY;
  ResY.Id = TEXT("image_size_y");
  ResY.Type = EActorAttributeType::Int;
  ResY.RecommendedValues = { TEXT("600") };
  ResY.bRestrictToRecommended = false;

  FActorVariation XSize;
  XSize.Id = TEXT("x_size");
  XSize.Type = EActorAttributeType::Float;
  XSize.RecommendedValues = { TEXT("1000.0") };
  XSize.bRestrictToRecommended = false;

  FActorVariation YSize;
  YSize.Id = TEXT("y_size");
  YSize.Type = EActorAttributeType::Float;
  YSize.RecommendedValues = { TEXT("900.0") };
  YSize.bRestrictToRecommended = false;

  FActorVariation MaxAngle;
  MaxAngle.Id = TEXT("max_angle");
  MaxAngle.Type = EActorAttributeType::Float;
  MaxAngle.RecommendedValues = { TEXT("200.0") };
  MaxAngle.bRestrictToRecommended = false;

  FActorVariation Fx;
  Fx.Id = TEXT("f_x");
  Fx.Type = EActorAttributeType::Float;
  Fx.RecommendedValues = { TEXT("300.0") };
  Fx.bRestrictToRecommended = false;

  FActorVariation Fy;
  Fy.Id = TEXT("f_y");
  Fy.Type = EActorAttributeType::Float;
  Fy.RecommendedValues = { TEXT("300.0") };
  Fy.bRestrictToRecommended = false;

  FActorVariation Cx;
  Cx.Id = TEXT("c_x");
  Cx.Type = EActorAttributeType::Float;
  Cx.RecommendedValues = { TEXT("600.0") };
  Cx.bRestrictToRecommended = false;

  FActorVariation Cy;
  Cy.Id = TEXT("c_y");
  Cy.Type = EActorAttributeType::Float;
  Cy.RecommendedValues = { TEXT("400.0") };
  Cy.bRestrictToRecommended = false;

  FActorVariation D1;
  D1.Id = TEXT("d_1");
  D1.Type = EActorAttributeType::Float;
  D1.RecommendedValues = { TEXT("0.0") };
  D1.bRestrictToRecommended = false;

  FActorVariation D2;
  D2.Id = TEXT("d_2");
  D2.Type = EActorAttributeType::Float;
  D2.RecommendedValues = { TEXT("0.0") };
  D2.bRestrictToRecommended = false;

  FActorVariation D3;
  D3.Id = TEXT("d_3");
  D3.Type = EActorAttributeType::Float;
  D3.RecommendedValues = { TEXT("0.0") };
  D3.bRestrictToRecommended = false;

  FActorVariation D4;
  D4.Id = TEXT("d_4");
  D4.Type = EActorAttributeType::Float;
  D4.RecommendedValues = { TEXT("0.0") };
  D4.bRestrictToRecommended = false;

  // Optional cube face size
  FActorVariation Cube;
  Cube.Id = TEXT("cube_size");
  Cube.Type = EActorAttributeType::Int;
  Cube.RecommendedValues = { TEXT("512") };
  Cube.bRestrictToRecommended = false;

  Definition.Variations.Append({ ResX, ResY, XSize, YSize, MaxAngle, Fx, Fy, Cx, Cy, D1, D2, D3, D4, Cube});

  return Definition;
}

void AFisheyeSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);

  // Prefer standard image_size_* if provided, else fall back to x_size/y_size
  const bool bHasImageX = Description.Variations.Contains(TEXT("image_size_x"));
  const bool bHasImageY = Description.Variations.Contains(TEXT("image_size_y"));

  if (bHasImageX) {
    XSize = static_cast<float>(UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      TEXT("image_size_x"), Description.Variations, 800));
  } else {
    XSize = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      TEXT("x_size"), Description.Variations, 1000.0f);
  }

  if (bHasImageY) {
    YSize = static_cast<float>(UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      TEXT("image_size_y"), Description.Variations, 600));
  } else {
    YSize = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      TEXT("y_size"), Description.Variations, 900.0f);
  }

  MaxAngle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "max_angle",
      Description.Variations,
      200.0f);

  Fx = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "f_x",
      Description.Variations,
      300.0f);

  Fy = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "f_y",
      Description.Variations,
      300.0f);

  Cx = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "c_x",
      Description.Variations,
      600.0f);

  Cy = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "c_y",
      Description.Variations,
      400.0f);

  D1 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_1",
      Description.Variations,
      0.0f);

  D2 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_2",
      Description.Variations,
      0.0f);

  D3 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_3",
      Description.Variations,
      0.0f);

  D4 = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "d_4",
      Description.Variations,
      0.0f);

  CubeSize = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      TEXT("cube_size"),
      Description.Variations,
      0);
  CaptureRenderTarget -> SizeX = (CubeSize > 0 ? CubeSize : static_cast<int32>(XSize));
}

int32 AFisheyeSensor::GetImageWidth() const
{
  check(CaptureRenderTarget != nullptr);
  return XSize;
}

int32 AFisheyeSensor::GetImageHeight() const
{
  check(CaptureRenderTarget != nullptr);
  return YSize;
}

float AFisheyeSensor::GetFOV() const
{
  check(CaptureRenderTarget != nullptr);
  return MaxAngle;
}

template <typename TSensor>
void AFisheyeSensor::SendPixelsInRenderThread(TSensor &Sensor, float MaxAngle, float SizeX, float SizeY, float Fx, float Fy, float Cx, float Cy, float D1, float D2, float D3, float D4)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AFisheyeSensor::SendPixelsInRenderThread);
  check(Sensor.CaptureRenderTarget != nullptr);

  if (Sensor.IsPendingKill()) {
    return;
  }

  UTextureRenderTargetCube *RenderTarget = Sensor.CaptureRenderTarget;

  TArray64<uint8> PixelData;
  FIntPoint Size;
  EPixelFormat Format;

  CubemapHelpersFisheye::FFisheyeParams FisheyeParams;
  FisheyeParams.ImageSize = FIntPoint(SizeX, SizeY);
  FisheyeParams.CameraMatrix = FVector4(Fx, Fy, Cx, Cy);
  FisheyeParams.DistortionCoeffs = FVector4(D1, D2, D3, D4);
  FisheyeParams.MaxAngle = MaxAngle;

  // Must be called on game thread since it uses GameThread_GetRenderTargetResource internally
  bool bUnwrapSuccess = CubemapHelpersFisheye::GenerateLongLatUnwrapFisheye(
      RenderTarget, PixelData, FisheyeParams, Size, Format);

  if (!bUnwrapSuccess) {
    return;
  }

  auto Stream = Sensor.GetDataStream(Sensor);
  auto Buffer = Stream.PopBufferFromPool();

  Buffer.copy_from(
    carla::sensor::SensorRegistry::get<TSensor *>::type::header_offset,
    PixelData.GetData(),
    PixelData.Num());

  if (Buffer.data()) {
    SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
    Stream.Send(Sensor, std::move(Buffer));
  }
}

void AFisheyeSensor::BeginPlay()
{

  const int32 Face = (CubeSize > 0 ? CubeSize : static_cast<int32>(GetImageWidth()));
  CaptureRenderTarget-> Init(Face, PF_B8G8R8A8);

  Fisheye->Deactivate();
  Fisheye->TextureTarget = CaptureRenderTarget;
  Fisheye->CaptureScene();

  Fisheye->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR; 

  Fisheye->UpdateContent();
  Fisheye->Activate();

  UKismetSystemLibrary::ExecuteConsoleCommand(
      GetWorld(),
      FString("g.TimeoutForBlockOnRenderFence 300000"));

  GetEpisode().GetWeather()->NotifyWeather();

  Super::BeginPlay();
}

void AFisheyeSensor::PrePhysTick(float DeltaTime)
{
  Super::PrePhysTick(DeltaTime);
  IStreamingManager::Get().AddViewInformation(
      Fisheye->GetComponentLocation(),
      XSize,
      YSize);
  Fisheye->UpdateContent();
  SendPixelsInRenderThread(*this, MaxAngle, XSize, YSize, Fx, Fy, Cx, Cy, D1, D2, D3, D4);

  ReadyToCapture = true;
}

void AFisheyeSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  Super::PostPhysTick(World, TickType, DeltaTime);
  ReadyToCapture = true;
}

void AFisheyeSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  SCENE_CAPTURE_COUNTER_CUBE = 0u;
}
