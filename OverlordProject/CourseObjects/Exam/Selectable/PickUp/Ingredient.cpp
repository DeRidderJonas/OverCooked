#include "stdafx.h"
#include "Ingredient.h"

#include "ColliderComponent.h"
#include "ContentManager.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SpriteFont.h"
#include "TextRenderer.h"
#include "TransformComponent.h"
#include "../../ProgressBar.h"


Ingredient::Ingredient(IngredientType type, PostHighlight* pPostHighlight, bool* pIsPaused)
	: PickUp(pPostHighlight)
	, m_Type(type)
	, m_ChopPercent(0.f)
	, m_CookPercent(0.f)
	, m_isChopping(false)
	, m_isCooking(false)
	, m_NeedsCooking(false)
	, m_pIsPaused(pIsPaused)
	, m_pRigidBody(nullptr)
	, m_pCollider(nullptr)
{
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Arial_20.fnt");
}

Ingredient::~Ingredient()
{
	if(!m_pProgressBar->GetParent())
		SafeDelete(m_pProgressBar);
}

bool Ingredient::IsReady() const
{
	if (m_NeedsCooking)
		return IsCooked();
	else
		return IsChopped();
}

void Ingredient::Chop()
{
	m_isChopping = true;
	if(!m_pProgressBar->GetParent())
		AddChild(m_pProgressBar);
}

void Ingredient::StopChopping()
{
	m_isChopping = false;
}

bool Ingredient::IsChopped() const
{
	return m_ChopPercent >= 1.f;
}

void Ingredient::Cook()
{
	m_isCooking = true;
}

void Ingredient::StopCooking()
{
	m_isCooking = false;
}

bool Ingredient::IsCooked() const
{
	return m_CookPercent >= 1.f;
}

bool Ingredient::NeedsCooking() const
{
	return m_NeedsCooking;
}

void Ingredient::OnAddedToContainer()
{
	m_pRigidBody->SetKinematic(true);
	m_pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group9);

	m_pCollider->EnableTrigger(true);
}

void Ingredient::RemoveProgressBar()
{
	if (m_pProgressBar->GetParent())
		RemoveChild(m_pProgressBar);
}

Ingredient::IngredientType Ingredient::GetType() const
{
	return m_Type;
}

void Ingredient::Initialize(const GameContext&)
{
	auto physx = PhysxManager::GetInstance()->GetPhysics();
	
	switch (m_Type)
	{
	case IngredientType::Cabbage:
		{
			m_pModel = new ModelComponent(L"./Resources/Meshes/Cabbage.ovm");
			m_pModel->SetMaterial(5);
			std::shared_ptr<physx::PxGeometry> geom{ new physx::PxSphereGeometry(2) };
			auto pMat = physx->createMaterial(0.3f, 0.3f, 0.05f);
			AddComponent(m_pCollider = new ColliderComponent(geom, *pMat));
		}
		break;
	case IngredientType::Tomato:
		{
			m_pModel = new ModelComponent(L"./Resources/Meshes/tomato.ovm");
			m_pModel->SetMaterial(6);
			std::shared_ptr<physx::PxGeometry> geom{ new physx::PxSphereGeometry(2) };
			auto pMat = physx->createMaterial(0.3f, 0.3f, 0.05f);
			AddComponent(m_pCollider = new ColliderComponent(geom, *pMat));
		}
		break;
	case IngredientType::Steak:
		{
			m_pModel = new ModelComponent(L"./Resources/Meshes/steak.ovm");
			m_pModel->SetMaterial(7);
			std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{2,1,2} };
			auto pMat = physx->createMaterial(0.3f, 0.3f, 0.05f);
			AddComponent(m_pCollider = new ColliderComponent(geom, *pMat));
		}
		m_NeedsCooking = true;
		break;
	}

	m_pRigidBody = new RigidBodyComponent();
	m_pRigidBody->SetDensity(50.f);
	m_pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	AddComponent(m_pRigidBody);

	AddComponent(m_pModel);

	m_pProgressBar = new ProgressBar();
}

void Ingredient::Draw(const GameContext&)
{
}

void Ingredient::Update(const GameContext& gameContext)
{
	if (*m_pIsPaused)
		return;
	
	if(m_isChopping)
	{
		float chopSpeed{ 1.f / m_TimeToChop };
		m_ChopPercent += chopSpeed * gameContext.pGameTime->GetElapsed();
		if (m_ChopPercent > 1.f) m_ChopPercent = 1.f;
	}
	if(m_isCooking)
	{
		float cookSpeed{ 1.f / m_TimeToCook };
		m_CookPercent += cookSpeed * gameContext.pGameTime->GetElapsed();
		if (m_CookPercent > 1.f) m_CookPercent = 1.f;
	}

	float percentage{ m_NeedsCooking ? (m_ChopPercent + m_CookPercent) / 2.f : m_ChopPercent };
	if(m_pProgressBar->GetParent())
	{
		m_pProgressBar->SetPercentage(percentage);
		const auto& pos = GetTransform()->GetWorldPosition();
		auto screenPos = gameContext.pCamera->GetScreenPosition(pos);

		m_pProgressBar->GetTransform()->Translate(screenPos.x + m_ProgressBarOffset.x, screenPos.y + m_ProgressBarOffset.y, 0.95f);
	}
	
}

float Ingredient::GetMeshWidth() const
{
	switch (m_Type)
	{
	case IngredientType::Cabbage: 
		return 17.f;
	case IngredientType::Tomato:
		return 17.f;
	case IngredientType::Steak:
	default:
		return 22.f;
	}
}
