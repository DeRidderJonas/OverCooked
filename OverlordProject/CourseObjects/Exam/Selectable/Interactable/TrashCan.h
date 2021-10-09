#pragma once
#include "../Interactable.h"

class TrashCan final : public Interactable
{
public:
	TrashCan(const DirectX::XMFLOAT3& position, PostHighlight* pPostHighlight);
	~TrashCan() override = default;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;

	void OnActivate() override;
	void OnDeactivate() override;

	float GetMeshWidth() const override;
	void OnStore() override;
private:
	FMOD::Sound* m_pTrashCan;
	FMOD::Channel* m_pChannel;
};

