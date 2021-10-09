#include "stdafx.h"
#include "KitchenScene.h"

#include <numeric>

#include "Chair.h"
#include "Chef.h"
#include "ColliderComponent.h"
#include "ContentManager.h"
#include "Selectable/PickUp/Ingredient.h"
#include "Selectable/Interactable/ChoppingBlock.h"
#include "Selectable/Interactable/KitchenIsle.h"
#include "Selectable/Interactable/Stove.h"
#include "ModelComponent.h"
#include "PauseMenuDisplay.h"
#include "PhysxManager.h"
#include "PhysxProxy.h"
#include "Recipe.h"
#include "RigidBodyComponent.h"
#include "SceneManager.h"
#include "ScoreDisplay.h"
#include "ScoreScene.h"
#include "SoundManager.h"
#include "Table.h"
#include "TimerDisplay.h"
#include "TransformComponent.h"
#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "../../Materials/Shadow/SkinnedDiffuseMaterial_Shadow.h"
#include "Selectable/PickUp/Container.h"
#include "PP/PostHighlight.h"
#include "Selectable/Interactable/DeliveryIsle.h"
#include "Selectable/Interactable/IngredientCrate.h"
#include "Selectable/Interactable/PlateReturnIsle.h"
#include "Selectable/Interactable/TrashCan.h"


#define FPS_COUNTER 1

KitchenScene::KitchenScene()
	: GameScene(L"KitchenScene")
	, m_pPlayer(nullptr)
	, m_pCamera(nullptr)
	, m_FpsInterval(FPS_COUNTER)
	, m_pPostHighlight(nullptr)
	, m_score(0)
	, m_Timer(m_InitialTime)
	, m_TimeSinceLastRecipe(m_NewRecipeCooldown - 5.f)
	, m_IsPaused(false)
{
}

void KitchenScene::Deliver(Container* pContainer)
{
	RemoveChild(pContainer);
	for(auto& pIngredient : pContainer->GetIngredients())
	{
		RemoveChild(pIngredient);
	}

	auto recipeIt = std::find_if(m_Recipes.begin(), m_Recipes.end(), [&pContainer](Recipe* pRecipe) {return *pRecipe == *pContainer; });

	if (recipeIt == m_Recipes.end())
		return;

	RemoveChild(*recipeIt);

	if (!(*recipeIt)->IsLate())
		m_score += 50;
	else
	{
		if ((*recipeIt)->IsExpired())
			m_score += 10;
		else
			m_score += 25;
	}

	m_Recipes.erase(std::remove(m_Recipes.begin(), m_Recipes.end(), *recipeIt), m_Recipes.end());

	int offset{};
	for(auto& pRecipe : m_Recipes)
	{
		pRecipe->GetTransform()->Translate(offset * m_RecipeIngredientWidth + m_RecipeStartPos.x, m_RecipeStartPos.y, m_RecipeStartPos.z);
		offset += pRecipe->GetAmountOfIngredients();
	}
}

void KitchenScene::Reset()
{
	m_pPlayer->GetTransform()->Translate(0, 2, 0);
	m_score = 0;
	m_Timer = m_InitialTime;
	m_IsPaused = false;
	m_TimeSinceLastRecipe = m_NewRecipeCooldown - 5.f;

	for(auto pRecipes : m_Recipes)
	{
		RemoveChild(pRecipes);
	}
	m_Recipes.clear();

	ResetChildren();
	
	RemoveChildrenOfType<PickUp>();
}

void KitchenScene::OnUnpaused()
{
	m_IsPaused = false;
	GetGameContext().pInput->ClearInputActions();

	m_pPlayer->AddInputActions();

	GetGameContext().pInput->AddInputAction({ m_PauseInputId, InputTriggerState::Pressed, VK_ESCAPE, -1, 0x0010 });

	AddPostProcessingEffect(m_pPostHighlight);
}

