#include "stdafx.h"
#include "Container.h"

#include "ColliderComponent.h"
#include "GameScene.h"
#include "Ingredient.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SoundManager.h"
#include "TransformComponent.h"

Container::Container(ContainerType type, PostHighlight* pPostHighlight)
	: PickUp(pPostHighlight)
	, m_type(type)
	, m_pChannel(nullptr)
	, m_pAddIngredient(nullptr)
	, m_pTransfer(nullptr)
{
}

bool Container::AddIngredient(Ingredient* pIngredient)
{
	if (m_type == ContainerType::Plate && !pIngredient->IsReady())
		return false;

	if(m_type == ContainerType::Pan && (!pIngredient->IsChopped() || !pIngredient->NeedsCooking()))
		return false;
	
	m_ingredients.push_back(pIngredient);
	pIngredient->SetCanBeSelected(false);
	pIngredient->OnAddedToContainer();

	if (m_type == ContainerType::Plate)
		pIngredient->RemoveProgressBar();

	if(m_pAddIngredient)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pAddIngredient, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}

	return true;
}

const std::vector<Ingredient*>& Container::GetIngredients() const
{
	return m_ingredients;
}

void Container::TransferIngredients(Container* pDestinationContainer)
{
	bool notReady = std::any_of(m_ingredients.begin(), m_ingredients.end(), [](Ingredient* pIngredient)
		{
			return !pIngredient->IsReady();
		});
	if (notReady)
		return;

	if(m_pTransfer)
	{
		auto pFmod = SoundManager::GetInstance()->GetSystem();
		auto res = pFmod->playSound(m_pTransfer, 0, false, &m_pChannel);
		SoundManager::GetInstance()->ErrorCheck(res);
	}
	
	for(auto& pIngredient : m_ingredients)
	{
		pDestinationContainer->AddIngredient(pIngredient);
		pIngredient->RemoveProgressBar();
	}
	m_ingredients.clear();
}

void Container::DestroyIngredients()
{
	for (auto& pIngredient : m_ingredients)
	{
		GetScene()->RemoveChild(pIngredient);
	}
	m_ingredients.clear();
}

Container::ContainerType Container::GetType() const
{
	return m_type;
}

void Container::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();
	auto pFmod = SoundManager::GetInstance()->GetSystem();
	
	switch (m_type)
	{
	case ContainerType::Plate:
		{
			m_pModel = new ModelComponent(L"./Resources/Meshes/plate.ovm");
			m_pModel->SetMaterial(2);
			std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{2.f,2.f,2.f} };
			auto pMat = physx->createMaterial(0.9f, 0.9f, 0.05f);
			AddComponent(new ColliderComponent(geom, *pMat));
			auto res = pFmod->createSound("./Resources/Sounds/PutOnPlate.wav", FMOD_DEFAULT, 0, &m_pAddIngredient);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
		break;
	case ContainerType::Pan:
		{
			m_pModel = new ModelComponent(L"./Resources/Meshes/pan.ovm");
			m_pModel->SetMaterial(8);
			std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{2.f,2.f,2.f} };
			auto pMat = physx->createMaterial(0.9f, 0.9f, 0.05f);
			AddComponent(new ColliderComponent(geom, *pMat));
			auto res = pFmod->createSound("./Resources/Sounds/PutInPan.wav", FMOD_DEFAULT, 0, &m_pAddIngredient);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
		break;
	}

	auto res = pFmod->createSound("./Resources/Sounds/TransferPotToPlate.wav", FMOD_DEFAULT, 0, &m_pTransfer);
	SoundManager::GetInstance()->ErrorCheck(res);

	auto pRigidBody = new RigidBodyComponent();
	pRigidBody->SetDensity(50.f);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(pRigidBody);

	AddComponent(m_pModel);
}

void Container::Draw(const GameContext&)
{
}

void Container::Update(const GameContext&)
{
	const auto& pos = GetTransform()->GetPosition();
	for(Ingredient* pIngredient : m_ingredients)
	{
		pIngredient->GetTransform()->Translate(pos);
	}
}

void Container::PostInitialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();

	auto d6Joint = physx::PxD6JointCreate(*physx, nullptr, physx::PxTransform::createIdentity(), GetComponent<RigidBodyComponent>()->GetPxRigidBody(), physx::PxTransform::createIdentity());
	d6Joint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);
	d6Joint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eFREE);
	d6Joint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eFREE);
	d6Joint->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eLOCKED);
	d6Joint->setMotion(physx::PxD6Axis::eSWING2, physx::PxD6Motion::eLOCKED);
	d6Joint->setMotion(physx::PxD6Axis::eTWIST, physx::PxD6Motion::eLOCKED);
}

float Container::GetMeshWidth() const
{
	switch (m_type)
	{
	case ContainerType::Plate: 
		return 20.f;
	case ContainerType::Pan:
	default: 
		return 20.f;
	}
}
