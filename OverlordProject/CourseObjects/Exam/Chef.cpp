#include "stdafx.h"
#include "Chef.h"


#include "ColliderComponent.h"
#include "ControllerComponent.h"
#include "GameScene.h"
#include "Selectable/Interactable.h"
#include "ModelAnimator.h"
#include "ModelComponent.h"
#include "PhysxManager.h"
#include "Selectable/PickUp.h"
#include "RigidBodyComponent.h"
#include "Selectable.h"
#include "SoundManager.h"
#include "TransformComponent.h"
#include "Selectable/Interactable/DeliveryIsle.h"
#include "Selectable/Interactable/TrashCan.h"
#include "Selectable/PickUp/Container.h"
#include "Selectable/PickUp/Ingredient.h"

Chef::Chef(bool* pPaused)
	: GameObject()
	, m_pPaused(pPaused)
	, m_pModel(nullptr)
	, m_pController(nullptr)
	, m_pReachableCollider(nullptr)
	, m_pSelectedItem(nullptr)
	, m_pHeldItem(nullptr)
	, m_isInteracting(false)
	, m_pChannel(nullptr)
	, m_pWalkingChannel(nullptr)
	, m_pPickup(nullptr)
	, m_pWalking(nullptr)
{
}

void Chef::AddInputActions()
{
	auto& gameContext = GetScene()->GetGameContext();
	
	gameContext.pInput->AddInputAction(InputAction{ Input::Up, InputTriggerState::Down, 'W', -1, 0x0040 });
	gameContext.pInput->AddInputAction(InputAction{ Input::Down, InputTriggerState::Down, 'S', -1, 0x0040 });
	gameContext.pInput->AddInputAction(InputAction{ Input::Left, InputTriggerState::Down, 'A', -1, 0x0040 });
	gameContext.pInput->AddInputAction(InputAction{ Input::Right, InputTriggerState::Down, 'D', -1, 0x0040 });
	gameContext.pInput->AddInputAction(InputAction{ Input::Pickup, InputTriggerState::Pressed, 'E', -1, 0x1000 });
	gameContext.pInput->AddInputAction(InputAction{ Input::Interact, InputTriggerState::Pressed, 'F', -1, 0x4000 });
	gameContext.pInput->AddInputAction(InputAction{ Input::StopInteract, InputTriggerState::Released, 'F', -1, 0x4000 });
}

void Chef::Reset()
{
	m_ItemsInReach.erase(std::remove_if(m_ItemsInReach.begin(), m_ItemsInReach.end(), [](GameObject* pGo)
		{
			auto pPickup = dynamic_cast<PickUp*>(pGo);
			return pPickup != nullptr;
		}), m_ItemsInReach.end());

	m_pHeldItem = nullptr;
	m_pModel->GetAnimator()->SetAnimation(Animation::Idle);
	m_pModel->GetAnimator()->Play();
	
	if (m_pChannel)
		m_pChannel->setPaused(true);
	if (m_pWalkingChannel)
		m_pWalkingChannel->setPaused(true);
}

void Chef::Initialize(const GameContext&)
{
	//ModelComponent / Animator
	m_pModel = new ModelComponent(L"./Resources/Meshes/Chef.ovm");
	m_pModel->SetMaterial(0);
	auto go = new GameObject();
	go->AddComponent(m_pModel);
	AddChild(go);

	//ControllerComponent
	auto pPhysMat = PhysxManager::GetInstance()->GetPhysics()->createMaterial(m_StaticFriction, m_DynamicFriction, m_Restitution);
	m_pController = new ControllerComponent(pPhysMat, m_ControllerRadius, m_ControllerHeight);
	AddComponent(m_pController);

	//Reachable Collider
	CreateReachableCollider();

	auto pFmod = SoundManager::GetInstance()->GetSystem();

	auto res = pFmod->createSound("./Resources/Sounds/Pickup.wav", FMOD_DEFAULT, 0, &m_pPickup);
	SoundManager::GetInstance()->ErrorCheck(res);

	res = pFmod->createSound("./Resources/Sounds/Walking.wav", FMOD_LOOP_NORMAL, 0, &m_pWalking);
	SoundManager::GetInstance()->ErrorCheck(res);
	res = pFmod->playSound(m_pWalking, 0, true, &m_pWalkingChannel);
	SoundManager::GetInstance()->ErrorCheck(res);
}

