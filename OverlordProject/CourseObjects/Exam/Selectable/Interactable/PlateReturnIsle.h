#pragma once
#include <deque>

#include "../Interactable.h"

class Container;

class PlateReturnIsle final : public Interactable
{
public:
	PlateReturnIsle(const DirectX::XMFLOAT3& pos, PostHighlight* pHighlight, bool* pIsPaused);
	~PlateReturnIsle() override = default;

	void OnFoodDelivered();

	void Reset() override;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;
	void OnActivate() override;
	void OnDeactivate() override;
	bool CanStore(PickUp* pPickup) const override;

	float GetMeshWidth() const override;
	void OnTake() override;
private:
	const float m_PlateSpawnTimeAfterDeliver{ 10.f };
	std::vector<float> m_timesUntillPlateSpawn;

	const float m_offsetBetweenPlatesY{ 2.f };
	
	std::deque<Container*> m_pPlates;
	bool m_updateStored;

	FMOD::Sound* m_pReturnPlate;
	FMOD::Channel* m_pChannel;

	bool* m_pIsPaused;
};

