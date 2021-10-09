#include "stdafx.h"
#include "ProgressBar.h"

#include "SpriteComponent.h"
#include "TransformComponent.h"

ProgressBar::ProgressBar()
	: m_Percentage(1.f)
	, m_pSpriteComponent(nullptr)
{
}

void ProgressBar::SetPercentage(float percentage)
{
	m_Percentage = percentage;

	float currScale{ GetTransform()->GetScale().x };

	m_pSpriteComponent->GetTransform()->Translate(m_BarOffset.x, currScale * m_BarOffset.y, 0.9f);
	m_pSpriteComponent->GetTransform()->Scale(currScale * percentage, 1, 1);
}

void ProgressBar::SetBarColor(bool isGreen)
{
	if (isGreen)
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/Progress_Green.png");
	else
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/Progress_Red.png");
}

void ProgressBar::Initialize(const GameContext&)
{
	AddComponent(new SpriteComponent(L"./Resources/Textures/Progress_Foreground.png"));
	m_pChild = new GameObject();
	m_pSpriteComponent = new SpriteComponent(L"./Resources/Textures/Progress_Green.png");
	m_pChild->AddComponent(m_pSpriteComponent);
	AddChild(m_pChild);
}

void ProgressBar::Draw(const GameContext&)
{
}

void ProgressBar::Update(const GameContext&)
{
}

void ProgressBar::PostInitialize(const GameContext&)
{
	m_pChild->GetTransform()->Translate(m_BarOffset.x, m_BarOffset.y, 0.9f);
	GetTransform()->Translate(0, 0, 0.95f);
}