void KitchenScene::Initialize()
{
	//GetPhysxProxy()->EnablePhysxDebugRendering(true);
	auto gameContext = GetGameContext();

	m_pPostHighlight = new PostHighlight();
	AddPostProcessingEffect(m_pPostHighlight);
	
	gameContext.pShadowMapper->SetLight({ -155.6139526f,66.1346436f,-111.1850471f }, { 0.740129888f, -0.597205281f, 0.309117377f });

	CreateMaterials();
	CreateFloor();
	CreateLevel();
	AddDecorations();

	//Player
	m_pPlayer = new Chef(&m_IsPaused);
	AddChild(m_pPlayer);
	m_pPlayer->GetTransform()->Scale(0.1f, 0.1f, 0.1f);

	//CameraController
	m_pCamera = new CameraComponent();
	auto cameraGO = new GameObject();
	cameraGO->AddComponent(m_pCamera);
	AddChild(cameraGO);
	cameraGO->GetTransform()->Translate(0, m_CameraDistance, 0);
	cameraGO->GetTransform()->Rotate(90, 0, 0);
	m_pCamera->SetActive();

	//Score Display
	m_pScoreDisplay = new ScoreDisplay(&m_score);
	m_pScoreDisplay->GetTransform()->Translate(40, 600, .95f);
	AddChild(m_pScoreDisplay);

	//Timer display
	auto pTimerDisplay = new TimerDisplay(&m_Timer);
	pTimerDisplay->GetTransform()->Translate(1150, 605, 0.95f);
	AddChild(pTimerDisplay);

	//Sound
	auto pFmod = SoundManager::GetInstance()->GetSystem();
	auto res = pFmod->createSound("./Resources/Sounds/KitchenAmbient.mp3", FMOD_LOOP_NORMAL, 0, &m_pAmbient);
	SoundManager::GetInstance()->ErrorCheck(res);
	res = pFmod->playSound(m_pAmbient, 0, true, &m_pChannel);
	SoundManager::GetInstance()->ErrorCheck(res);

	//Pause Menu
	auto pPauseMenu = new PauseMenuDisplay(&m_IsPaused);
	pPauseMenu->GetTransform()->Translate(450, 80, 0.95f);
	AddChild(pPauseMenu);
}

void KitchenScene::Update()
{
	auto gameContext = GetGameContext();
	float deltaTime{ gameContext.pGameTime->GetElapsed() };
	
	auto playerPos = m_pPlayer->GetTransform()->GetPosition();
	auto cameraPos = m_pCamera->GetTransform()->GetPosition();
	
	m_pCamera->GetTransform()->Translate(playerPos.x, cameraPos.y, playerPos.z);

	m_FpsInterval += deltaTime;
	if (m_FpsInterval >= FPS_COUNTER)
	{
		m_FpsInterval -= FPS_COUNTER;
		Logger::LogFormat(LogLevel::Info, L"FPS: %i", gameContext.pGameTime->GetFPS());
	}

	if (m_IsPaused)
		return;
	
	m_TimeSinceLastRecipe += deltaTime;
	if(m_TimeSinceLastRecipe >= m_NewRecipeCooldown)
	{
		m_TimeSinceLastRecipe -= m_NewRecipeCooldown;
		AddNewRecipe();
	}

	m_Timer -= deltaTime;
	if (m_Timer <= 0)
	{
		m_Timer = 0.f;
		SceneManager::GetInstance()->SetActiveGameScene(L"ScoreScene");
		auto pScoreScene = dynamic_cast<ScoreScene*>(SceneManager::GetInstance()->GetScene(L"ScoreScene"));
		if(pScoreScene)
		{
			pScoreScene->SetScore(m_score);
		}
		Reset();
	}

	if(gameContext.pInput->IsActionTriggered(m_PauseInputId))
	{
		m_IsPaused = true;
		RemovePostProcessingEffect(m_pPostHighlight, false);
	}
}

void KitchenScene::Draw()
{
	
}

void KitchenScene::SceneActivated()
{
	GetGameContext().pInput->ClearInputActions();
	
	m_pPlayer->AddInputActions();

	GetGameContext().pInput->AddInputAction({ m_PauseInputId, InputTriggerState::Pressed, VK_ESCAPE, -1, 0x0010 });
	
	if(m_pChannel)
		m_pChannel->setPaused(false);

	Reset();
}

