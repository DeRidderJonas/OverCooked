#include "stdafx.h"
#include "PlateReturnIsle.h"

#include "ColliderComponent.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"
#include "../PickUp/Container.h"

PlateReturnIsle::PlateReturnIsle(const DirectX::XMFLOAT3& pos, PostHighlight* pHighlight, bool* pIsPaused)
	: Interactable(pos, pHighlight, {0,15,0})
	, m_updateStored(false)
	, m_pIsPaused(pIsPaused)
	, m_pReturnPlate(nullptr)
	, m_pChannel(nullptr)
{
}

void PlateReturnIsle::OnFoodDelivered()
{
	m_timesUntillPlateSpawn.push_back(m_PlateSpawnTimeAfterDeliver);
}

void PlateReturnIsle::Reset()
{
	m_timesUntillPlateSpawn.clear();
	for(auto pContainer : m_pPlates)
	{
		GetScene()->RemoveChild(pContainer);
	}
	m_pPlates.clear();

	Interactable::Reset();
}

void PlateReturnIsle::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos);

	m_pModel = new ModelComponent(L"./Resources/Meshes/KitchenIsle.ovm");
	m_pModel->SetMaterial(14);
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> geom{new physx::PxBoxGeometry{ 5,10,5 }};
	auto pMat = physx->createMaterial(0.f, 0.f, 0.f);
	AddComponent(new ColliderComponent(geom, *pMat));

	auto pFmod = SoundManager::GetInstance()->GetSystem();
	auto res = pFmod->createSound("./Resources/Sounds/ReturnPlate.wav", FMOD_DEFAULT, 0, &m_pReturnPlate);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void PlateReturnIsle::Draw(const GameContext&)
{
}

void PlateReturnIsle::Update(const GameContext& gameContext)
{
	if (*m_pIsPaused)
		return;
	
	float deltaTime{ gameContext.pGameTime->GetElapsed() };
	std::transform(m_timesUntillPlateSpawn.begin(), m_timesUntillPlateSpawn.end(), m_timesUntillPlateSpawn.begin(), [&deltaTime](float& time) {return time - deltaTime; });

	int amountOfPlatesToAdd = std::count_if(m_timesUntillPlateSpawn.begin(), m_timesUntillPlateSpawn.end(), [](const float& time) {return time <= 0.f; });
	m_timesUntillPlateSpawn.erase(std::remove_if(m_timesUntillPlateSpawn.begin(), m_timesUntillPlateSpawn.end(), [](const float& time) {return time <= 0.f; }), m_timesUntillPlateSpawn.end());

	for (int i = 0; i < amountOfPlatesToAdd; ++i)
	{
		auto pPlate = new Container(Container::ContainerType::Plate, m_pPH);

		using namespace DirectX;
		auto spawnPosV = DirectX::XMLoadFloat3(&GetTransform()->GetPosition()) + DirectX::XMLoadFloat3(&m_StoredPosOffset);
		DirectX::XMFLOAT3 spawnPos{};
		DirectX::XMStoreFloat3(&spawnPos, spawnPosV);
		spawnPos.y += m_pPlates.size() * m_offsetBetweenPlatesY;
		
		GetScene()->AddChild(pPlate);

		pPlate->GetTransform()->Translate(spawnPos);
		pPlate->GetComponent<RigidBodyComponent>()->SetKinematic(true);

		pPlate->SetCanBeSelected(false);
		
		m_pPlates.push_back(pPlate);

		Store(pPlate);

		if(m_pReturnPlate)
		{
			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pReturnPlate, 0, false, &m_pChannel);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
	}

	if(m_updateStored)
	{
		if (!m_pPlates.empty())
			Store(*(m_pPlates.begin()));
		m_updateStored = false;
	}
}

void PlateReturnIsle::OnActivate()
{
}

void PlateReturnIsle::OnDeactivate()
{
}

bool PlateReturnIsle::CanStore(PickUp* pPickup) const
{
	auto pContainer = dynamic_cast<Container*>(pPickup);
	return pContainer && pContainer->GetType() == Container::ContainerType::Plate;
}

float PlateReturnIsle::GetMeshWidth() const
{
	return 50.f;
}

void PlateReturnIsle::OnTake()
{
	m_updateStored = true;
	if(!m_pPlates.empty())
		m_pPlates.pop_front();
}
