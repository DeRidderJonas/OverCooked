#include "stdafx.h"
#include "ScoreDisplay.h"

#include "ContentManager.h"
#include "SpriteComponent.h"
#include "SpriteFont.h"
#include "TextRenderer.h"
#include "TransformComponent.h"

ScoreDisplay::ScoreDisplay(int* pScore)
	: m_pScore(pScore)
{
}

void ScoreDisplay::Initialize(const GameContext&)
{
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Consolas_32.fnt");
	
	AddComponent(new SpriteComponent(L"./Resources/Textures/goldCoin.png"));
}

void ScoreDisplay::Draw(const GameContext&)
{
	if (!m_pFont->GetFontName().empty())
	{
		auto pos = GetTransform()->GetPosition();
		TextRenderer::GetInstance()->DrawText(m_pFont, std::to_wstring(*m_pScore), { pos.x + m_numberOffset.x, pos.y + m_numberOffset.y }, static_cast<DirectX::XMFLOAT4>(DirectX::Colors::Black));
	}
}

void ScoreDisplay::PostDraw(const GameContext&)
{
}

void ScoreDisplay::Update(const GameContext&)
{
}
