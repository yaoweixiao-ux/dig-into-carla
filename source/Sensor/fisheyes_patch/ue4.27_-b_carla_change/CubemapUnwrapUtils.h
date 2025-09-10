// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	CubemapUnwapUtils.h: Pixel and Vertex shader to render a cube map as 2D texture
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "BatchedElements.h"
#include "GlobalShader.h"

class UTextureCube;
class UTextureRenderTargetCube;

namespace CubemapHelpers
{
	/**
	* Creates an unwrapped 2D image of the cube map ( longitude/latitude )
	* @param	CubeTexture	Source UTextureCube object.
	* @param	BitsOUT	Raw bits of the 2D image bitmap.
	* @param	SizeXOUT	Filled with the X dimension of the output bitmap.
	* @param	SizeYOUT	Filled with the Y dimension of the output bitmap.
	* @param	FormatOUT	Filled with the pixel format of the output bitmap.
	* @return	true on success.
	*/
	ENGINE_API bool GenerateLongLatUnwrap(const UTextureCube* CubeTexture, TArray64<uint8>& BitsOUT, FIntPoint& SizeOUT, EPixelFormat& FormatOUT);

	/**
	* Creates an unwrapped 2D image of the cube map ( longitude/latitude )
	* @param	CubeTarget	Source UTextureRenderTargetCube object.
	* @param	BitsOUT	Raw bits of the 2D image bitmap.
	* @param	SizeXOUT	Filled with the X dimension of the output bitmap.
	* @param	SizeYOUT	Filled with the Y dimension of the output bitmap.
	* @param	FormatOUT	Filled with the pixel format of the output bitmap.
	* @return	true on success.
	*/
	ENGINE_API bool GenerateLongLatUnwrap(const UTextureRenderTargetCube* CubeTarget, TArray64<uint8>& BitsOUT, FIntPoint& SizeOUT, EPixelFormat& FormatOUT);
}

namespace CubemapHelpersFisheye
{
    struct FFisheyeParams
    {
        FVector4 DistortionCoeffs;
        FVector4 CameraMatrix;
        FIntPoint ImageSize;
        float MaxAngle;
    };

    /**
    * Creates an unwrapped 2D fisheye image of the cube map.
    */
    ENGINE_API bool GenerateLongLatUnwrapFisheye(const UTextureCube* CubeTexture, TArray64<uint8>& BitsOUT, FFisheyeParams& FisheyeParams, FIntPoint& SizeOUT, EPixelFormat& FormatOUT);

    /**
    * Creates an unwrapped 2D fisheye image of the cube render target.
    */
    ENGINE_API bool GenerateLongLatUnwrapFisheye(const UTextureRenderTargetCube* CubeTarget, TArray64<uint8>& BitsOUT, FFisheyeParams& FisheyeParams, FIntPoint& SizeOUT, EPixelFormat& FormatOUT);
}

/**
 * A vertex shader for rendering a texture on a simple element.
 */
class FCubemapTexturePropertiesVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FCubemapTexturePropertiesVS,Global);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsPCPlatform(Parameters.Platform);}

	FCubemapTexturePropertiesVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		Transform.Bind(Initializer.ParameterMap,TEXT("Transform"), SPF_Mandatory);
	}
	FCubemapTexturePropertiesVS() {}

	void SetParameters(FRHICommandList& RHICmdList, const FMatrix& TransformValue);

private:
	LAYOUT_FIELD(FShaderParameter, Transform);
};

/**
 * Simple pixel shader reads from a cube map texture and unwraps it in the LongitudeLatitude form.
 */
template<bool bHDROutput>
class FCubemapTexturePropertiesPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FCubemapTexturePropertiesPS,Global);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsPCPlatform(Parameters.Platform);}

	FCubemapTexturePropertiesPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		CubeTexture.Bind(Initializer.ParameterMap,TEXT("CubeTexture"));
		CubeTextureSampler.Bind(Initializer.ParameterMap,TEXT("CubeTextureSampler"));
		ColorWeights.Bind(Initializer.ParameterMap,TEXT("ColorWeights"));
		PackedProperties0.Bind(Initializer.ParameterMap,TEXT("PackedProperties0"));
		Gamma.Bind(Initializer.ParameterMap,TEXT("Gamma"));
	}
	FCubemapTexturePropertiesPS() {}

	void SetParameters(FRHICommandList& RHICmdList, const FTexture* Texture, const FMatrix& ColorWeightsValue, float MipLevel, float GammaValue);

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("HDR_OUTPUT"), bHDROutput ? TEXT("1") : TEXT("0"));
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, CubeTexture);
	LAYOUT_FIELD(FShaderResourceParameter, CubeTextureSampler);
	LAYOUT_FIELD(FShaderParameter, PackedProperties0);
	LAYOUT_FIELD(FShaderParameter, ColorWeights);
	LAYOUT_FIELD(FShaderParameter, Gamma);
};


class ENGINE_API FMipLevelBatchedElementParameters : public FBatchedElementParameters
{
public:
	FMipLevelBatchedElementParameters(float InMipLevel, bool bInHDROutput = false)
		: bHDROutput(bInHDROutput)
		, MipLevel(InMipLevel)
	{
	}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
	template<typename TPixelShader> void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture);

	bool bHDROutput;
	/** Parameters that need to be passed to the shader */
	float MipLevel;
};


