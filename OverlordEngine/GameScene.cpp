#include "stdafx.h"
#include "GameScene.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "Prefabs.h"
#include "Components.h"
#include "DebugRenderer.h"
#include "RenderTarget.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "PhysxProxy.h"
#include "SoundManager.h"
#include <algorithm>

#include "PostProcessingMaterial.h"

GameScene::GameScene(std::wstring sceneName):
	m_pChildren(std::vector<GameObject*>()),
	m_GameContext(GameContext()),
	m_IsInitialized(false),
	m_SceneName(std::move(sceneName)),
	m_pDefaultCamera(nullptr),
	m_pActiveCamera(nullptr),
	m_pPhysxProxy(nullptr)
{
}

GameScene::~GameScene()
{
	SafeDelete(m_GameContext.pGameTime);
	SafeDelete(m_GameContext.pInput);
	SafeDelete(m_GameContext.pMaterialManager);
	SafeDelete(m_GameContext.pShadowMapper);

	for (auto pChild : m_pChildren)
	{
		SafeDelete(pChild);
	}

	for(auto pToAdd : m_pToAdd)
	{
		SafeDelete(pToAdd);
	}
	
	SafeDelete(m_pPhysxProxy);

	for(auto pEffect : m_PostProcessingMaterials)
	{
		delete pEffect;
	}
}

void GameScene::AddChild(GameObject* obj)
{
#if _DEBUG
	if (obj->m_pParentScene)
	{
		if (obj->m_pParentScene == this)
			Logger::LogWarning(L"GameScene::AddChild > GameObject is already attached to this GameScene");
		else
			Logger::LogWarning(
				L"GameScene::AddChild > GameObject is already attached to another GameScene. Detach it from it's current scene before attaching it to another one.");

		return;
	}

	if (obj->m_pParentObject)
	{
		Logger::LogWarning(
			L"GameScene::AddChild > GameObject is currently attached to a GameObject. Detach it from it's current parent before attaching it to another one.");
		return;
	}
#endif

	obj->m_pParentScene = this;
	obj->RootInitialize(m_GameContext);
	m_pToAdd.push_back(obj);
}

void GameScene::RemoveChild(GameObject* obj, bool deleteObject)
{
	const auto it = find(m_pChildren.begin(), m_pChildren.end(), obj);

#if _DEBUG
	if (it == m_pChildren.end())
	{
		Logger::LogWarning(L"GameScene::RemoveChild > GameObject to remove is not attached to this GameScene!");
		return;
	}
#endif

	m_pToRemove.insert(std::make_pair(obj, deleteObject));
}

void GameScene::RootInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	if (m_IsInitialized)
		return;

	//Create DefaultCamera
	auto freeCam = new FreeCamera();
	freeCam->SetRotation(30, 0);
	freeCam->GetTransform()->Translate(0, 50, -80);
	AddChild(freeCam);
	m_pDefaultCamera = freeCam->GetComponent<CameraComponent>();
	m_pActiveCamera = m_pDefaultCamera;
	m_GameContext.pCamera = m_pDefaultCamera;

	//Create GameContext
	m_GameContext.pGameTime = new GameTime();
	m_GameContext.pGameTime->Reset();
	m_GameContext.pGameTime->Stop();

	m_GameContext.pInput = new InputManager();
	InputManager::Initialize();

	m_GameContext.pMaterialManager = new MaterialManager();
	m_GameContext.pShadowMapper = new ShadowMapRenderer();

	m_GameContext.pDevice = pDevice;
	m_GameContext.pDeviceContext = pDeviceContext;

	//Initialize ShadowMapper
	m_GameContext.pShadowMapper->Initialize(m_GameContext);

	// Initialize Physx
	m_pPhysxProxy = new PhysxProxy();
	m_pPhysxProxy->Initialize(this);

	//User-Scene Initialize
	Initialize();

	//Root-Scene Initialize
	for (auto pChild : m_pChildren)
	{
		pChild->RootInitialize(m_GameContext);
	}

	for(auto pEffect : m_PostProcessingMaterials)
	{
		pEffect->Initialize(m_GameContext);
	}
	
	m_IsInitialized = true;
}

void GameScene::RootUpdate()
{
	m_GameContext.pGameTime->Update();
	m_GameContext.pInput->Update();
	m_GameContext.pCamera = m_pActiveCamera;

	SoundManager::GetInstance()->GetSystem()->update();

	//User-Scene Update
	Update();

	RootAddChildren();

	//Root-Scene Update
	for (auto pChild : m_pChildren)
	{
		pChild->RootUpdate(m_GameContext);
	}

	RootRemoveChildren();

	std::sort(m_PostProcessingMaterials.begin(), m_PostProcessingMaterials.end(), [](PostProcessingMaterial* pLeft, PostProcessingMaterial* pRight)
		{
			return pLeft->GetRenderIndex() < pRight->GetRenderIndex();
		});

	m_pPhysxProxy->Update(m_GameContext);
}

