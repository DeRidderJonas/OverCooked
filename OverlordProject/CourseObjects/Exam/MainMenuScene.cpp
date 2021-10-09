#include "stdafx.h"
#include "MainMenuScene.h"

#include "GameObject.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "SpriteComponent.h"

MainMenuScene::MainMenuScene()
	: GameScene(L"MainMenuScene")
	, m_SelectedButton(Start)
	, m_lifetime(0.f)
	, m_LeftThumbInputCooldownRemaining(0)
{
}

void MainMenuScene::Initialize()
{
	auto pBackground = new GameObject();
	m_pSpriteComponent = new SpriteComponent(L"./Resources/Textures/Undercooked_MainMenu.png");
	pBackground->AddComponent(m_pSpriteComponent);
	
	AddChild(pBackground);

	auto pFmod = SoundManager::GetInstance()->GetSystem();

	auto res = pFmod->createSound("./Resources/Sounds/MenuSelect.wav", FMOD_DEFAULT, 0, &m_pSelect);
	SoundManager::GetInstance()->ErrorCheck(res);

	res = pFmod->createSound("./Resources/Sounds/MenuConfirm.wav", FMOD_DEFAULT, 0, &m_pConfirm);
	SoundManager::GetInstance()->ErrorCheck(res);

	res = pFmod->createSound("./Resources/Sounds/MenuBackgroundMusic.mp3", FMOD_LOOP_NORMAL, 0, &m_pBackground);
	SoundManager::GetInstance()->ErrorCheck(res);
	res = pFmod->playSound(m_pBackground, 0, false, &m_pBackgroundChannel);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void MainMenuScene::Update()
{
	auto& gameContext{ GetGameContext() };
	m_lifetime += gameContext.pGameTime->GetElapsed();
	
	if(gameContext.pInput->IsActionTriggered(Input::MenuItemGoLeft))
	{
		MenuGoLeft();
	}

	if(gameContext.pInput->IsActionTriggered(Input::MenuItemGoRight))
	{
		MenuGoRight();
	}
	
	if(gameContext.pInput->IsActionTriggered(Input::SelectMenuItem))
	{
		switch(m_SelectedButton)
		{
		case Start: 
			StartGame();
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
		bool yCorrect{ mousePos.y >= m_ButtonsYValues.x && mousePos.y <= m_ButtonsYValues.y };
		if (!yCorrect)
		{
		}

		else if (mousePos.x >= m_StartButtonXValues.x && mousePos.x <= m_StartButtonXValues.y)
			StartGame();
		else if (mousePos.x >= m_ExitButtonPosXValues.x && mousePos.x <= m_ExitButtonPosXValues.y)
			ExitGame();
	}

	auto leftThumb = gameContext.pInput->GetThumbstickPosition();
	m_LeftThumbInputCooldownRemaining -= gameContext.pGameTime->GetElapsed();
	if(!(DirectX::XMScalarNearEqual(leftThumb.x, 0.f, 0.1f) && DirectX::XMScalarNearEqual(leftThumb.y, 0.f, 0.1f)) && m_LeftThumbInputCooldownRemaining <= 0.f)
	{
		m_LeftThumbInputCooldownRemaining = m_LeftThumbInputCooldown;
		if (leftThumb.x > 0.f)
			MenuGoRight();
		else if (leftThumb.x < 0.f)
			MenuGoLeft();
	}
	
	auto mouseMovement{ gameContext.pInput->GetMouseMovement() };
	if(!(DirectX::XMScalarNearEqual(static_cast<float>(mouseMovement.x), 0.f, 0.1f) && DirectX::XMScalarNearEqual(static_cast<float>(mouseMovement.y), 0.f, 0.1f)))
	{
		auto mousePos = gameContext.pInput->GetMousePosition();
		bool yCorrect{ mousePos.y >= 500.f && mousePos.y <= 580.f };
		if (!yCorrect)
			m_SelectedButton = None;
		else if (mousePos.x >= m_StartButtonXValues.x && mousePos.x <= m_StartButtonXValues.y)
			m_SelectedButton = Start;
		else if (mousePos.x >= m_ExitButtonPosXValues.x && mousePos.x <= m_ExitButtonPosXValues.y)
			m_SelectedButton = Exit;
	}

	if(m_lifetime <= .5f)
		m_SelectedButton = Start;

	switch(m_SelectedButton)
	{
	case Start:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/Undercooked_MainMenu_Start.png");
		break;
	case Exit:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/Undercooked_MainMenu_Exit.png");
		break;
	case None: 
	default:
		m_pSpriteComponent->SetTexture(L"./Resources/Textures/Undercooked_MainMenu.png");
	}
}

void MainMenuScene::Draw()
{
}

void MainMenuScene::SceneActivated()
{
	m_SelectedButton = Start;

	auto& gameContext{ GetGameContext() };
	
	gameContext.pInput->AddInputAction(InputAction{ Input::MenuItemGoLeft, InputTriggerState::Pressed, 'A' });
	gameContext.pInput->AddInputAction(InputAction{ Input::MenuItemGoRight, InputTriggerState::Pressed, 'D' });
	gameContext.pInput->AddInputAction(InputAction{ Input::SelectMenuItem, InputTriggerState::Pressed, 'E', -1, 0x1000 });
	gameContext.pInput->AddInputAction(InputAction{ Input::ClickMenuItem, InputTriggerState::Pressed, -1, 0x01 });

	if(m_pBackgroundChannel)
	{
		m_pBackgroundChannel->setPaused(false);
	}
}

void MainMenuScene::SceneDeactivated()
{
	auto& gameContext{ GetGameContext() };
	gameContext.pInput->ClearInputActions();

	if(m_pBackgroundChannel)
	{
		m_pBackgroundChannel->setPaused(true);
	}
}

void MainMenuScene::MenuGoLeft()
{
	switch (m_SelectedButton)
	{
	case Start:
		m_SelectedButton = Exit;
		break;
	case Exit:
	case None:
	default:
		m_SelectedButton = Start;
	}

	if(m_pSelect)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pSelect, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void MainMenuScene::MenuGoRight()
{
	switch (m_SelectedButton)
	{
	case Exit:
		m_SelectedButton = Start;
		break;
	case Start:
	case None:
	default:
		m_SelectedButton = Exit;
	}

	if (m_pSelect)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pSelect, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void MainMenuScene::StartGame()
{
	SceneManager::GetInstance()->SetActiveGameScene(L"ControlsSchemaScene");

	if(m_pConfirm)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pConfirm, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
}

void MainMenuScene::ExitGame()
{
	PostQuitMessage(0);
}
