#include "stdafx.h"
#include "TrashCan.h"

#include "ColliderComponent.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"
#include "../PickUp.h"
#include "../PickUp/Container.h"
#include "../PickUp/Ingredient.h"

TrashCan::TrashCan(const DirectX::XMFLOAT3& position, PostHighlight* pPostHighlight)
	: Interactable(position, pPostHighlight, {0,0,0})
	, m_pTrashCan(nullptr)
	, m_pChannel(nullptr)
{
}

void TrashCan::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos.x, m_pos.y+2, m_pos.z);

	m_pModel = new ModelComponent(L"./Resources/Meshes/TrashCan.ovm");
	m_pModel->SetMaterial(9);
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physx->createMaterial(0.f, 0.f, 0.1f);
	AddComponent(new ColliderComponent(geom, *pMat));

	auto pFmod = SoundManager::GetInstance()->GetSystem();
	auto res = pFmod->createSound("./Resources/Sounds/TrashCan.wav", FMOD_DEFAULT, 0, &m_pTrashCan);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void TrashCan::Draw(const GameContext&)
{
}

void TrashCan::Update(const GameContext&)
{
	
}

void TrashCan::OnActivate()
{
}

void TrashCan::OnDeactivate()
{
}

float TrashCan::GetMeshWidth() const
{
	return 30.f;
}

void TrashCan::OnStore()
{
	if (m_pStored)
	{
		auto pContainer = dynamic_cast<Container*>(m_pStored);
		if (pContainer)
		{
			pContainer->DestroyIngredients();
			m_pStored = nullptr;
			return;
		}
		m_pStored->GetScene()->RemoveChild(m_pStored);
		m_pStored = nullptr;

		if(m_pTrashCan)
		{
			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pTrashCan, 0, false, &m_pChannel);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
	}
}
