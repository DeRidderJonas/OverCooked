#include "stdafx.h"
#include "PostHighlight.h"

#include "RenderTarget.h"

PostHighlight::PostHighlight()
	: PostProcessingMaterial(L"./Resources/Effects/Post/Highlight.fx", 1)
	, m_pTextureMapVariabele(nullptr)
	, m_pTopLeft(nullptr)
	, m_pBottomRight(nullptr)
{
}

void PostHighlight::SetTopLeft(float x, float y)
{
	if (m_pTopLeft)
	{
		float topLeft[]{ x,y };
		m_pTopLeft->SetFloatVector(topLeft);
	}
}

void PostHighlight::SetBottomRight(float x, float y)
{
	if (m_pBottomRight)
	{
		float bottomRight[]{ x,y };
		m_pBottomRight->SetFloatVector(bottomRight);
	}
}

void PostHighlight::LoadEffectVariables()
{
	m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
	if(!m_pTextureMapVariabele->IsValid())
	{
		Logger::LogWarning(L"PostHightlight::LoadEffectVariables >> variable \'gTexture\' not found!");
		m_pTextureMapVariabele = nullptr;
	}
	
	m_pTopLeft = GetEffect()->GetVariableByName("gTopLeft")->AsVector();
	if(!m_pTopLeft->IsValid())
	{
		Logger::LogWarning(L"PostHightlight::LoadEffectVariables >> variable \'gTopLeft\' not found!");
		m_pTopLeft = nullptr;
	}

	m_pBottomRight = GetEffect()->GetVariableByName("gBottomRight")->AsVector();
	if(!m_pBottomRight->IsValid())
	{
		Logger::LogWarning(L"PostHighlight::LoadEffectVariables >> variable \'gBottomRight\' not found!");
		m_pBottomRight = nullptr;
	}
}

void PostHighlight::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	if(m_pTextureMapVariabele && pRendertarget)
	{
		m_pTextureMapVariabele->SetResource(pRendertarget->GetShaderResourceView());
	}	
}