void KitchenScene::SceneDeactivated()
{
	GetGameContext().pInput->ClearInputActions();
	if(m_pChannel)
		m_pChannel->setPaused(true);
}

void KitchenScene::CreateMaterials() const
{
	auto gameContext = GetGameContext();

	auto diffuseMat = new SkinnedDiffuseMaterial_Shadow();
	diffuseMat->SetDiffuseTexture(L"./Resources/Textures/Chef_Diffuse.png");
	diffuseMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	gameContext.pMaterialManager->AddMaterial(diffuseMat, 0);
	
	auto groundMat = new DiffuseMaterial_Shadow();
	groundMat->SetDiffuseTexture(L"./Resources/Textures/PlanksTiled.tif");
	groundMat->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	gameContext.pMaterialManager->AddMaterial(groundMat, 1);
	
	auto isleMat = new DiffuseMaterial_Shadow();
	isleMat->SetDiffuseTexture(L"./Resources/Textures/KitchenIsle_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(isleMat, 2);

	auto stoveMat = new DiffuseMaterial_Shadow();
	stoveMat->SetDiffuseTexture(L"./Resources/Textures/Stove_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(stoveMat, 3);

	auto choppingMat = new DiffuseMaterial_Shadow();
	choppingMat->SetDiffuseTexture(L"./Resources/Textures/ChoppingBlock_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(choppingMat, 4);
	
	auto cabbageMat = new DiffuseMaterial_Shadow();
	cabbageMat->SetDiffuseTexture(L"./Resources/Textures/Cabbage_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(cabbageMat, 5);

	auto tomatoMat = new DiffuseMaterial_Shadow();
	tomatoMat->SetDiffuseTexture(L"./Resources/Textures/Tomato_Diffuse.jpg");
	gameContext.pMaterialManager->AddMaterial(tomatoMat, 6);

	auto steakMat = new DiffuseMaterial_Shadow();
	steakMat->SetDiffuseTexture(L"./Resources/Textures/Steak_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(steakMat, 7);

	auto panMat = new DiffuseMaterial_Shadow();
	panMat->SetDiffuseTexture(L"./Resources/Textures/Pan_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(panMat, 8);

	auto trashCanMat = new DiffuseMaterial_Shadow();
	trashCanMat->SetDiffuseTexture(L"./Resources/Textures/TrashCan_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(trashCanMat, 9);

	auto deliverMat = new DiffuseMaterial_Shadow();
	deliverMat->SetDiffuseTexture(L"./Resources/Textures/DeliveryIsle_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(deliverMat, 10);
	
	auto crateTomatoMat = new DiffuseMaterial_Shadow();
	crateTomatoMat->SetDiffuseTexture(L"./Resources/Textures/Crate_Tomato_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(crateTomatoMat, 11);
	
	auto crateSteakMat = new DiffuseMaterial_Shadow();
	crateSteakMat->SetDiffuseTexture(L"./Resources/Textures/Crate_Steak_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(crateSteakMat, 12);
	
	auto crateCabbageMat = new DiffuseMaterial_Shadow();
	crateCabbageMat->SetDiffuseTexture(L"./Resources/Textures/Crate_Cabbage_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(crateCabbageMat, 13);

	auto returnMat = new DiffuseMaterial_Shadow();
	returnMat->SetDiffuseTexture(L"./Resources/Textures/ReturnIsle_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(returnMat, 14);

	auto tableMat = new DiffuseMaterial_Shadow();
	tableMat->SetDiffuseTexture(L"./Resources/Textures/Table_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(tableMat, 15);

	auto chairMat = new DiffuseMaterial_Shadow();
	chairMat->SetDiffuseTexture(L"./Resources/Textures/Chair_Diffuse.png");
	gameContext.pMaterialManager->AddMaterial(chairMat, 16);
}

void KitchenScene::CreateFloor()
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();
	auto pBouncyMaterial = physX->createMaterial(0, 0, 1);

	auto pGround = new GameObject();
	pGround->AddComponent(new RigidBodyComponent(true));
	std::shared_ptr<physx::PxGeometry> geom(new physx::PxPlaneGeometry());
	pGround->AddComponent(new ColliderComponent(geom, *pBouncyMaterial, physx::PxTransform(physx::PxQuat(DirectX::XM_PIDIV2, physx::PxVec3(0, 0, 1)))));
	AddChild(pGround);

	float tileWidth{ 100.f };
	int rows{ 5 }, cols{ 5 };
	DirectX::XMFLOAT2 pos{ -((rows/2.f) * tileWidth),-((cols/2.f) * tileWidth) };
	
	for (int i = 0; i < rows; ++i)
	{
		pos.x = -((rows/2.f) * tileWidth);
		for (int j = 0; j < cols; ++j)
		{
			auto pFloor = new GameObject();
			auto pFloorModel = new ModelComponent(L"./Resources/Meshes/UnitPlane.ovm");
			pFloorModel->SetMaterial(1);

			pFloor->AddComponent(pFloorModel);
			pFloor->GetTransform()->Scale(10.0f, 10.0f, 10.0f);
			pFloor->GetTransform()->Translate(pos.x, 0, pos.y);
			
			AddChild(pFloor);

			pos.x += tileWidth;
		}
		pos.y += tileWidth;
	}
}

void KitchenScene::CreateLevelBoundaries()
{
	const float xStart{ -75.f }, xStop{ 75 }, y{0}, zStart{ -65.f }, zStop{ 75.f };
	std::shared_ptr<physx::PxGeometry> boundaryGeom{ new physx::PxBoxGeometry(1,100,100) };
	auto pBoundMat = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.f, 0.f, 0.5f);
	
	auto pLevelBoundaries = new GameObject();
	pLevelBoundaries->AddComponent(new RigidBodyComponent(true));
	pLevelBoundaries->AddComponent(new ColliderComponent(boundaryGeom, *pBoundMat, physx::PxTransform{ xStart - 5.f,0,0, physx::PxQuat{DirectX::XM_PIDIV2, physx::PxVec3(1,0,0)} }));
	AddChild(pLevelBoundaries);

	pLevelBoundaries = new GameObject();
	pLevelBoundaries->AddComponent(new RigidBodyComponent(true));
	pLevelBoundaries->AddComponent(new ColliderComponent(boundaryGeom, *pBoundMat, physx::PxTransform{ xStop+5.f,0,0, physx::PxQuat{DirectX::XM_PIDIV2, physx::PxVec3(1,0,0)} }));
	AddChild(pLevelBoundaries);
	
	pLevelBoundaries = new GameObject();
	pLevelBoundaries->AddComponent(new RigidBodyComponent(true));
	pLevelBoundaries->AddComponent(new ColliderComponent(boundaryGeom, *pBoundMat, physx::PxTransform{ 0,0,zStart - 5.f, physx::PxQuat{DirectX::XM_PIDIV2, physx::PxVec3(0,1,0)} }));
	AddChild(pLevelBoundaries);
	
	pLevelBoundaries = new GameObject();
	pLevelBoundaries->AddComponent(new RigidBodyComponent(true));
	pLevelBoundaries->AddComponent(new ColliderComponent(boundaryGeom, *pBoundMat, physx::PxTransform{ 0,0,zStop + 5.f, physx::PxQuat{DirectX::XM_PIDIV2, physx::PxVec3(0,1,0)} }));
	AddChild(pLevelBoundaries);
	
}

void KitchenScene::CreateLevel()
{
	CreateLevelBoundaries();

	const float xStart{ -75.f }, y{0}, zStart{ 75.f };
	const float isleWidth{ 10.f }, isleDepth{ 10.f };

	DirectX::XMFLOAT3 currPos{ xStart, y, zStart };
	
	std::ifstream is{"./Resources/Levels/OverCooked_1.txt"};
	std::string line{};
	PlateReturnIsle* pPlateReturnIsle{ nullptr };
	while(std::getline(is, line))
	{
		for(const auto& type : line)
		{
			switch (type)
			{
			default:
			case '0':
				break;
			case '1':
				AddChild(new KitchenIsle(currPos, m_pPostHighlight));
				break;
			case '2':
				AddChild(new Stove(currPos, m_pPostHighlight));
				break;
			case '3':
				AddChild(new ChoppingBlock(currPos, m_pPostHighlight));
				break;
			case '4':
				AddChild(new IngredientCrate(Ingredient::IngredientType::Steak, currPos, m_pPostHighlight, &m_IsPaused));
				break;
			case '5':
				AddChild(new IngredientCrate(Ingredient::IngredientType::Tomato, currPos, m_pPostHighlight, &m_IsPaused));
				break;
			case '6':
				AddChild(new IngredientCrate(Ingredient::IngredientType::Cabbage, currPos, m_pPostHighlight, &m_IsPaused));
				break;
			case '7':
				AddChild(new TrashCan(currPos, m_pPostHighlight));
				break;
			case '8':
				AddChild(new DeliveryIsle(currPos, m_pPostHighlight, pPlateReturnIsle));
				break;
			case '9':
				AddChild(pPlateReturnIsle = new PlateReturnIsle(currPos, m_pPostHighlight, &m_IsPaused));
				break;
			case 'A':
				AddChild(new KitchenIsle(currPos, m_pPostHighlight, true));
				break;
			}
			currPos.x += isleWidth;
		}

		currPos.x = xStart;
		currPos.z -= isleDepth;
	}
	
}

void KitchenScene::AddDecorations()
{
	std::vector<DirectX::XMFLOAT2> tablePos{
		{-170,110}, {-115,110}, {-50,110}, {0,110}, {50,110}, {115,110}, {170, 110},
		{-170, 60}, {-115,60}, {115,60}, {170,60},
		{-170, 0}, {-115,0}, {115,0}, {170,0},
		{-170, -60}, {115,-60}, {170,-60},
		{-170,-110}, {-115,-110}, {-50,-110}, {0,-110}, {50,-110}, {115,-110}, {170, -110}
	};
	for(const auto& pos : tablePos)
	{
		AddChild(new Table({ pos.x,0,pos.y }));
		AddChild(new Chair({ pos.x-10.f,0,pos.y+10.f }, {0,0,0}));
		AddChild(new Chair({ pos.x+5.f,0,pos.y+10.f }, {0,0,0}));
		AddChild(new Chair({ pos.x-5.f,0,pos.y-10.f }, {0,180,0}));
		AddChild(new Chair({ pos.x+10.f,0,pos.y-10.f }, {0,180,0}));
	}
}

void KitchenScene::AddNewRecipe()
{
	std::vector<Ingredient::IngredientType> ingredientTypes{};

	if (rand() % 3 == 1) //Add all 3 ingredients
	{
		ingredientTypes.push_back(Ingredient::IngredientType::Tomato);
		ingredientTypes.push_back(Ingredient::IngredientType::Steak);
		ingredientTypes.push_back(Ingredient::IngredientType::Cabbage);
	}
	else
	{
		//Add only 2 ingredients
		auto ingredient0{ static_cast<Ingredient::IngredientType>(rand() % 3) };
		auto ingredient1{ static_cast<Ingredient::IngredientType>(rand() % 3) };
		while (ingredient0 == ingredient1)
		{
			ingredient1 = static_cast<Ingredient::IngredientType>(rand() % 3);
		}
		ingredientTypes.push_back(ingredient0);
		ingredientTypes.push_back(ingredient1);
	}

	float timeToCook{ randF(60,90) };
	auto pNewRecipe = new Recipe(ingredientTypes, &m_IsPaused, timeToCook);
	AddChild(pNewRecipe);

	int offset = std::accumulate(m_Recipes.begin(), m_Recipes.end(), 0, [](int total, Recipe* pRecipe) {return total + pRecipe->GetAmountOfIngredients(); });
	
	pNewRecipe->GetTransform()->Translate(offset * m_RecipeIngredientWidth + m_RecipeStartPos.x, m_RecipeStartPos.y, m_RecipeStartPos.z);
	m_Recipes.push_back(pNewRecipe);
}