/**
 * Simple pixel shader that renders a IES light profile for the purposes of visualization.
 */
class FIESLightProfilePS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FIESLightProfilePS,Global);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) && !IsConsolePlatform(Parameters.Platform);
	}

	FIESLightProfilePS() {}

	FIESLightProfilePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		IESTexture.Bind(Initializer.ParameterMap,TEXT("IESTexture"));
		IESTextureSampler.Bind(Initializer.ParameterMap,TEXT("IESTextureSampler"));
		BrightnessInLumens.Bind(Initializer.ParameterMap,TEXT("BrightnessInLumens"));
	}

	void SetParameters(FRHICommandList& RHICmdList, const FTexture* Texture, float InBrightnessInLumens);

private:
	/** The texture to sample. */
	LAYOUT_FIELD(FShaderResourceParameter, IESTexture);
	LAYOUT_FIELD(FShaderResourceParameter, IESTextureSampler);
	LAYOUT_FIELD(FShaderParameter, BrightnessInLumens);
};

class ENGINE_API FIESLightProfileBatchedElementParameters : public FBatchedElementParameters
{
public:
	FIESLightProfileBatchedElementParameters(float InBrightnessInLumens) : BrightnessInLumens(InBrightnessInLumens)
	{
	}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
	float BrightnessInLumens;
};

/** Fisheye variants */
class FCubemapTexturePropertiesVSFisheye : public FGlobalShader
{
    DECLARE_SHADER_TYPE(FCubemapTexturePropertiesVSFisheye,Global);
public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsPCPlatform(Parameters.Platform); }

    FCubemapTexturePropertiesVSFisheye(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        Transform.Bind(Initializer.ParameterMap, TEXT("Transform"), SPF_Mandatory);
    }
    FCubemapTexturePropertiesVSFisheye() {}

    void SetParameters(FRHICommandList& RHICmdList, const FMatrix& TransformValue);

private:
    LAYOUT_FIELD(FShaderParameter, Transform);
};

template<bool bHDROutput>
class FCubemapTexturePropertiesPSFisheye : public FGlobalShader
{
    DECLARE_SHADER_TYPE(FCubemapTexturePropertiesPSFisheye,Global);
public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsPCPlatform(Parameters.Platform); }

    FCubemapTexturePropertiesPSFisheye(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        CubeTexture.Bind(Initializer.ParameterMap, TEXT("CubeTexture"));
        CubeTextureSampler.Bind(Initializer.ParameterMap, TEXT("CubeTextureSampler"));
        ColorWeights.Bind(Initializer.ParameterMap, TEXT("ColorWeights"));
        PackedProperties0.Bind(Initializer.ParameterMap, TEXT("PackedProperties0"));
        Gamma.Bind(Initializer.ParameterMap, TEXT("Gamma"));
        DistortionCoeffs.Bind(Initializer.ParameterMap, TEXT("DistortionCoeffs"));
        CameraMatrix.Bind(Initializer.ParameterMap, TEXT("CameraMatrix"));
        ImageSize.Bind(Initializer.ParameterMap, TEXT("ImageSize"));
        MaxAngle.Bind(Initializer.ParameterMap, TEXT("MaxAngle"));
    }
    FCubemapTexturePropertiesPSFisheye() {}

    void SetParameters(
        FRHICommandList& RHICmdList,
        const FTexture* Texture,
        const FMatrix& ColorWeightsValue,
        float MipLevel,
        float GammaValue,
        const FVector4& DistortionCoeffsValue,
        const FVector4& CameraMatrixValue,
        const FIntPoint& ImageSizeValue,
        float MaxAngleValue);

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("HDR_OUTPUT"), bHDROutput ? TEXT("1") : TEXT("0"));
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, CubeTexture);
    LAYOUT_FIELD(FShaderResourceParameter, CubeTextureSampler);
    LAYOUT_FIELD(FShaderParameter, PackedProperties0);
    LAYOUT_FIELD(FShaderParameter, ColorWeights);
    LAYOUT_FIELD(FShaderParameter, Gamma);
    LAYOUT_FIELD(FShaderParameter, DistortionCoeffs);
    LAYOUT_FIELD(FShaderParameter, CameraMatrix);
    LAYOUT_FIELD(FShaderParameter, ImageSize);
    LAYOUT_FIELD(FShaderParameter, MaxAngle);
};

class ENGINE_API FMipLevelBatchedElementParametersFisheye : public FBatchedElementParameters
{
public:
    FMipLevelBatchedElementParametersFisheye(float InMipLevel,
                                             FVector4& InDistortionCoeffs,
                                             FVector4& InCameraMatrix,
                                             FIntPoint& InImageSize,
                                             float InMaxAngle,
                                             bool bInHDROutput = false)
        : bHDROutput(bInHDROutput)
        , MipLevel(InMipLevel)
        , DistortionCoeffs(InDistortionCoeffs)
        , CameraMatrix(InCameraMatrix)
        , ImageSize(InImageSize)
        , MaxAngle(InMaxAngle)
    {}

    virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
    template<typename TPixelShader>
    void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture);

    bool bHDROutput;
    float MipLevel;
    FVector4 DistortionCoeffs;
    FVector4 CameraMatrix;
    FIntPoint ImageSize;
    float MaxAngle;
};

