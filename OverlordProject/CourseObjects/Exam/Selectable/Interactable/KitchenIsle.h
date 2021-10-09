#pragma once
#include "../Interactable.h"

class KitchenIsle final : public Interactable
{
public:
	KitchenIsle(const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight, bool spawnWithPlate = false);
	~KitchenIsle() override = default;

	void Reset() override;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;
	void OnActivate() override;
	void OnDeactivate() override;
	float GetMeshWidth() const override;
	void PostInitialize(const GameContext&) override;
private:
	bool m_OriginalSpawnWithPlate;
	bool m_SpawnPlate;
};

