#pragma once
#include "GameObject.h"

class Selectable;
class Interactable;
class PickUp;
class ControllerComponent;
class ModelComponent;

class Chef final : public GameObject
{
public:
	Chef(bool* pPaused);
	virtual ~Chef() override = default;

	Chef(const Chef&) = delete;
	Chef& operator=(const Chef&) = delete;
	Chef(Chef&&) = delete;
	Chef& operator=(Chef&&) = delete;
	
	enum Input : int
	{
		Left = 1,
		Right = 2,
		Up = 3,
		Down = 4,
		Pickup = 5,
		Interact = 6,
		StopInteract = 7
	};

	void AddInputActions();

	void Reset() override;

protected:
	void Initialize(const GameContext&) override;
	void Update(const GameContext&) override;
	void PostInitialize(const GameContext&) override;
	
private:
	bool* m_pPaused;
	
	ModelComponent* m_pModel;
	ControllerComponent* m_pController;

	GameObject* m_pReachableCollider;

	//Physx Material
	const float m_StaticFriction{ 0.5f };
	const float m_DynamicFriction{ 0.5f };
	const float m_Restitution{ 0.2f };
	//Controller component
	const float m_ControllerRadius{ 5.f };
	const float m_ControllerHeight{ 13.f };

	float m_TotalYaw{};
	float m_RotationSpeed{450.f};

	float m_MaxRunVelocity{150.f},
		m_TerminalVelocity{200.f},
		m_RunAcceleration{50.f/0.3f},
		m_JumpAcceleration{9.81f/0.8f},
		m_RunVelocity{},
		m_JumpVelocity{};

	float m_RunVelocityPreviousFrame{};
	
	DirectX::XMFLOAT3 m_Velocity{};

	const float m_ReachableColliderOffset{ 10.f };
	const float m_HoldingOffset{ 3.5f };
	void CreateReachableCollider();

	std::vector<GameObject*> m_ItemsInReach;
	Selectable* m_pSelectedItem;
	PickUp* m_pHeldItem;
	bool m_isInteracting;
	
	void UpdateMovement(const GameContext&);
	void UpdateRotation(const GameContext&);
	void UpdateSelectedItem();
	void UpdateHeldItem() const;

	void StorePickUp(PickUp* pPickup);
	void InteractWith(Interactable* pInteractable);
	void StopInteractingWith(Interactable* pInteractable);
	
	enum Animation : int
	{
		Walk = 1,
		WalkHold = 2,
		Idle = 0,
		Chopping = 4,
		IdleHold = 3
	};

	FMOD::Sound* m_pPickup;
	FMOD::Sound* m_pWalking;
	FMOD::Channel* m_pChannel;
	FMOD::Channel* m_pWalkingChannel;
};