void Chef::Update(const GameContext& gameContext)
{
	//Sound
	if(m_pWalkingChannel)
	{
		FMOD::Sound* pSound{};
		m_pWalkingChannel->getCurrentSound(&pSound);
		if(pSound == nullptr)
		{
			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pWalking, 0, false, &m_pWalkingChannel);
			SoundManager::GetInstance()->ErrorCheck(res);
		}
	}

	if (*m_pPaused)
		return;
	
	//Animation and movement
	auto isIdleBeforeMovementUpdate = DirectX::XMScalarNearEqual(m_Velocity.x, 0.f, 0.1f) && DirectX::XMScalarNearEqual(m_Velocity.z, 0.f, 0.1f);
	if(!m_isInteracting) UpdateMovement(gameContext);
	auto isIdleAfterMovementUpdate = DirectX::XMScalarNearEqual(m_Velocity.x, 0.f, 0.1f) && DirectX::XMScalarNearEqual(m_Velocity.z, 0.f, 0.1f);
	if(!m_isInteracting) UpdateRotation(gameContext);

	if(m_isInteracting){}
	else if(!isIdleBeforeMovementUpdate && isIdleAfterMovementUpdate)
	{
		if (m_pHeldItem)
			m_pModel->GetAnimator()->SetAnimation(Animation::IdleHold);
		else
			m_pModel->GetAnimator()->SetAnimation(Animation::Idle);
		m_pModel->GetAnimator()->Play();
	}
	else if(isIdleBeforeMovementUpdate && !isIdleAfterMovementUpdate)
	{
		if(m_pHeldItem)
			m_pModel->GetAnimator()->SetAnimation(Animation::WalkHold);
		else
			m_pModel->GetAnimator()->SetAnimation(Animation::Walk);
		m_pModel->GetAnimator()->Play();
	}

	UpdateSelectedItem();
	UpdateHeldItem();

	if(gameContext.pInput->IsActionTriggered(Input::Pickup))
	{
		auto pPickup = dynamic_cast<PickUp*>(m_pSelectedItem);
		if (pPickup || m_pSelectedItem == nullptr)
		{
			StorePickUp(pPickup);
		}

		auto pInteractable = dynamic_cast<Interactable*>(m_pSelectedItem);
		if(pInteractable)
		{
			if(m_pHeldItem)
			{
				auto pTrashCan = dynamic_cast<TrashCan*>(m_pSelectedItem);
				if(pTrashCan)
				{
					auto pContainer = dynamic_cast<Container*>(m_pHeldItem);
					if (pContainer)
					{
						auto& ingredients = pContainer->GetIngredients();
						m_ItemsInReach.erase(std::remove_if(m_ItemsInReach.begin(), m_ItemsInReach.end(), [&ingredients](GameObject* pGo)
							{
								return std::find(ingredients.begin(), ingredients.end(), pGo) != ingredients.end();
							}), m_ItemsInReach.end());
					}
					else
						m_ItemsInReach.erase(std::remove(m_ItemsInReach.begin(), m_ItemsInReach.end(), m_pHeldItem), m_ItemsInReach.end());
				}

				auto pDelivery = dynamic_cast<DeliveryIsle*>(m_pSelectedItem);
				if(pDelivery)
				{
					m_ItemsInReach.erase(std::remove(m_ItemsInReach.begin(), m_ItemsInReach.end(), m_pHeldItem), m_ItemsInReach.end());
					auto pContainer = dynamic_cast<Container*>(m_pHeldItem);
					if(pContainer)
					{
						auto& ingredients = pContainer->GetIngredients();
						m_ItemsInReach.erase(std::remove_if(m_ItemsInReach.begin(), m_ItemsInReach.end(), [&ingredients](GameObject* pGo)
							{
								return std::find(ingredients.begin(), ingredients.end(), pGo) != ingredients.end();
							}), m_ItemsInReach.end());
					}
				}

				bool keepHoldingItem{ false };
				if(pInteractable->Store(m_pHeldItem, keepHoldingItem))
				{

					auto pFmod = SoundManager::GetInstance()->GetSystem();
					auto res = pFmod->playSound(m_pPickup, 0, false, &m_pChannel);
					SoundManager::GetInstance()->ErrorCheck(res);
					
					if (pTrashCan)
					{
						auto pContainer = dynamic_cast<Container*>(m_pHeldItem);
						if (pContainer)
						{
							auto temp = m_pHeldItem;
							m_pHeldItem = nullptr;
							StorePickUp(temp);
							keepHoldingItem = true;
						}
					}
					if (!keepHoldingItem)
					{
						m_pHeldItem = nullptr;

						m_pModel->GetAnimator()->SetAnimation(Animation::Idle);
						m_pModel->GetAnimator()->Play();
					}
				}
				else
					StorePickUp(m_pHeldItem);
			}
			else
			{
				auto pStored = pInteractable->Take();
				if(pStored)
					StorePickUp(pStored);
			}
		}
	}

	if (gameContext.pInput->IsActionTriggered(Input::Interact))
	{
		auto pInteractable = dynamic_cast<Interactable*>(m_pSelectedItem);
		InteractWith(pInteractable);
	}
	if(gameContext.pInput->IsActionTriggered(Input::StopInteract))
	{
		auto pInteractable = dynamic_cast<Interactable*>(m_pSelectedItem);
		StopInteractingWith(pInteractable);
	}
}

