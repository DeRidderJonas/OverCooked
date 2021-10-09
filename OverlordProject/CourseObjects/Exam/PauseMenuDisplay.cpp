#include "stdafx.h"
#include "PauseMenuDisplay.h"

#include "GameScene.h"
#include "KitchenScene.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "SpriteComponent.h"

PauseMenuDisplay::PauseMenuDisplay(bool* pIsPaused)
	: m_pIsPaused(pIsPaused)
	, m_PausedLastFrame(false)
{
}

PauseMenuDisplay::~PauseMenuDisplay()
{
	SafeDelete(m_pSpriteComponent);
}

void PauseMenuDisplay::Initialize(const GameContext&)
{
	m_pSpriteComponent = new SpriteComponent(L"./Resources/Textures/IngameMenu.png");

	auto pFmod = SoundManager::GetInstance()->GetSystem();

	auto res = pFmod->createSound("./Resources/Sounds/MenuSelect.wav", FMOD_DEFAULT, 0, &m_pSelect);
	SoundManager::GetInstance()->ErrorCheck(res);

	res = pFmod->createSound("./Resources/Sounds/MenuConfirm.wav", FMOD_DEFAULT, 0, &m_pConfirm);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void PauseMenuDisplay::Draw(const GameContext&)
{
}

void PauseMenuDisplay::Update(const GameContext& gameContext)
{
	if (!m_PausedLastFrame && *m_pIsPaused)
	{
		OnPaused();
	}

	m_PausedLastFrame = *m_pIsPaused;
	if (!*m_pIsPaused)
		return;

	if(gameContext.pInput->IsActionTriggered(Input::EscapeMenu))
	{
		OnUnPaused();
		return;
	}

	if (gameContext.pInput->IsActionTriggered(Input::PausedMenuUp))
		MenuGoUp();

	if (gameContext.pInput->IsActionTriggered(Input::PausedMenuDown))
		MenuGoDown();

	if(gameContext.pInput->IsActionTriggered(Input::PausedMenuSelect))
	{
		OnUnPaused();
		
		switch(m_Selected)
		{
		case Button::MainMenu: 
			ToMainMenu();
			break;
		case Button::Restart: 
			Restart();
			break;
		case Button::Exit: 
			ExitGame();
			break;
		case Button::None:
		default: ;
		}
	}

	if(gameContext.pInput->IsActionTriggered(Input::PausedMenuClick))
	{
		auto mousePos = gameContext.pInput->GetMousePosition();
		bool xCorrect{ mousePos.x >= m_ButtonXValues.x && mousePos.y <= m_ButtonXValues.y };
		if (xCorrect)
		{
			OnUnPaused();
			
			if (mousePos.y >= m_MainYValues.x && mousePos.y <= m_MainYValues.y)
				ToMainMenu();
			else if (mousePos.y >= m_RestartYValues.x && mousePos.y <= m_RestartYValues.y)
				Restart();
			else if (mousePos.y >= m_ExitYValues.x && mousePos.y <= m_ExitYValues.y)
				ExitGame();
			}
	}

	auto leftThumb = gameContext.pInput->GetThumbstickPosition();
	m_LeftThumbInputCooldownRemaining -= gameContext.pGameTime->GetElapsed();
	if (!(DirectX::XMScalarNearEqual(leftThumb.x, 0.f, 0.1f) && DirectX::XMScalarNearEqual(leftThumb.y, 0.f, 0.1f)) && m_LeftThumbInputCooldownRemaining <= 0.f)
	{
		m_LeftThumbInputCooldownRemaining = m_LeftThumbInputCooldown;
		if (leftThumb.y > 0.f)
			MenuGoUp();
		else if (leftThumb.y < 0.f)
			MenuGoDown();
	}

	auto mouseMovement{ gameContext.pInput->GetMouseMovement() };
	if (!(DirectX::XMScalarNearEqual(static_cast<float>(mouseMovement.x), 0.f, 0.1f) && DirectX::XMScalarNearEqual(static_cast<float>(mouseMovement.y), 0.f, 0.1f)))
	{
		auto mousePos = gameContext.pInput->GetMousePosition();
		bool xCorrect{ mousePos.x >= m_ButtonXValues.x && mousePos.y <= m_ButtonXValues.y };

		if (!xCorrect)
			m_Selected = Button::None;
		else if (mousePos.y >= m_MainYValues.x && mousePos.y <= m_MainYValues.y)
			m_Selected = Button::MainMenu;
		else if (mousePos.y >= m_RestartYValues.x && mousePos.y <= m_RestartYValues.y)
			m_Selected = Button::Restart;
		else if (mousePos.y >= m_ExitYValues.x && mousePos.y <= m_ExitYValues.y)
			m_Selected = Button::Exit;
	}

	switch(m_Selected)
	{
	case Button::MainMenu: 
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/IngameMenu_MainMenu.png");
		break;
	case Button::Restart:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/IngameMenu_Restart.png");
		break;
	case Button::Exit:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/IngameMenu_Exit.png");
		break;
	case Button::None:
	default:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/IngameMenu.png");
		break;
	}
}

void PauseMenuDisplay::OnPaused()
{
	m_Selected = Button::MainMenu;

	GetScene()->GetGameContext().pInput->ClearInputActions();
	SetupInput();

	AddComponent(m_pSpriteComponent);
}

void PauseMenuDisplay::OnUnPaused()
{
	auto pKitchenScene = dynamic_cast<KitchenScene*>(GetScene());
	if(pKitchenScene)
	{
		pKitchenScene->OnUnpaused();
		RemoveComponent(m_pSpriteComponent);
	}
}

void PauseMenuDisplay::SetupInput()
{
	auto& gameContext = GetScene()->GetGameContext();

	gameContext.pInput->AddInputAction({ Input::PausedMenuUp, InputTriggerState::Pressed, 'W' });
	gameContext.pInput->AddInputAction({ Input::PausedMenuDown, InputTriggerState::Pressed, 'S' });
	gameContext.pInput->AddInputAction({ Input::PausedMenuSelect, InputTriggerState::Pressed, 'E', -1, 0x1000 });
	gameContext.pInput->AddInputAction({ Input::PausedMenuClick, InputTriggerState::Pressed, -1, 0x01 });
	gameContext.pInput->AddInputAction({ Input::EscapeMenu, InputTriggerState::Pressed, VK_ESCAPE, -1, 0x0010});
}

void PauseMenuDisplay::MenuGoUp()
{
	switch (m_Selected)
	{
	case Button::MainMenu: 
		m_Selected = Button::Exit;
		break;
	case Button::Exit: 
		m_Selected = Button::Restart;
		break;
	case Button::Restart: 
	case Button::None:
	default:
		m_Selected = Button::MainMenu;
		break;
	}

	if(m_pSelect)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pSelect, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void PauseMenuDisplay::MenuGoDown()
{
	switch (m_Selected)
	{
	case Button::MainMenu: 
		m_Selected = Button::Restart;
		break;
	case Button::Exit: 
		m_Selected = Button::MainMenu;
		break;
	case Button::Restart: 
	case Button::None: 
	default:
		m_Selected = Button::Exit;
		break;
	}

	if (m_pSelect)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pSelect, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void PauseMenuDisplay::ToMainMenu()
{
	SceneManager::GetInstance()->SetActiveGameScene(L"MainMenuScene");

	if (m_pConfirm)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pConfirm, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void PauseMenuDisplay::Restart()
{
	auto pKitchenScene = dynamic_cast<KitchenScene*>(GetScene());
	if(pKitchenScene)
	{
		if (m_pConfirm)
		{
			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pConfirm, 0, false, &m_pChannel);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
	}

	SceneManager::GetInstance()->SetActiveGameScene(L"ControlsSchemaScene");
}

void PauseMenuDisplay::ExitGame()
{
	PostQuitMessage(0);
}
