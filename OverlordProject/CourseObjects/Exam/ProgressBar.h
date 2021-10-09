#pragma once
#include "GameObject.h"

class SpriteComponent;

class ProgressBar final : public GameObject
{
public:
	ProgressBar();
	~ProgressBar() override = default;

	void SetPercentage(float percentage);
	void SetBarColor(bool isGreen);
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;
	void PostInitialize(const GameContext&) override;
private:
	float m_Percentage;

	DirectX::XMFLOAT2 m_BarOffset{ 2,1.5f };
	GameObject* m_pChild;
	SpriteComponent* m_pSpriteComponent;
};

