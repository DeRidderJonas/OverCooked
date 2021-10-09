#include "stdafx.h"
#include "Selectable.h"

#include "GameScene.h"
#include "TransformComponent.h"
#include "PP/PostHighlight.h"

Selectable::Selectable(PostHighlight* pPostHighlight)
	: m_pPH(pPostHighlight)
	, m_canBeSelected(true)
{
}

bool Selectable::CanBeSelected() const
{
	return m_canBeSelected;
}

void Selectable::SetCanBeSelected(bool value)
{
	m_canBeSelected = value;
}

void Selectable::Select()
{
	const auto& pos = GetTransform()->GetWorldPosition();
	auto screenPos = GetScene()->GetGameContext().pCamera->GetScreenPosition(pos);

	float meshWidth{ GetMeshWidth() };
	m_pPH->SetTopLeft(screenPos.x - meshWidth / 2.f, screenPos.y + meshWidth / 2.f);
	m_pPH->SetBottomRight(screenPos.x + meshWidth / 2.f, screenPos.y - meshWidth / 2.f);
}

void Selectable::Unselect()
{
	m_pPH->SetTopLeft(0.f, 0.f);
	m_pPH->SetBottomRight(0.f, 0.f);
}
