#include "stdafx.h"
#include "Table.h"

#include "ColliderComponent.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "TransformComponent.h"

Table::Table(const DirectX::XMFLOAT3& pos)
	: m_pos{pos}
	, m_pModel(nullptr)
{
}

void Table::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();
	
	GetTransform()->Translate(m_pos);

	m_pModel = new ModelComponent(L"./Resources/Meshes/Table.ovm");
	m_pModel->SetMaterial(15);
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group1);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physx->createMaterial(0.f, 0.f, 0.f);
	AddComponent(new ColliderComponent(geom, *pMat));
}
