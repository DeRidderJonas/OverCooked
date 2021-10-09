#pragma once
#include "../Interactable.h"

class ParticleEmitterComponent;

class Stove final : public Interactable
{
public:
	Stove(const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight);
	~Stove() override = default;

	void Reset() override;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;
	void OnActivate() override;
	void OnDeactivate() override;
	float GetMeshWidth() const override;
	bool CanStore(PickUp* pPickup) const override;
	void OnStore() override;
	void OnTake() override;
	void PostInitialize(const GameContext&) override;
private:
	ParticleEmitterComponent* m_pParticleEmitter;

	FMOD::Sound* m_pCooking;
	FMOD::Channel* m_pChannel;
};

