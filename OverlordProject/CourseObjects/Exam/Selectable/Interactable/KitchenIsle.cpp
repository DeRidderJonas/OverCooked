#include "stdafx.h"
#include "KitchenIsle.h"

#include "ColliderComponent.h"
#include "ContentManager.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "TransformComponent.h"
#include "../PickUp/Container.h"


KitchenIsle::KitchenIsle(const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight, bool spawnWithPlate)
	: Interactable(pos, pPostHighlight, DirectX::XMFLOAT3{0,15,0})
	, m_SpawnPlate(spawnWithPlate)
	, m_OriginalSpawnWithPlate(spawnWithPlate)
{
}

void KitchenIsle::Reset()
{
	Interactable::Reset();

	m_SpawnPlate = m_OriginalSpawnWithPlate;
}

void KitchenIsle::Initialize(const GameContext&)
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
}

void KitchenIsle::Draw(const GameContext&)
{
}

void KitchenIsle::Update(const GameContext&)
{
	if (!m_SpawnPlate)
		return;

	auto pPlate = new Container(Container::ContainerType::Plate, m_pPH);
	GetScene()->AddChild(pPlate);
	Store(pPlate);
	m_SpawnPlate = false;
}

void KitchenIsle::OnActivate()
{
}

void KitchenIsle::OnDeactivate()
{
}

float KitchenIsle::GetMeshWidth() const
{
	return 50.f;
}

void KitchenIsle::PostInitialize(const GameContext&)
{
}

