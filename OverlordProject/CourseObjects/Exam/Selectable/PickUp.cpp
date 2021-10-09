#include "stdafx.h"
#include "PickUp.h"

#include "GameScene.h"
#include "TransformComponent.h"

PickUp::PickUp(PostHighlight* pHighlight)
	: Selectable(pHighlight)
	, m_pModel(nullptr)
{
}

void PickUp::Initialize(const GameContext&)
{
}

void PickUp::Draw(const GameContext&)
{
}

void PickUp::Update(const GameContext&)
{
}
