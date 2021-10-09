#include "stdafx.h"
#include "ControlsSchemaScene.h"

#include "GameObject.h"
#include "SceneManager.h"
#include "SpriteComponent.h"

ControlsSchemaScene::ControlsSchemaScene()
	: GameScene(L"ControlsSchemaScene")
{
}

void ControlsSchemaScene::Initialize()
{
	auto pGo = new GameObject();
	auto pSpriteComponent = new SpriteComponent(L"./Resources/Textures/Controls.png");
	pGo->AddComponent(pSpriteComponent);
	AddChild(pGo);
}

void ControlsSchemaScene::Update()
{
	if(GetGameContext().pInput->IsActionTriggered(m_ContinueInputId))
	{
		SceneManager::GetInstance()->SetActiveGameScene(L"KitchenScene");
	}
}

void ControlsSchemaScene::Draw()
{
}

void ControlsSchemaScene::SceneActivated()
{
	GetGameContext().pInput->ClearInputActions();
	GetGameContext().pInput->AddInputAction({ m_ContinueInputId, InputTriggerState::Pressed, 'E', -1, 0x1000 });
}

void ControlsSchemaScene::SceneDeactivated()
{
}