void Chef::PostInitialize(const GameContext&)
{
	m_pController->SetFootPosition(0, 1, 0);
	m_pModel->GetTransform()->Translate(0, -90, 0);
	m_pModel->GetAnimator()->SetAnimation(Animation::Idle);
	m_pModel->GetAnimator()->Play();
}

void Chef::CreateReachableCollider()
{
	m_pReachableCollider = new GameObject();

	auto pRigidBody = new RigidBodyComponent();
	pRigidBody->SetKinematic(true);
	pRigidBody->SetCollisionGroup(CollisionGroupFlag::Group0);
	m_pReachableCollider->AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> geom{ new physx::PxBoxGeometry{7,10,7} };
	auto pMat = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.f, 0.f, 0.f);
	auto pColliderComponent = new ColliderComponent(geom, *pMat);
	pColliderComponent->EnableTrigger(true);
	m_pReachableCollider->AddComponent(pColliderComponent);

	auto triggerCallback = [this](GameObject* , GameObject* otherobject, TriggerAction action)
	{
		if (!(dynamic_cast<PickUp*>(otherobject) || dynamic_cast<Interactable*>(otherobject)))
			return;
		
		if(action == TriggerAction::ENTER)
		{
			m_ItemsInReach.push_back(otherobject);
		}
		else if(action == TriggerAction::LEAVE)
		{
			m_ItemsInReach.erase(std::remove(m_ItemsInReach.begin(), m_ItemsInReach.end(), otherobject), m_ItemsInReach.end());
		}
		
	};
	m_pReachableCollider->SetOnTriggerCallBack(triggerCallback);

	AddChild(m_pReachableCollider);
}

