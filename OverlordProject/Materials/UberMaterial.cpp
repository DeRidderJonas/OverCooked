#include "stdafx.h"
#include "UberMaterial.h"

#include "ContentManager.h"
#include "TextureData.h"

ID3DX11EffectVectorVariable* UberMaterial::m_pLightDirectionVariable{ nullptr };

ID3DX11EffectScalarVariable* UberMaterial::m_pUseDiffuseTextureVariable{ nullptr };
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pDiffuseSRVvariable{ nullptr };
ID3DX11EffectVectorVariable* UberMaterial::m_pDiffuseColorVariable{ nullptr };

ID3DX11EffectVectorVariable* UberMaterial::m_pSpecularColorVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pUseSpecularLevelTextureVariable{ nullptr };
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pSpecularLevelSRVvariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pShininessVariable{ nullptr };

ID3DX11EffectVectorVariable* UberMaterial::m_pAmbientColorVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pAmbientIntensityVariable{ nullptr };

ID3DX11EffectScalarVariable* UberMaterial::m_pFlipGreenChannelVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pUseNormalMappingVariable{ nullptr };
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pNormalMappingSRVvariable{ nullptr };

ID3DX11EffectScalarVariable* UberMaterial::m_pUseEnvironmentMappingVariable{ nullptr };
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pEnvironmentSRVvariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pReflectionStrengthVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pRefractionStrengthVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pRefractionIndexVariable{ nullptr };

ID3DX11EffectScalarVariable* UberMaterial::m_pOpacityVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pUseOpacityMapVariable{ nullptr };
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pOpacitySRVvariable{ nullptr };

ID3DX11EffectScalarVariable* UberMaterial::m_pUseBlinnVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pUsePhongVariable{ nullptr };

ID3DX11EffectScalarVariable* UberMaterial::m_pUseFresnelFalloffVariable{ nullptr };
ID3DX11EffectVectorVariable* UberMaterial::m_pFresnelColorVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelPowerVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelMultiplierVariable{ nullptr };
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelHardnessVariable{ nullptr };


UberMaterial::UberMaterial()
	: Material(L"Resources/Effects/GP2_W3B_UberShader_Jonas_DeRidder_2GD07N.fx")
	, m_LightDirection(-0.577f, -0.577f, 0.577f)
	, m_bDiffuseTexture(false)
	, m_pDiffuseTexture(nullptr)
	, m_ColorDiffuse(0.3f,0.3f,0.3f,1)
	, m_ColorSpecular(1,1,1,1)
	, m_bSpecularLevelTexture(false)
	, m_pSpecularLevelTexture(nullptr)
	, m_Shininess(15)
	, m_ColorAmbient(0.8f,0.8f,0.8f,1)
	, m_AmbientIntensity(0)
	, m_bFlipGreenChannel(false)
	, m_bNormalMapping(false)
	, m_pNormalMappingTexture(nullptr)
	, m_bEnvironmentMapping(false)
	, m_pEnvironmentCube(nullptr)
	, m_ReflectionStrength(0.f)
	, m_RefractionStrength(0.f)
	, m_RefractionIndex(0.3f)
	, m_Opacity(1)
	, m_bOpacityMap(false)
	, m_pOpacityMap(nullptr)
	, m_bSpecularBlinn(false)
	, m_bSpecularPhong(false)
	, m_bFresnelFaloff(false)
	, m_FresnelMultiplier(1)
	, m_ColorFresnel(1,1,1,1)
	, m_FresnelPower(0)
	, m_FresnelHardness(0)
{
}

void UberMaterial::SetLightDirection(const DirectX::XMFLOAT3& direction)
{
	m_LightDirection = direction;
}

void UberMaterial::EnableDiffuseTexture(bool enable)
{
	m_bDiffuseTexture = enable;
}

void UberMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::SetDiffuseColor(const DirectX::XMFLOAT4& color)
{
	m_ColorDiffuse = color;
}

void UberMaterial::SetSpecularColor(const DirectX::XMFLOAT4& color)
{
	m_ColorSpecular = color;
}

void UberMaterial::EnableSpecularLevelTexture(bool enable)
{
	m_bSpecularLevelTexture = enable;
}

