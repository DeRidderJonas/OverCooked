#pragma once
#include "../Interactable.h"

class PlateReturnIsle;

class DeliveryIsle final : public Interactable
{
public:
	DeliveryIsle(const DirectX::XMFLOAT3& position, PostHighlight* pPostHighlight, PlateReturnIsle* pPlateReturnIsle);
	~DeliveryIsle() override = default;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;

	void OnActivate() override;
	void OnDeactivate() override;

	float GetMeshWidth() const override;
	bool CanStore(PickUp* pPickup) const override;
private:
	PlateReturnIsle* m_pPlateReturnIsle;

	FMOD::Sound* m_pDeliver;
	FMOD::Channel* m_pChannel;
};

