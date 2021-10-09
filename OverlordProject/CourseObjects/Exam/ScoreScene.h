#pragma once
#include "GameScene.h"

class SpriteFont;
class SpriteComponent;

class ScoreScene final : public GameScene
{
public:
	ScoreScene();
	~ScoreScene() override = default;

	void SetScore(int score);
protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;
	void SceneDeactivated() override;
private:
	int m_Score;
	DirectX::XMFLOAT2 m_ScorePosition{ 640,320 };
	SpriteFont* m_pFont = nullptr;

	DirectX::XMFLOAT2 m_ButtonYValues{ 500,580 };
	DirectX::XMFLOAT2 m_BackXValues{ 270, 490 };
	DirectX::XMFLOAT2 m_ExitXValues{ 760, 980 };
	
	const float m_LeftThumbInputCooldown{ 0.3f };
	float m_LeftThumbInputCooldownRemaining;

	void MenuGoLeft();
	void MenuGoRight();
	void GoBack();
	void ExitGame();

	SpriteComponent* m_pSpriteComponent;
	
	enum Button
	{
		Back,
		Exit,
		None
	};

	Button m_Selected;

	enum Input : int
	{
		MenuItemLeft = 0,
		MenuItemRight = 1,
		SelectMenuItem = 2,
		ClickMenuItem = 3
	};

	FMOD::Sound* m_pSelect;
	FMOD::Sound* m_pConfirm;
	FMOD::Channel* m_pChannel;

	FMOD::Sound* m_pBackground;
	FMOD::Channel* m_pBackgroundChannel;
};

