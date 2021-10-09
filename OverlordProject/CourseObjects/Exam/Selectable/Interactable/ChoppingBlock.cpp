#include "stdafx.h"
#include "ChoppingBlock.h"

#include "ColliderComponent.h"
#include "../PickUp/Ingredient.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"

ChoppingBlock::ChoppingBlock(const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight)
	: Interactable(pos, pPostHighlight, DirectX::XMFLOAT3{0,15,0})
	, m_pChoppingBlock(nullptr)
	, m_pChopping(nullptr)
	, m_pChannel(nullptr)
{
}

void ChoppingBlock::Reset()
{
	Interactable::Reset();

	if (m_pChannel)
		m_pChannel->setPaused(true);
}

void ChoppingBlock::Initialize(const GameContext&)
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos);
	
	m_pModel = new ModelComponent(L"./Resources/Meshes/KitchenIsle.ovm");
	m_pModel->SetMaterial(2);
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> boxGeom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physX->createMaterial(0.1f, 0.1f, 0.1f);
	AddComponent(new ColliderComponent(boxGeom, *pMat, physx::PxTransform::createIdentity()));

	m_pChoppingBlock = new ModelComponent(L"./Resources/Meshes/CuttingBoard.ovm");
	m_pChoppingBlock->SetMaterial(4);
	auto go2 = new GameObject();
	go2->AddComponent(m_pChoppingBlock);
	AddChild(go2);
	go2->GetTransform()->Translate(- 1.f, 15.f, 0);

	auto pFmod = SoundManager::GetInstance()->GetSystem();
	auto res = pFmod->createSound("./Resources/Sounds/Chopping.wav", FMOD_LOOP_NORMAL, 0, &m_pChopping);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void ChoppingBlock::Draw(const GameContext&)
{
}

void ChoppingBlock::Update(const GameContext&)
{
}

void ChoppingBlock::OnActivate()
{
	auto pIngredient = dynamic_cast<Ingredient*>(m_pStored);
	if (pIngredient)
	{
		pIngredient->Chop();

		if(m_pChopping)
		{
			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pChopping, 0, false, &m_pChannel);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
	}
}

void ChoppingBlock::OnDeactivate()
{
	auto pIngredient = dynamic_cast<Ingredient*>(m_pStored);
	if (pIngredient)
	{
		pIngredient->StopChopping();

		if(m_pChannel)
		{
			m_pChannel->setPaused(true);
		}
	}
}

float ChoppingBlock::GetMeshWidth() const
{
	return 50.f;
}

void ChoppingBlock::OnTake()
{
	m_pChannel->setPaused(true);
}