void GameScene::RootDraw()
{
	//object-Scene SHADOW_PASS - start by setting the correct render target, render all to shadow map and end by reset default render target
	m_GameContext.pShadowMapper->Begin(m_GameContext);
	for(auto pChild : m_pChildren)
	{
		pChild->RootDrawShadowMap(m_GameContext);
	}
	m_GameContext.pShadowMapper->End(m_GameContext);
	
	//User-Scene Draw
	Draw();

	//Object-Scene Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootDraw(m_GameContext);
	}

	//Object-Scene Post-Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootPostDraw(m_GameContext);
	}

	//Draw PhysX
	m_pPhysxProxy->Draw(m_GameContext);

	//Draw Debug Stuff
	DebugRenderer::Draw(m_GameContext);
	SpriteRenderer::GetInstance()->Draw(m_GameContext);
	TextRenderer::GetInstance()->Draw(m_GameContext);

	if (m_PostProcessingMaterials.empty())
		return;

	auto pGame = SceneManager::GetInstance()->GetGame();
	auto INIT_RT = pGame->GetRenderTarget();
	auto PREV_RT = INIT_RT;

	for(auto pEffect : m_PostProcessingMaterials)
	{
		auto TEMP_RT = pEffect->GetRenderTarget();
		pGame->SetRenderTarget(TEMP_RT);
		pEffect->Draw(m_GameContext, PREV_RT);
		PREV_RT = TEMP_RT;
	}

	pGame->SetRenderTarget(INIT_RT);
	SpriteRenderer::GetInstance()->DrawImmediate(m_GameContext, PREV_RT->GetShaderResourceView(), {});
	
}

void GameScene::RootSceneActivated()
{
	//Start Timer
	m_GameContext.pGameTime->Start();
	SceneActivated();
}

void GameScene::RootSceneDeactivated()
{
	//Stop Timer
	m_GameContext.pGameTime->Stop();
	SceneDeactivated();
}

void GameScene::RootWindowStateChanged(int state, bool active) const
{
	//TIMER
	if (state == 0)
	{
		if (active)m_GameContext.pGameTime->Start();
		else m_GameContext.pGameTime->Stop();
	}
}

void GameScene::RootAddChildren()
{
	m_pChildren.insert(m_pChildren.end(), m_pToAdd.begin(), m_pToAdd.end());
	m_pToAdd.clear();
}

void GameScene::RootRemoveChildren()
{
	m_pChildren.erase(std::remove_if(m_pChildren.begin(), m_pChildren.end(), [this](GameObject* pGo)
		{
			return std::find_if(this->m_pToRemove.begin(), this->m_pToRemove.end(), [&pGo](const std::pair<GameObject*,bool>& pair)
				{
					return pair.first == pGo;
				})
					!= this->m_pToRemove.end();
		}), m_pChildren.end());

	for(auto& pair : m_pToRemove)
	{
		if (pair.second)
			delete pair.first;
		else
			pair.first->m_pParentScene = nullptr;
	}
	
	m_pToRemove.clear();
}

void GameScene::AddPostProcessingEffect(PostProcessingMaterial* pEffect)
{
	auto findIt = std::find(m_PostProcessingMaterials.begin(), m_PostProcessingMaterials.end(), pEffect);
	if (findIt != m_PostProcessingMaterials.end())
		return;
	
	m_PostProcessingMaterials.push_back(pEffect);
	pEffect->Initialize(GetGameContext());
}

void GameScene::RemovePostProcessingEffect(PostProcessingMaterial* pEffect, bool deleteEffect)
{
	auto findIt = std::find(m_PostProcessingMaterials.begin(), m_PostProcessingMaterials.end(), pEffect);

	if (findIt == m_PostProcessingMaterials.end())
		return;

	if(deleteEffect) delete *findIt;
	m_PostProcessingMaterials.erase(findIt);
}

void GameScene::SetActiveCamera(CameraComponent* pCameraComponent)
{
	if (m_pActiveCamera != nullptr)
		m_pActiveCamera->m_IsActive = false;

	m_pActiveCamera = (pCameraComponent) ? pCameraComponent : m_pDefaultCamera;
	m_pActiveCamera->m_IsActive = true;
}

void GameScene::ResetChildren()
{
	for(auto pChild : m_pChildren)
	{
		pChild->Reset();
	}
}
