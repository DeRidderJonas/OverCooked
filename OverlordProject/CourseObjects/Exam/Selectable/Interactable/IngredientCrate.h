#pragma once
#include "../Interactable.h"
#include "../PickUp/Ingredient.h"
class IngredientCrate final : public Interactable
{
public:
	IngredientCrate(Ingredient::IngredientType type, const DirectX::XMFLOAT3& pos, PostHighlight* pPostHighlight, bool* pIsPaused);
	~IngredientCrate() override = default;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;

	void OnActivate() override;
	void OnDeactivate() override;
	float GetMeshWidth() const override;
private:
	Ingredient::IngredientType m_type;
	FMOD::Sound* m_pSoundOpenCrate{ nullptr };
	FMOD::Channel* m_pChannel{ nullptr };

	bool* m_pIsPaused;
};

