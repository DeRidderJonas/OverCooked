#pragma once
#include "../Selectable.h"

class PostHighlight;
class PickUp;
class ModelComponent;

class Interactable : public Selectable
{
public:
	Interactable(const DirectX::XMFLOAT3& position, PostHighlight* pPostHighlight, const DirectX::XMFLOAT3& storedPosOffset);
	~Interactable() override = default;

	bool Store(PickUp* pPickup, bool& keepHoldingItem);
	bool Store(PickUp* pPickup);
	PickUp* Take();
	void Activate();
	void Deactivate();

	void Reset() override;
protected:
	void Initialize(const GameContext&) override = 0;
	void PostInitialize(const GameContext&) override;
	void Draw(const GameContext&) override = 0;
	void Update(const GameContext&) override = 0;

	ModelComponent* m_pModel;
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_StoredPosOffset;

	virtual void OnActivate() = 0;
	virtual void OnDeactivate() = 0;
	virtual bool CanStore(PickUp* pPickup) const;
	virtual void OnStore();
	virtual void OnTake();
	
	PickUp* m_pStored;
	bool m_isActive;
};

