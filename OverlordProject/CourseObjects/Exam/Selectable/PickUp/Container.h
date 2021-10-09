#pragma once
#include "../PickUp.h"

class Ingredient;

class Container final : public PickUp
{
public:
	enum class ContainerType
	{
		Plate,
		Pan
	};

	Container(ContainerType type, PostHighlight* pPostHighlight);
	~Container() override = default;

	bool AddIngredient(Ingredient* pIngredient);
	const std::vector<Ingredient*>& GetIngredients() const;

	void TransferIngredients(Container* pDestinationContainer);
	void DestroyIngredients();
	
	ContainerType GetType() const;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;
	void PostInitialize(const GameContext&) override;
	float GetMeshWidth() const override;
private:
	ContainerType m_type;

	std::vector<Ingredient*> m_ingredients;

	FMOD::Sound* m_pAddIngredient;
	FMOD::Sound* m_pTransfer;
	FMOD::Channel* m_pChannel;
};

