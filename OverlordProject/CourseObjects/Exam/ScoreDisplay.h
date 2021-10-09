#pragma once
#include "GameObject.h"

class SpriteFont;

class ScoreDisplay final : public GameObject
{
public:
	ScoreDisplay(int* pScore);
	~ScoreDisplay() override = default;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void PostDraw(const GameContext&) override;
	void Update(const GameContext&) override;
private:
	SpriteFont* m_pFont = nullptr;
	DirectX::XMFLOAT2 m_numberOffset{ 30,35 };
	
	int* m_pScore;
};

