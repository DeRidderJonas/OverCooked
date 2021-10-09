#pragma once
#include <set>

class PostProcessingMaterial;
class GameObject;
class SceneManager;
class CameraComponent;
class PhysxProxy;

class GameScene
{
public:
	GameScene(std::wstring sceneName);
	GameScene(const GameScene& other) = delete;
	GameScene(GameScene&& other) noexcept = delete;
	GameScene& operator=(const GameScene& other) = delete;
	GameScene& operator=(GameScene&& other) noexcept = delete;
	virtual ~GameScene();

	void AddChild(GameObject* obj);
	void RemoveChild(GameObject* obj, bool deleteObject = true);

	const GameContext& GetGameContext() const { return m_GameContext; }

	PhysxProxy* GetPhysxProxy() const { return m_pPhysxProxy; }
	void SetActiveCamera(CameraComponent* pCameraComponent);

	void ResetChildren();
	
	template<typename T>
	void RemoveChildrenOfType()
	{
		for(auto pChild : m_pChildren)
		{
			auto pType = dynamic_cast<T*>(pChild);
			if (pType)
				RemoveChild(pChild);
		}
	}
protected:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void SceneActivated() {}
	virtual void SceneDeactivated() {}

	void AddPostProcessingEffect(PostProcessingMaterial* pEffect);
	void RemovePostProcessingEffect(PostProcessingMaterial* pEffect, bool deleteEffect = true);
private:
	friend class SceneManager;

	void RootInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void RootUpdate();
	void RootDraw();
	void RootSceneActivated();
	void RootSceneDeactivated();
	void RootWindowStateChanged(int state, bool active) const;

	void RootAddChildren();
	void RootRemoveChildren();
	
	std::vector<PostProcessingMaterial*> m_PostProcessingMaterials{};

	std::vector<GameObject*> m_pChildren;
	std::vector<GameObject*> m_pToAdd;
	std::set<std::pair<GameObject*, bool>> m_pToRemove;
	GameContext m_GameContext;
	bool m_IsInitialized;
	std::wstring m_SceneName;
	CameraComponent *m_pDefaultCamera, *m_pActiveCamera;
	PhysxProxy* m_pPhysxProxy;
};
