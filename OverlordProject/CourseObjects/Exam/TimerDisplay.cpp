#include "stdafx.h"
#include "TimerDisplay.h"

#include "ContentManager.h"
#include "SpriteComponent.h"
#include "TextRenderer.h"
#include "SpriteFont.h"
#include "TransformComponent.h"

TimerDisplay::TimerDisplay(float* pTimer)
	: m_pTimer{pTimer}
{
}

void TimerDisplay::Initialize(const GameContext&)
{
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Consolas_32.fnt");

	AddComponent(new SpriteComponent(L"./Resources/Textures/stopwatch.png"));
}

void TimerDisplay::Draw(const GameContext&)
{
	if(!m_pFont->GetFontName().empty())
	{
		int mins{ static_cast<int>(*m_pTimer) / 60 };
		int sec{ static_cast<int>(*m_pTimer) % 60 };

		wchar_t buffer[6]{};
		std::swprintf(buffer, 6, L"%.2d:%.2d", mins, sec);

		auto pos = GetTransform()->GetPosition();
		TextRenderer::GetInstance()->DrawText(m_pFont, buffer, { pos.x + m_TimerOffset.x, pos.y + m_TimerOffset.y }, static_cast<DirectX::XMFLOAT4>(DirectX::Colors::Brown));
	}
}
