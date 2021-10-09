#pragma once
#include "../PickUp.h"

class ProgressBar;
class ColliderComponent;
class RigidBodyComponent;
class SpriteFont;

class Ingredient final : public PickUp
{
public:
	enum class IngredientType
	{
		Cabbage,
		Tomato,
		Steak
	};
	
	Ingredient(IngredientType type, PostHighlight* pPostHighlight, bool* pIsPaused);
	~Ingredient() override;

	bool IsReady() const;
	
	void Chop();
	void StopChopping();
	bool IsChopped() const;

	void Cook();
	void StopCooking();
	bool IsCooked() const;
	bool NeedsCooking() const;

	void OnAddedToContainer();
	void RemoveProgressBar();

	IngredientType GetType() const;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;

	float GetMeshWidth() const override;
private:
	IngredientType m_Type;

	const float m_TimeToChop{ 1.5f };
	float m_ChopPercent;
	bool m_isChopping;
	
	const float m_TimeToCook{ 1.5f };
	float m_CookPercent;
	bool m_isCooking;
	bool m_NeedsCooking;

	SpriteFont* m_pFont;

	RigidBodyComponent* m_pRigidBody;
	ColliderComponent* m_pCollider;

	bool* m_pIsPaused;

	DirectX::XMFLOAT2 m_ProgressBarOffset{ -25,-25 };
	ProgressBar* m_pProgressBar;
};