void Chef::UpdateMovement(const GameContext& gameContext)
{
	using namespace DirectX;

	//Input Movement
	DirectX::XMFLOAT2 move{ 0,0 };
	if (gameContext.pInput->IsActionTriggered(Input::Up))
		move.y = 1.f;
	if (gameContext.pInput->IsActionTriggered(Input::Down))
		move.y = -1.f;
	if (gameContext.pInput->IsActionTriggered(Input::Left))
		move.x = -1.f;
	if (gameContext.pInput->IsActionTriggered(Input::Right))
		move.x = 1.f;
	if(DirectX::XMScalarNearEqual(move.x, 0.f, 0.01f) && DirectX::XMScalarNearEqual(move.y, 0.f, 0.01f))
	{
		move = gameContext.pInput->GetThumbstickPosition();
	}

	if (!DirectX::XMScalarNearEqual(move.x, 0.f, 0.01f) || !DirectX::XMScalarNearEqual(move.y, 0.f, 0.01f))
	{
		m_RunVelocity += m_RunAcceleration * gameContext.pGameTime->GetElapsed();
		if (m_RunVelocity > m_MaxRunVelocity) m_RunVelocity = m_MaxRunVelocity;
		float yVel = m_Velocity.y;
		DirectX::XMVECTOR direction{ move.x, 0.f, move.y };
		direction = DirectX::XMVector3Normalize(direction);
		auto scaledDirection = direction * m_RunVelocity;
		DirectX::XMStoreFloat3(&m_Velocity, scaledDirection);
		m_Velocity.y = yVel;
	}

	if (DirectX::XMScalarNearEqual(move.x, 0.f, 0.01f) && DirectX::XMScalarNearEqual(move.y, 0.f, 0.01f))
	{
		m_Velocity.x = 0;
		m_Velocity.z = 0;
		m_RunVelocity = 0;
	}

	//Gravity
	auto collisionFlags = m_pController->GetCollisionFlags();
	if (!(collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
	{
		m_JumpVelocity -= m_JumpAcceleration * gameContext.pGameTime->GetElapsed();
		if (abs(m_JumpVelocity) > m_TerminalVelocity)
			m_JumpVelocity = -m_TerminalVelocity;
	}
	else
		m_Velocity.y = 0.f;

	m_Velocity.y += m_JumpVelocity;

	DirectX::XMVECTOR velocityVector = DirectX::XMLoadFloat3(&m_Velocity);
	auto scaledVelocity = DirectX::XMVector3Normalize(velocityVector);
	DirectX::XMFLOAT3 scaledFloat3{};
	DirectX::XMStoreFloat3(&scaledFloat3, scaledVelocity);

	const auto forward = DirectX::XMLoadFloat3(&GetTransform()->GetForward());
	const auto right = DirectX::XMLoadFloat3(&GetTransform()->GetRight());
	XMVECTOR movement{};

	movement += forward * scaledFloat3.z;
	movement += right * scaledFloat3.x;
	
	movement = DirectX::XMVector3Normalize(movement);
	movement *= m_RunVelocity * gameContext.pGameTime->GetElapsed();
	
	XMFLOAT3 displacement{};
	DirectX::XMStoreFloat3(&displacement, movement);
	displacement.y = m_Velocity.y * gameContext.pGameTime->GetElapsed();
	m_pController->Move(displacement);

	
	if (m_RunVelocity > 0.f && m_RunVelocityPreviousFrame <= 0.f && m_pWalkingChannel)
	{
		m_pWalkingChannel->setPaused(false);
	}
	else if(m_RunVelocity <= 0.f && m_RunVelocityPreviousFrame > 0.f && m_pWalkingChannel)
	{
		m_pWalkingChannel->setPaused(true);
	}

	m_RunVelocityPreviousFrame = m_RunVelocity;
}

void Chef::UpdateRotation(const GameContext& gameContext)
{
	m_TotalYaw = fmod(m_TotalYaw, 360.f);
	while (m_TotalYaw < 0.f) m_TotalYaw += 360.f;
	
	bool up{ gameContext.pInput->IsActionTriggered(Input::Up) };
	bool down{ gameContext.pInput->IsActionTriggered(Input::Down) };
	bool left{ gameContext.pInput->IsActionTriggered(Input::Left) };
	bool right{ gameContext.pInput->IsActionTriggered(Input::Right) };

	if(!up && !down && !left && !right)
	{
		auto leftThumb = gameContext.pInput->GetThumbstickPosition();
		up = leftThumb.y > 0.f;
		down = leftThumb.y < 0.f;
		left = leftThumb.x < 0.f;
		right = leftThumb.x > 0.f;
	}

	float desiredYaw{ m_TotalYaw };
	if (up)
	{
		desiredYaw = 180.f;
		if (left) desiredYaw = 135.f;
		if (right) desiredYaw = 225.f;
	}
	else if (down)
	{
		desiredYaw = 0.f;
		if (left) desiredYaw = 45.f;
		if (right) desiredYaw = 315.f;
	}
	else if (left)
		desiredYaw = 90.f;
	else if (right)
		desiredYaw = 270.f;

	float rotationDirection{ 0.f };
	if(!DirectX::XMScalarNearEqual(desiredYaw, m_TotalYaw, 1.f))
	{
		//https://math.stackexchange.com/questions/110080/shortest-way-to-achieve-target-angle
		float alpha{ desiredYaw - m_TotalYaw };
		float beta{ desiredYaw - m_TotalYaw + 360 };
		float theta{ desiredYaw - m_TotalYaw - 360 };

		float values[3]{ alpha, beta, theta };
		auto minIt = std::min_element(std::begin(values), std::end(values), [](float left, float right) {return abs(left) < abs(right); });
		rotationDirection = *minIt > 0.f ? 1.f : -1.f;
	}

	m_TotalYaw += rotationDirection * m_RotationSpeed * gameContext.pGameTime->GetElapsed();
	m_pModel->GetTransform()->Rotate(0, m_TotalYaw, 0);
}

void Chef::UpdateSelectedItem()
{
	//Reachable Collider
	using namespace DirectX;
	auto forwardV = DirectX::XMLoadFloat3(&m_pModel->GetTransform()->GetForward());
	forwardV *= m_ReachableColliderOffset;
	auto playerPos = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());

	DirectX::XMFLOAT3 colliderPos{};
	DirectX::XMStoreFloat3(&colliderPos, playerPos - forwardV);

	m_pReachableCollider->GetTransform()->Translate(colliderPos);

	//Selected item
	auto playerPosition = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());
	auto nearestItem = std::min_element(m_ItemsInReach.begin(), m_ItemsInReach.end(), [&playerPosition, this](GameObject* left, GameObject* right)
		{
			auto pLeft = dynamic_cast<Selectable*>(left);
			auto pRight = dynamic_cast<Selectable*>(right);

			if (pLeft == m_pHeldItem) return false;
			if (pRight == m_pHeldItem) return true;

			if (!pLeft->CanBeSelected()) return false;
			if (!pRight->CanBeSelected()) return true;
		
			auto leftPos = DirectX::XMLoadFloat3(&left->GetTransform()->GetPosition());
			auto rightPos = DirectX::XMLoadFloat3(&right->GetTransform()->GetPosition());
			auto leftDistV = DirectX::XMVector3Length(playerPosition - leftPos);
			auto rightDistV = DirectX::XMVector3Length(playerPosition - rightPos);
			DirectX::XMFLOAT3 leftDist3{};
			DirectX::XMFLOAT3 rightDist3{};
			DirectX::XMStoreFloat3(&leftDist3, leftDistV);
			DirectX::XMStoreFloat3(&rightDist3, rightDistV);

			float leftDist = leftDist3.x;
			float rightDist = rightDist3.x;

			return leftDist < rightDist;
		});

	if (nearestItem != m_ItemsInReach.end() && *nearestItem != m_pHeldItem)
	{
		auto pSelectable = dynamic_cast<Selectable*>(*nearestItem);

		if (pSelectable && pSelectable->CanBeSelected())
		{
			pSelectable->Select();
			m_pSelectedItem = pSelectable;
		}
		else
		{
			if (m_pSelectedItem) m_pSelectedItem->Unselect();
			m_pSelectedItem = nullptr;
		}
	}
	else
	{
		if(m_pSelectedItem) m_pSelectedItem->Unselect();
		m_pSelectedItem = nullptr;
	}
}

