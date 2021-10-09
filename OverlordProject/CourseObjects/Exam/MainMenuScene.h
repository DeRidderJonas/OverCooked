#pragma once
#include "GameScene.h"

class SpriteComponent;

class MainMenuScene final : public GameScene
{
public:
	MainMenuScene();
	~MainMenuScene() override = default;
protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;
	void SceneDeactivated() override;
private:
	const float m_LeftThumbInputCooldown{ 0.3f };
	float m_LeftThumbInputCooldownRemaining;
	
	void MenuGoLeft();
	void MenuGoRight();
	void StartGame();
	void ExitGame();

	DirectX::XMFLOAT2 m_ButtonsYValues{ 500, 580 };
	DirectX::XMFLOAT2 m_StartButtonXValues{270,490};
	DirectX::XMFLOAT2 m_ExitButtonPosXValues{ 760,980 };

	SpriteComponent* m_pSpriteComponent;
	
	float m_lifetime;
	
	enum Button
	{
		Start,
		Exit,
		None
	};

	Button m_SelectedButton;

	enum Input : int
	{
		MenuItemGoLeft = 0,
		MenuItemGoRight = 1,
		SelectMenuItem = 2,
		ClickMenuItem = 3
	};

	FMOD::Sound* m_pSelect;
	FMOD::Sound* m_pConfirm;
	FMOD::Channel* m_pChannel;

	FMOD::Sound* m_pBackground;
	FMOD::Channel* m_pBackgroundChannel;
};

