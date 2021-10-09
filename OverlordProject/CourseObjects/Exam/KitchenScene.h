#pragma once
#include "GameScene.h"

class ScoreDisplay;
class Recipe;
class Container;
class PostHighlight;
class Chef;

class KitchenScene final : public GameScene
{
public:
	KitchenScene();
	~KitchenScene() override = default;

	KitchenScene(const KitchenScene&) = delete;
	KitchenScene& operator=(const KitchenScene&) = delete;
	KitchenScene(KitchenScene&&) = delete;
	KitchenScene& operator=(KitchenScene&&) = delete;

	void Deliver(Container* pContainer);
	void Reset();
	void OnUnpaused();
protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;
	void SceneDeactivated() override;
private:
	Chef* m_pPlayer;
	CameraComponent* m_pCamera;
	PostHighlight* m_pPostHighlight;

	bool m_IsPaused;
	int m_PauseInputId{ 10 };

	//Camera Component
	const float m_CameraDistance{ 175.f };

	float m_FpsInterval{ 1.f };

	std::vector<Recipe*> m_Recipes;
	float m_NewRecipeCooldown{ 25.f };
	float m_TimeSinceLastRecipe{ 0.f };

	const float m_RecipeIngredientWidth{ 60.f };
	const DirectX::XMFLOAT3 m_RecipeStartPos{ 10.f,20.f, .97f };
	
	ScoreDisplay* m_pScoreDisplay;
	int m_score;

	float m_InitialTime{ 60 * 5 };
	float m_Timer;

	FMOD::Sound* m_pAmbient;
	FMOD::Channel* m_pChannel;
	
	void CreateMaterials() const;
	void CreateFloor();
	void CreateLevelBoundaries();
	void CreateLevel();

	void AddDecorations();

	void AddNewRecipe();
};

