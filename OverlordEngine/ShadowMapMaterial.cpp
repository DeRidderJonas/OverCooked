//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ShadowMapMaterial.h"
#include "ContentManager.h"

ShadowMapMaterial::~ShadowMapMaterial()
{
	SafeRelease(m_pInputLayouts[0]);
	SafeRelease(m_pInputLayouts[1]);
	m_InputLayoutDescriptions[0].clear();
	m_InputLayoutDescriptions[1].clear();
}

void ShadowMapMaterial::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		//initialize the effect, techniques, shader variables, input layouts (hint use EffectHelper::BuildInputLayout), etc.
		m_pShadowEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ShadowMapGenerator.fx");

		m_pShadowTechs[0] = m_pShadowEffect->GetTechniqueByName("GenerateShadows");
		m_pShadowTechs[1] = m_pShadowEffect->GetTechniqueByName("GenerateShadows_Skinned");

		
		EffectHelper::BuildInputLayout(gameContext.pDevice, m_pShadowTechs[0], &m_pInputLayouts[0], m_InputLayoutDescriptions[0], m_InputLayoutSizes[0], m_InputLayoutIds[0]);
		EffectHelper::BuildInputLayout(gameContext.pDevice, m_pShadowTechs[1], &m_pInputLayouts[1], m_InputLayoutDescriptions[1], m_InputLayoutSizes[1], m_InputLayoutIds[1]);

		auto effectVar = m_pShadowEffect->GetVariableByName("gWorld");
		m_pWorldMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		effectVar = m_pShadowEffect->GetVariableByName("gLightViewProj");
		m_pLightVPMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		effectVar = m_pShadowEffect->GetVariableByName("gBones");
		m_pBoneTransforms = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		
		
	}
}

void ShadowMapMaterial::SetLightVP(DirectX::XMFLOAT4X4 lightVP) const
{
	if (!m_pLightVPMatrixVariable) return;
	
	auto matrix = DirectX::XMLoadFloat4x4(&lightVP);
	m_pLightVPMatrixVariable->SetMatrix(reinterpret_cast<float*>(&matrix));
}

void ShadowMapMaterial::SetWorld(DirectX::XMFLOAT4X4 world) const
{
	if (!m_pWorldMatrixVariable) return;

	auto matrix = DirectX::XMLoadFloat4x4(&world);
	m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<float*>(&matrix));
}

void ShadowMapMaterial::SetBones(const float* pData, int count) const
{
	if (!m_pBoneTransforms) return;

	m_pBoneTransforms->SetMatrixArray(pData, 0, count);
}
