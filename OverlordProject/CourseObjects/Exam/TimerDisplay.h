#pragma once
#include "GameObject.h"

class SpriteFont;

class TimerDisplay final : public GameObject
{
public:
	TimerDisplay(float* pTimer);
	~TimerDisplay() override = default;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
private:
	SpriteFont* m_pFont = nullptr;
	DirectX::XMFLOAT2 m_TimerOffset{ 13,40 };
	
	float* m_pTimer;
};

