#include "stdafx.h"
#include "Stove.h"

#include "ColliderComponent.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "ParticleEmitterComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"
#include "../PickUp/Container.h"
#include "../PickUp/Ingredient.h"

Stove::Stove(const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight)
	: Interactable(pos, pPostHighlight, DirectX::XMFLOAT3{0,15,0})
	, m_pParticleEmitter(nullptr)
	, m_pChannel(nullptr)
	, m_pCooking(nullptr)
{
}

void Stove::Reset()
{
	m_pParticleEmitter->Stop();

	Interactable::Reset();

	auto pPan = new Container(Container::ContainerType::Pan, m_pPH);
	GetScene()->AddChild(pPan);
	Store(pPan);

	if (m_pChannel)
		m_pChannel->setPaused(true);
}

void Stove::Initialize(const GameContext&)
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos);
	
	m_pModel = new ModelComponent(L"./Resources/Meshes/Stove.ovm");
	m_pModel->SetMaterial(3);
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> boxGeom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physX->createMaterial(0.1f, 0.1f, 0.1f);
	AddComponent(new ColliderComponent(boxGeom, *pMat, physx::PxTransform::createIdentity()));

	auto pGo = new GameObject();
	m_pParticleEmitter = new ParticleEmitterComponent(L"./Resources/Textures/Smoke.png");
	m_pParticleEmitter->SetVelocity(DirectX::XMFLOAT3(0, 6.0f, 0));
	m_pParticleEmitter->SetMinSize(1.0f);
	m_pParticleEmitter->SetMaxSize(2.0f);
	m_pParticleEmitter->SetMinEnergy(1.0f);
	m_pParticleEmitter->SetMaxEnergy(2.0f);
	m_pParticleEmitter->SetMinSizeGrow(3.5f);
	m_pParticleEmitter->SetMaxSizeGrow(5.5f);
	m_pParticleEmitter->SetMinEmitterRange(0.2f);
	m_pParticleEmitter->SetMaxEmitterRange(0.5f);
	m_pParticleEmitter->SetColor(DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.6f));
	pGo->AddComponent(m_pParticleEmitter);
	AddChild(pGo);
	pGo->GetTransform()->Translate(0,75,-92);

	auto pFmod = SoundManager::GetInstance()->GetSystem();
	auto res = pFmod->createSound("./Resources/Sounds/Cooking.wav", FMOD_LOOP_NORMAL, 0, &m_pCooking);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void Stove::Draw(const GameContext&)
{
}

void Stove::Update(const GameContext&)
{
}

void Stove::OnActivate()
{
}

void Stove::OnDeactivate()
{
}

float Stove::GetMeshWidth() const
{
	return 50.f;
}

bool Stove::CanStore(PickUp* pPickup) const
{
	auto pContainer = dynamic_cast<Container*>(pPickup);
	if (!pContainer || pContainer->GetType() != Container::ContainerType::Pan)
		return false;
	
	return true;
}

void Stove::OnStore()
{
	auto pPan = dynamic_cast<Container*>(m_pStored);
	if (pPan && pPan->GetType() == Container::ContainerType::Pan)
	{
		if (pPan->GetIngredients().empty())
			return;
		
		for(auto& pIngredient : pPan->GetIngredients())
		{
			pIngredient->Cook();
		}
		m_pParticleEmitter->Start();

		if(m_pCooking)
		{
			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pCooking, 0, false, &m_pChannel);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
	}
}

void Stove::OnTake()
{
	auto pPan = dynamic_cast<Container*>(m_pStored);
	if (pPan && pPan->GetType() == Container::ContainerType::Pan)
	{
		for(auto& pIngredient : pPan->GetIngredients())
		{
			pIngredient->StopCooking();
		}
		m_pParticleEmitter->Stop();

		if(m_pChannel)
		{
			m_pChannel->setPaused(true);
		}
	}
}

void Stove::PostInitialize(const GameContext&)
{
	auto pPan = new Container(Container::ContainerType::Pan, m_pPH);
	GetScene()->AddChild(pPan);
	Store(pPan);
}
