#include "stdafx.h"
#include "ScoreScene.h"

#include "ContentManager.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "SpriteComponent.h"
#include "SpriteFont.h"
#include "TextRenderer.h"
#include "TransformComponent.h"

ScoreScene::ScoreScene()
	: GameScene(L"ScoreScene")
	, m_Selected(Back)
	, m_LeftThumbInputCooldownRemaining(0)
	, m_Score(0)
{
}

void ScoreScene::SetScore(int score)
{
	m_Score = score;
}

void ScoreScene::Initialize()
{
	auto pBackground = new GameObject();
	m_pSpriteComponent = new SpriteComponent(L"./Resources/Textures/ScoreMenu.png");
	pBackground->AddComponent(m_pSpriteComponent);
	pBackground->GetTransform()->Translate(0, 0, 0.95f);

	AddChild(pBackground);

	auto pFmod = SoundManager::GetInstance()->GetSystem();

	auto res = pFmod->createSound("./Resources/Sounds/MenuSelect.wav", FMOD_DEFAULT, 0, &m_pSelect);
	SoundManager::GetInstance()->ErrorCheck(res);

	res = pFmod->createSound("./Resources/Sounds/MenuConfirm.wav", FMOD_DEFAULT, 0, &m_pConfirm);
	SoundManager::GetInstance()->ErrorCheck(res);

	res = pFmod->createSound("./Resources/Sounds/MenuBackgroundMusic.mp3", FMOD_LOOP_NORMAL, 0, &m_pBackground);
	SoundManager::GetInstance()->ErrorCheck(res);
	res = pFmod->playSound(m_pBackground, 0, true, &m_pBackgroundChannel);
	SoundManager::GetInstance()->ErrorCheck(res);

	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Consolas_50.fnt");
}

void ScoreScene::Update()
{
	auto& gameContext{ GetGameContext() };

	if (gameContext.pInput->IsActionTriggered(Input::MenuItemLeft))
		MenuGoLeft();

	if (gameContext.pInput->IsActionTriggered(Input::MenuItemRight))
		MenuGoRight();

	if(gameContext.pInput->IsActionTriggered(Input::SelectMenuItem))
	{
		switch(m_Selected)
		{
		case Back:
			GoBack();
			break;
		case Exit:
			ExitGame();
			break;
		case None: 
		default: ;
		}
	}

	if(gameContext.pInput->IsActionTriggered(Input::ClickMenuItem))
	{
		auto mousePos = gameContext.pInput->GetMousePosition();
		bool yCorrect{ mousePos.y >= m_ButtonYValues.x && mousePos.y <= m_ButtonYValues.y };
		if (!yCorrect)
		{
		}

		else if (mousePos.x >= m_BackXValues.x && mousePos.x <= m_BackXValues.y)
			GoBack();
		else if (mousePos.x >= m_ExitXValues.x && mousePos.x <= m_ExitXValues.y)
			ExitGame();
		
	}

	auto leftThumb = gameContext.pInput->GetThumbstickPosition();
	m_LeftThumbInputCooldownRemaining -= gameContext.pGameTime->GetElapsed();
	if (!(DirectX::XMScalarNearEqual(leftThumb.x, 0.f, 0.1f) && DirectX::XMScalarNearEqual(leftThumb.y, 0.f, 0.1f)) && m_LeftThumbInputCooldownRemaining <= 0.f)
	{
		m_LeftThumbInputCooldownRemaining = m_LeftThumbInputCooldown;
		if (leftThumb.x > 0.f)
			MenuGoRight();
		else if (leftThumb.x < 0.f)
			MenuGoLeft();
	}

	auto mouseMovement{ gameContext.pInput->GetMouseMovement() };
	if (!(DirectX::XMScalarNearEqual(static_cast<float>(mouseMovement.x), 0.f, 0.1f) && DirectX::XMScalarNearEqual(static_cast<float>(mouseMovement.y), 0.f, 0.1f)))
	{
		auto mousePos = gameContext.pInput->GetMousePosition();
		bool yCorrect{ mousePos.y >= m_ButtonYValues.x && mousePos.y <= m_ButtonYValues.y };
		if (!yCorrect)
			m_Selected = None;
		else if (mousePos.x >= m_BackXValues.x && mousePos.x <= m_BackXValues.y)
			m_Selected = Back;
		else if (mousePos.x >= m_ExitXValues.x && mousePos.x <= m_ExitXValues.y)
			m_Selected = Exit;
	}

	switch(m_Selected)
	{
	case Back: 
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/ScoreMenu_Back.png");
		break;
	case Exit:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/ScoreMenu_Exit.png");
		break;
	case None:
	default:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/ScoreMenu.png");
		break;
	}
}

void ScoreScene::Draw()
{
	if (!m_pFont->GetFontName().empty())
	{
		TextRenderer::GetInstance()->DrawText(m_pFont, std::to_wstring(m_Score), { m_ScorePosition.x, m_ScorePosition.y }, static_cast<DirectX::XMFLOAT4>(DirectX::Colors::Gold));
	}
}

void ScoreScene::SceneActivated()
{
	m_Selected = Back;

	auto& gameContext{ GetGameContext() };

	gameContext.pInput->AddInputAction({ Input::MenuItemLeft, InputTriggerState::Pressed, 'A' });
	gameContext.pInput->AddInputAction({ Input::MenuItemRight, InputTriggerState::Pressed, 'D' });
	gameContext.pInput->AddInputAction({ Input::SelectMenuItem, InputTriggerState::Pressed, 'E', -1, 0x1000 });
	gameContext.pInput->AddInputAction({ Input::ClickMenuItem, InputTriggerState::Pressed, -1, 0x01 });

	if (m_pBackgroundChannel)
	{
		m_pBackgroundChannel->setPaused(false);
	}
}

void ScoreScene::SceneDeactivated()
{
	auto& gameContext{ GetGameContext() };
	gameContext.pInput->ClearInputActions();

	if (m_pBackgroundChannel)
	{
		m_pBackgroundChannel->setPaused(true);
	}
}

void ScoreScene::MenuGoLeft()
{
	switch(m_Selected)
	{
	case Back:
		m_Selected = Exit;
		break;
	case Exit:
	case None:
	default:
		m_Selected = Back;
		break;
	}

	if (m_pSelect)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pSelect, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void ScoreScene::MenuGoRight()
{
	switch(m_Selected)
	{
	case Exit: 
		m_Selected = Back;
		break;
	case Back: 
	case None: 
	default:
		m_Selected = Exit;
		break;
	}

	if (m_pSelect)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pSelect, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void ScoreScene::GoBack()
{
	SceneManager::GetInstance()->SetActiveGameScene(L"MainMenuScene");

	if (m_pConfirm)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pConfirm, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void ScoreScene::ExitGame()
{
	PostQuitMessage(0);
}
