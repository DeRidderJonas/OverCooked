#pragma once
#include "GameScene.h"

class ControlsSchemaScene final : public GameScene
{
public:
	ControlsSchemaScene();
	~ControlsSchemaScene() override = default;
protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void SceneActivated() override;
	void SceneDeactivated() override;
private:
	const int m_ContinueInputId{ 0 };
};

