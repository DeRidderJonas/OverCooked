#include "stdafx.h"
#include "IngredientCrate.h"

#include "ColliderComponent.h"
#include "GameScene.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"

IngredientCrate::IngredientCrate(Ingredient::IngredientType type, const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight, bool* pIsPaused)
	: Interactable(pos, pPostHighlight, {0,15,0})
	, m_type(type)
	, m_pIsPaused(pIsPaused)
	, m_pChannel(nullptr)
	, m_pSoundOpenCrate(nullptr)
{
	
}

void IngredientCrate::Initialize(const GameContext&)
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	GetTransform()->Translate(m_pos);

	m_pModel = new ModelComponent(L"./Resources/Meshes/Crate.ovm");
	switch (m_type)
	{
	case Ingredient::IngredientType::Cabbage: 
		m_pModel->SetMaterial(13);
		break;
	case Ingredient::IngredientType::Tomato: 
		m_pModel->SetMaterial(11);
		break;
	case Ingredient::IngredientType::Steak:
	default:
		m_pModel->SetMaterial(12);
	}
	AddComponent(m_pModel);

	auto pRigidBody = new RigidBodyComponent(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> boxGeom{ new physx::PxBoxGeometry{5,10,5} };
	auto pMat = physX->createMaterial(0.f, 0.1f, 0.1f);
	AddComponent(new ColliderComponent(boxGeom, *pMat, physx::PxTransform::createIdentity()));

	auto pFMOD = SoundManager::GetInstance()->GetSystem();
	auto res = pFMOD->createSound("./Resources/Sounds/CrateOpen.wav", FMOD_DEFAULT, 0, &m_pSoundOpenCrate);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void IngredientCrate::Draw(const GameContext&)
{
}

void IngredientCrate::Update(const GameContext&)
{
}

void IngredientCrate::OnActivate()
{
	if (m_pStored != nullptr)
		return;

	auto pFMOD = SoundManager::GetInstance()->GetSystem();
	auto res = pFMOD->playSound(m_pSoundOpenCrate, 0, false, &m_pChannel);
	SoundManager::GetInstance()->ErrorCheck(res);
	
	auto pIngredient = new Ingredient(m_type, m_pPH, m_pIsPaused);

	using namespace DirectX;
	auto spawnPosV = DirectX::XMLoadFloat3(&GetTransform()->GetPosition()) + DirectX::XMLoadFloat3(&m_StoredPosOffset);
	DirectX::XMFLOAT3 spawnPos{};
	DirectX::XMStoreFloat3(&spawnPos, spawnPosV);

	GetScene()->AddChild(pIngredient);
	
	pIngredient->GetTransform()->Translate(spawnPos);
	
	Store(pIngredient);
}

void IngredientCrate::OnDeactivate()
{
}

float IngredientCrate::GetMeshWidth() const
{
	return 50.f;
}
