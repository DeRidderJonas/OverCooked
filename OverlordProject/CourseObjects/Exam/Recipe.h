#pragma once
#include "Selectable/PickUp/Ingredient.h"
#include "Selectable/PickUp/Container.h"

class Recipe final : public GameObject
{
public:
	Recipe(const std::vector<Ingredient::IngredientType>& ingredients, bool* pIsPaused, float time, float lateTime = 0);
	~Recipe() override = default;

	bool operator==(const Container& other);

	bool IsLate() const;
	bool IsExpired() const;
	int GetAmountOfIngredients() const;
protected:
	void Initialize(const GameContext&) override;
	void Update(const GameContext&) override;
	void Draw(const GameContext&) override;
	void PostInitialize(const GameContext&) override;
private:
	float m_InitialTime;
	float m_LateTime;
	SpriteFont* m_pFont = nullptr;

	std::vector<Ingredient::IngredientType> m_Ingredients;

	std::vector<GameObject*> m_children{};
	float m_TimeRemaining;
	bool m_isLate;

	bool* m_pIsPaused;

	ProgressBar* m_pProgressBar;
};

