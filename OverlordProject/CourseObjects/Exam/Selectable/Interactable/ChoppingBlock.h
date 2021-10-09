#pragma once
#include "../Interactable.h"

class ChoppingBlock final : public Interactable
{
public:
	ChoppingBlock(const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight);
	~ChoppingBlock() override = default;

	void Reset() override;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;

	void OnActivate() override;
	void OnDeactivate() override;
	float GetMeshWidth() const override;
	void OnTake() override;
private:
	ModelComponent* m_pChoppingBlock;

	FMOD::Sound* m_pChopping;
	FMOD::Channel* m_pChannel;
};