void UberMaterial::SetSpecularLevelTexture(const std::wstring& assetFile)
{
	m_pSpecularLevelTexture = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::SetShininess(int shininess)
{
	m_Shininess = shininess;
}

void UberMaterial::SetAmbientColor(const DirectX::XMFLOAT4& color)
{
	m_ColorAmbient = color;
}

void UberMaterial::SetAmbientIntensity(float intensity)
{
	m_AmbientIntensity = intensity;
}

void UberMaterial::FlipNormalGreenCHannel(bool flip)
{
	m_bFlipGreenChannel = flip;
}

void UberMaterial::EnableNormalMapping(bool enable)
{
	m_bNormalMapping = enable;
}

void UberMaterial::SetNormalMapTexture(const std::wstring& assetFile)
{
	m_pNormalMappingTexture = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::EnableEnvironmentMapping(bool enable)
{
	m_bEnvironmentMapping = enable;
}

void UberMaterial::SetEnvironmentCube(const std::wstring& assetFile)
{
	m_pEnvironmentCube = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::SetReflectionStrength(float strength)
{
	m_ReflectionStrength = strength;
}

void UberMaterial::SetRefractionStrength(float strength)
{
	m_RefractionStrength = strength;
}

void UberMaterial::SetRefractionIndex(float index)
{
	m_RefractionIndex = index;
}

void UberMaterial::SetOpacity(float opacity)
{
	m_Opacity = opacity;
}

void UberMaterial::EnableOpacityMap(bool enable)
{
	m_bOpacityMap = enable;
}

void UberMaterial::SetOpacityTexture(const std::wstring& assetFile)
{
	m_pOpacityMap = ContentManager::Load<TextureData>(assetFile);
}

void UberMaterial::EnableSpecularBlinn(bool enable)
{
	m_bSpecularBlinn = enable;
}

void UberMaterial::EnableSpecularPhong(bool enable)
{
	m_bSpecularPhong = enable;
}

void UberMaterial::EnableFresnelFaloff(bool enable)
{
	m_bFresnelFaloff = enable;
}

void UberMaterial::SetFresnelColor(const DirectX::XMFLOAT4& color)
{
	m_ColorFresnel = color;
}

void UberMaterial::SetFresnelPower(float power)
{
	m_FresnelPower = power;
}

void UberMaterial::SetFresnelMultiplier(float multiplier)
{
	m_FresnelMultiplier = multiplier;
}

void UberMaterial::SetFresnelHardness(float hardness)
{
	m_FresnelHardness = hardness;
}

void UberMaterial::LoadEffectVariables()
{
	InitVectorVariable(&m_pLightDirectionVariable, "gLightDirection");

	InitScalarVariable(&m_pUseDiffuseTextureVariable, "gUseTextureDiffuse");
	InitTextureVariable(&m_pDiffuseSRVvariable, "gTextureDiffuse");
	InitVectorVariable(&m_pDiffuseColorVariable, "gColorDiffuse");

	InitVectorVariable(&m_pSpecularColorVariable, "gColorSpecular");
	InitScalarVariable(&m_pUseSpecularLevelTextureVariable, "gUseTextureSpecularIntensity");
	InitTextureVariable(&m_pSpecularLevelSRVvariable, "gTextureSpecularIntensity");
	InitScalarVariable(&m_pShininessVariable, "gShininess");

	InitVectorVariable(&m_pAmbientColorVariable, "gColorAmbient");
	InitScalarVariable(&m_pAmbientIntensityVariable, "gAmbientIntensity");

	InitScalarVariable(&m_pFlipGreenChannelVariable, "gFlipGreenChannel");
	InitScalarVariable(&m_pUseNormalMappingVariable, "gUseTextureNormal");
	InitTextureVariable(&m_pNormalMappingSRVvariable, "gTextureNormal");

	InitScalarVariable(&m_pUseEnvironmentMappingVariable, "gUseEnvironmentMapping");
	InitTextureVariable(&m_pEnvironmentSRVvariable, "gCubeEnvironment");
	InitScalarVariable(&m_pReflectionStrengthVariable, "gReflectionStrength");
	InitScalarVariable(&m_pRefractionStrengthVariable, "gRefractionStrength");
	InitScalarVariable(&m_pRefractionIndexVariable, "gRefractionIndex");

	InitScalarVariable(&m_pOpacityVariable, "gOpacityIntensity");
	InitScalarVariable(&m_pUseOpacityMapVariable, "gTextureOpacityIntensity");
	InitTextureVariable(&m_pOpacitySRVvariable, "gTextureOpacity");

	InitScalarVariable(&m_pUseBlinnVariable, "gUseSpecularBlinn");
	InitScalarVariable(&m_pUsePhongVariable, "gUseSpecularPhong");

	InitScalarVariable(&m_pUseFresnelFalloffVariable, "gUseFresnelFalloff");
	InitVectorVariable(&m_pFresnelColorVariable, "gColorFresnel");
	InitScalarVariable(&m_pFresnelPowerVariable, "gFresnelPower");
	InitScalarVariable(&m_pFresnelMultiplierVariable, "gFresnelMultiplier");
	InitScalarVariable(&m_pFresnelHardnessVariable, "gFresnelHardness");
}

void UberMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	if (m_pLightDirectionVariable) m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
	
	if (m_pUseDiffuseTextureVariable) m_pUseDiffuseTextureVariable->SetBool(m_bDiffuseTexture);
	if (m_pDiffuseSRVvariable && m_pDiffuseTexture) m_pDiffuseSRVvariable->SetResource(m_pDiffuseTexture->GetShaderResourceView());
	if (m_pDiffuseColorVariable) m_pDiffuseColorVariable->SetFloatVector(&m_ColorDiffuse.x);

	if (m_pSpecularColorVariable) m_pSpecularColorVariable->SetFloatVector(&m_ColorSpecular.x);
	if (m_pUseSpecularLevelTextureVariable) m_pUseSpecularLevelTextureVariable->SetBool(m_bSpecularLevelTexture);
	if (m_pSpecularLevelSRVvariable && m_pSpecularLevelTexture) m_pSpecularLevelSRVvariable->SetResource(m_pSpecularLevelTexture->GetShaderResourceView());
	if (m_pShininessVariable) m_pShininessVariable->SetInt(m_Shininess);

	if (m_pAmbientColorVariable) m_pAmbientColorVariable->SetFloatVector(&m_ColorAmbient.x);
	if (m_pAmbientIntensityVariable) m_pAmbientIntensityVariable->SetFloat(m_AmbientIntensity);

	if (m_pFlipGreenChannelVariable) m_pFlipGreenChannelVariable->SetBool(m_bFlipGreenChannel);
	if (m_pUseNormalMappingVariable) m_pUseNormalMappingVariable->SetBool(m_bNormalMapping);
	if (m_pNormalMappingSRVvariable && m_pNormalMappingTexture) m_pNormalMappingSRVvariable->SetResource(m_pNormalMappingTexture->GetShaderResourceView());

	if (m_pUseEnvironmentMappingVariable) m_pUseEnvironmentMappingVariable->SetBool(m_bEnvironmentMapping);
	if (m_pEnvironmentSRVvariable && m_pEnvironmentCube) m_pEnvironmentSRVvariable->SetResource(m_pEnvironmentCube->GetShaderResourceView());
	if (m_pReflectionStrengthVariable) m_pReflectionStrengthVariable->SetFloat(m_ReflectionStrength);
	if (m_pRefractionStrengthVariable) m_pRefractionStrengthVariable->SetFloat(m_RefractionStrength);
	if (m_pRefractionIndexVariable)m_pRefractionIndexVariable->SetFloat(m_RefractionIndex);

	if (m_pOpacityVariable) m_pOpacityVariable->SetFloat(m_Opacity);
	if (m_pUseOpacityMapVariable) m_pUseOpacityMapVariable->SetBool(m_bOpacityMap);
	if (m_pOpacitySRVvariable && m_pOpacityMap) m_pOpacitySRVvariable->SetResource(m_pOpacityMap->GetShaderResourceView());

	if (m_pUseBlinnVariable) m_pUseBlinnVariable->SetBool(m_bSpecularBlinn);
	if (m_pUsePhongVariable) m_pUsePhongVariable->SetBool(m_bSpecularPhong);

	if (m_pUseFresnelFalloffVariable) m_pUseFresnelFalloffVariable->SetBool(m_bFresnelFaloff);
	if (m_pFresnelColorVariable) m_pFresnelColorVariable->SetFloatVector(&m_ColorFresnel.x);
	if (m_pFresnelPowerVariable) m_pFresnelPowerVariable->SetFloat(m_FresnelPower);
	if (m_pFresnelMultiplierVariable) m_pFresnelMultiplierVariable->SetFloat(m_FresnelMultiplier);
	if (m_pFresnelHardnessVariable) m_pFresnelHardnessVariable->SetFloat(m_FresnelHardness);
}

void UberMaterial::InitVectorVariable(ID3DX11EffectVectorVariable** vectorVariable, const std::string& variableName)
{
	if (!*vectorVariable)
	{
		*vectorVariable = GetEffect()->GetVariableByName(variableName.c_str())->AsVector();
		if (!(*vectorVariable)->IsValid())
		{
			Logger::LogWarning( L"UberMaterial::LoadEffectVariables > variable not found:");
			std::cout << variableName;
			*vectorVariable = nullptr;
		}
	}
}

void UberMaterial::InitScalarVariable(ID3DX11EffectScalarVariable** scalarVariable, const std::string& variableName)
{
	if (!*scalarVariable)
	{
		*scalarVariable = GetEffect()->GetVariableByName(variableName.c_str())->AsScalar();
		if (!(*scalarVariable)->IsValid())
		{
			Logger::LogWarning(L"UberMaterial::LoadEffectVariables > variable not found:");
			std::cout << variableName;
			*scalarVariable = nullptr;
		}
	}
}

void UberMaterial::InitTextureVariable(ID3DX11EffectShaderResourceVariable** textureVariable,
	const std::string& variableName)
{
	if (!*textureVariable)
	{
		*textureVariable = GetEffect()->GetVariableByName(variableName.c_str())->AsShaderResource();
		if (!(*textureVariable)->IsValid())
		{
			Logger::LogWarning(L"UberMaterial::LoadEffectVariables > variable not found:");
			std::cout << variableName;
			*textureVariable = nullptr;
		}
	}
}