void Chef::UpdateHeldItem() const
{
	if (!m_pHeldItem) return;

	using namespace DirectX;
	auto forwardV = DirectX::XMLoadFloat3(&m_pModel->GetTransform()->GetForward());
	forwardV *= m_HoldingOffset;
	auto playerPos = DirectX::XMLoadFloat3(&GetTransform()->GetPosition());

	DirectX::XMFLOAT3 heldPos{};
	DirectX::XMStoreFloat3(&heldPos, playerPos - forwardV);

	m_pHeldItem->GetTransform()->Translate(heldPos);
}

void Chef::StorePickUp(PickUp* pPickup)
{
	if(m_pHeldItem)
	{
		auto pItemRigid = m_pHeldItem->GetComponent<RigidBodyComponent>();
		pItemRigid->SetKinematic(false);
		pItemRigid->ClearForce();
		pItemRigid->ClearTorque();
		pItemRigid->SetLinearVelocity(physx::PxVec3{ 0,0,0 });
		
		m_pHeldItem = nullptr;

		m_pModel->GetAnimator()->SetAnimation(Animation::Idle);
		m_pModel->GetAnimator()->Play();
	}
	else
	{
		m_pHeldItem = pPickup;
		if(m_pHeldItem)
		{
			m_pHeldItem->GetComponent<RigidBodyComponent>()->SetKinematic(true);
			m_pHeldItem->GetTransform()->Rotate(0, 0, 0);

			auto pFmod = SoundManager::GetInstance()->GetSystem();
			auto res = pFmod->playSound(m_pPickup, 0, false, &m_pChannel);
			SoundManager::GetInstance()->ErrorCheck(res);

			m_pModel->GetAnimator()->SetAnimation(Animation::IdleHold);
			m_pModel->GetAnimator()->Play();
		}
	}
}

void Chef::InteractWith(Interactable* pInteractable)
{
	if (pInteractable)
	{
		pInteractable->Activate();
		m_isInteracting = true;
		m_pModel->GetAnimator()->SetAnimation(Animation::Chopping);
		m_pModel->GetAnimator()->Play();
	}
}

void Chef::StopInteractingWith(Interactable* pInteractable)
{
	if (pInteractable) pInteractable->Deactivate();
	m_isInteracting = false;
	m_pModel->GetAnimator()->SetAnimation(Animation::Idle);
	m_pModel->GetAnimator()->Play();
}
