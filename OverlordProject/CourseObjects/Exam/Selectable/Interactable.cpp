#include "stdafx.h"
#include "Interactable.h"

#include "GameScene.h"
#include "ModelComponent.h"
#include "PickUp.h"
#include "RigidBodyComponent.h"
#include "TransformComponent.h"
#include "PickUp/Container.h"
#include "PickUp/Ingredient.h"

Interactable::Interactable(const DirectX::XMFLOAT3& position, PostHighlight* pPostHighlight, const DirectX::XMFLOAT3& storedPosOffset)
	: Selectable(pPostHighlight)
	, m_pModel(nullptr)
	, m_pStored(nullptr)
	, m_pos(position)
	, m_StoredPosOffset(storedPosOffset)
	, m_isActive(false)
{
}

bool Interactable::Store(PickUp* pPickup, bool& keepHoldingItem)
{
	keepHoldingItem = false;
	
	if (m_pStored != nullptr)
	{
		auto pStoredContainer = dynamic_cast<Container*>(m_pStored);
		auto pIngredient = dynamic_cast<Ingredient*>(pPickup);
		auto pContainer = dynamic_cast<Container*>(pPickup);
		
		//Can only store if stored is plate container and pPickup is ingredient or pan container (in which case the ingredients are transfered)
		if (!pStoredContainer || 
			(pStoredContainer->GetType() == Container::ContainerType::Pan && pContainer && pContainer->GetType() == Container::ContainerType::Plate))
			return false;

		//Transfer ingredients from pan to plate
		if(pStoredContainer && pStoredContainer->GetType() == Container::ContainerType::Plate && pContainer && pContainer->GetType() == Container::ContainerType::Pan)
		{
			pContainer->TransferIngredients(pStoredContainer);
			keepHoldingItem = true;
			return true;
		}

		if (!pIngredient)
			return false;
		
		if (pStoredContainer->AddIngredient(pIngredient))
		{
			OnStore();
			return true;
		}
		else return false;
	}

	if (!CanStore(pPickup))
		return false;
	
	m_pStored = pPickup;

	using namespace DirectX;

	auto pos = DirectX::XMLoadFloat3(&m_pos);
	auto offset = DirectX::XMLoadFloat3(&m_StoredPosOffset);

	DirectX::XMFLOAT3 offsetPos{};
	DirectX::XMStoreFloat3(&offsetPos, pos + offset);

	m_pStored->SetCanBeSelected(false);
	m_pStored->GetTransform()->Translate(offsetPos);
	m_pStored->GetComponent<RigidBodyComponent>()->SetKinematic(true);

	OnStore();
	return true;
}

bool Interactable::Store(PickUp* pPickup)
{
	bool discardBool{ false };
	return Store(pPickup, discardBool);
}

PickUp* Interactable::Take()
{
	if (!m_pStored) return nullptr;

	m_pStored->SetCanBeSelected(true);
	auto temp = m_pStored;

	OnTake();
	
	m_pStored = nullptr;
	
	return temp;
}

void Interactable::Activate()
{
	m_isActive = true;
	OnActivate();
}

void Interactable::Deactivate()
{
	m_isActive = false;
	OnDeactivate();
}

void Interactable::Reset()
{
	if(m_pStored)
	{
		GetScene()->RemoveChild(m_pStored);
		m_pStored = nullptr;
	}
}

void Interactable::PostInitialize(const GameContext&)
{
}

bool Interactable::CanStore(PickUp* ) const
{
	return true;
}

void Interactable::OnStore()
{
}

void Interactable::OnTake()
{
}
