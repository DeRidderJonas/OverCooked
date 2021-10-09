//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "DiffuseMaterial_Shadow.h"
#include "GeneralStructs.h"
#include "Logger.h"
#include "ContentManager.h"
#include "TextureData.h"
#include "Components.h"

ID3DX11EffectShaderResourceVariable* DiffuseMaterial_Shadow::m_pDiffuseSRVvariable = nullptr;
ID3DX11EffectShaderResourceVariable* DiffuseMaterial_Shadow::m_pShadowSRVvariable = nullptr;
ID3DX11EffectVectorVariable* DiffuseMaterial_Shadow::m_pLightDirectionVariable = nullptr;
ID3DX11EffectMatrixVariable* DiffuseMaterial_Shadow::m_pLightWVPvariable = nullptr;

DiffuseMaterial_Shadow::DiffuseMaterial_Shadow() : Material(L"./Resources/Effects/Shadow/PosNormTex3D_Shadow.fx"),
	m_pDiffuseTexture(nullptr)
{}

void DiffuseMaterial_Shadow::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
}

void DiffuseMaterial_Shadow::SetLightDirection(const DirectX::XMFLOAT3& dir)
{
	m_LightDirection = dir;
}

void DiffuseMaterial_Shadow::LoadEffectVariables()
{
	//load all the necessary shader variables
	if(!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if(!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariable() > \'gDiffuseMap\' variable not found");
			m_pDiffuseSRVvariable = nullptr;
		}
	}

	if(!m_pShadowSRVvariable)
	{
		m_pShadowSRVvariable = GetEffect()->GetVariableByName("gShadowMap")->AsShaderResource();
		if(!m_pShadowSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariable() > \'gShadowMap\' variable not found");
			m_pShadowSRVvariable = nullptr;
		}
	}

	if(!m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
		if(!m_pLightDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariable() > \'gLightDirection\' variable not found");
			m_pLightDirectionVariable = nullptr;
		}
	}

	if(!m_pLightWVPvariable)
	{
		m_pLightWVPvariable = GetEffect()->GetVariableByName("gWorldViewProj_Light")->AsMatrix();
		if(!m_pLightWVPvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariable() > \'gWorldViewProj_Light\' variable not found");
			m_pLightWVPvariable = nullptr;
		}
	}
}

void DiffuseMaterial_Shadow::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	//update all the necessary shader variables

	if(m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable->SetResource(m_pDiffuseTexture->GetShaderResourceView());
	}

	auto pShadowMap = gameContext.pShadowMapper->GetShadowMap();
	if(m_pShadowSRVvariable && pShadowMap)
	{
		m_pShadowSRVvariable->SetResource(pShadowMap);
	}

	if(m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
	}

	if(m_pLightWVPvariable)
	{
		using namespace DirectX;
		
		auto world = DirectX::XMLoadFloat4x4(&pModelComponent->GetTransform()->GetWorld());
		auto lightVP = DirectX::XMLoadFloat4x4(&gameContext.pShadowMapper->GetLightVP());

		auto Lwvp = world * lightVP;
		
		m_pLightWVPvariable->SetMatrix(reinterpret_cast<float*>(&Lwvp));
	}
}