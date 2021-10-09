#include "stdafx.h"
#include "DeliveryIsle.h"

#include "ColliderComponent.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "PlateReturnIsle.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"
#include "../PickUp.h"
#include "../../KitchenScene.h"
#include "../PickUp/Container.h"

DeliveryIsle::DeliveryIsle(const DirectX::XMFLOAT3& position, PostHighlight* pPostHighlight, PlateReturnIsle* pPlateReturnIsle)
	: Interactable(position, pPostHighlight, {0,25,0})
	, m_pPlateReturnIsle(pPlateReturnIsle)
	, m_pDeliver(nullptr)
	, m_pChannel(nullptr)
{
}

void DeliveryIsle::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos);

	m_pModel = new ModelComponent(L"./Resources/Meshes/KitchenIsle.ovm");
	m_pModel->SetMaterial(10);
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physx->createMaterial(0.f, 0.f, 0.f);
	AddComponent(new ColliderComponent(geom, *pMat));

	auto pFmod = SoundManager::GetInstance()->GetSystem();
	auto res = pFmod->createSound("./Resources/Sounds/DeliverBell.wav", FMOD_DEFAULT, 0, &m_pDeliver);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void DeliveryIsle::Draw(const GameContext&)
{
}

void DeliveryIsle::Update(const GameContext&)
{
	if(m_pStored)
	{
		auto pContainer{ dynamic_cast<Container*>(m_pStored) };
		dynamic_cast<KitchenScene*>(GetScene())->Deliver(pContainer);
		m_pStored = nullptr;
		if (m_pPlateReturnIsle)
		{
			m_pPlateReturnIsle->OnFoodDelivered();

			if(m_pDeliver)
			{
				auto pFmod = SoundManager::GetInstance()->GetSystem();
				auto res = pFmod->playSound(m_pDeliver, 0, false, &m_pChannel);
				SoundManager::GetInstance()->ErrorCheck(res);
			}
		}
	}
}

void DeliveryIsle::OnActivate()
{
}

void DeliveryIsle::OnDeactivate()
{
}

float DeliveryIsle::GetMeshWidth() const
{
	return 50.f;
}

bool DeliveryIsle::CanStore(PickUp* pPickup) const
{
	auto pContainer = dynamic_cast<Container*>(pPickup);
	if (!pContainer || pContainer->GetType() != Container::ContainerType::Plate)
		return false;

	return true;
}
