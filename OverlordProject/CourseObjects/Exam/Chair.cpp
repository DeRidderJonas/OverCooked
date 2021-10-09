#include "stdafx.h"
#include "Chair.h"

#include "ColliderComponent.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "TransformComponent.h"

Chair::Chair(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot)
	: m_pos{pos}
	, m_rot(rot)
{
}

void Chair::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos);

	auto pModel = new ModelComponent(L"./Resources/Meshes/Chair.ovm");
	pModel->SetMaterial(16);
	AddComponent(pModel);

	auto pRigidBody = new RigidBodyComponent(false);
	pRigidBody->SetKinematic(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group1);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physx->createMaterial(0.f, 0.f, 0.f);
	AddComponent(new ColliderComponent(geom, *pMat));
}

void Chair::PostInitialize(const GameContext&)
{
	GetTransform()->Rotate(m_rot);
	
}
