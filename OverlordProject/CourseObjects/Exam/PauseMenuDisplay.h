#pragma once
#include "GameObject.h"

class SpriteComponent;

class PauseMenuDisplay final : public GameObject
{
public:
	PauseMenuDisplay(bool* pIsPaused);
	~PauseMenuDisplay() override;
protected:
	void Initialize(const GameContext&) override;
	void Draw(const GameContext&) override;
	void Update(const GameContext&) override;
private:
	DirectX::XMFLOAT2 m_ButtonXValues{ 520, 810 };
	DirectX::XMFLOAT2 m_MainYValues{ 140, 230 };
	DirectX::XMFLOAT2 m_RestartYValues{ 285, 380 };
	DirectX::XMFLOAT2 m_ExitYValues{ 440, 522 };
	
	bool* m_pIsPaused;
	bool m_PausedLastFrame;

	const float m_LeftThumbInputCooldown{ 0.3f };
	float m_LeftThumbInputCooldownRemaining;
	
	enum class Button
	{
		MainMenu,
		Restart,
		Exit,
		None
	};

	Button m_Selected;

	enum Input
	{
		PausedMenuUp = 0,
		PausedMenuDown = 1,
		PausedMenuSelect = 2,
		PausedMenuClick = 3,
		EscapeMenu = 4
	};

	SpriteComponent* m_pSpriteComponent;

	FMOD::Sound* m_pSelect;
	FMOD::Sound* m_pConfirm;
	FMOD::Channel* m_pChannel;

	void OnPaused();
	void OnUnPaused();
	void SetupInput();

	void MenuGoUp();
	void MenuGoDown();
	void ToMainMenu();
	void Restart();
	void ExitGame();
};

