#include "stdafx.h"
#include "Recipe.h"

#include "ContentManager.h"
#include "ProgressBar.h"
#include "SpriteComponent.h"
#include "SpriteFont.h"
#include "TextRenderer.h"
#include "TransformComponent.h"

Recipe::Recipe(const std::vector<Ingredient::IngredientType>& ingredients, bool* pIsPaused, float time, float lateTime)
	: m_Ingredients(ingredients)
	, m_InitialTime(time)
	, m_LateTime(lateTime > 0 ? lateTime : time)
	, m_TimeRemaining(time)
	, m_isLate(false)
	, m_pIsPaused(pIsPaused)
	, m_pProgressBar(nullptr)
{
}

bool Recipe::operator==(const Container& other)
{
	if (other.GetType() == Container::ContainerType::Pan)
		return false;
	
	const auto& containerIngredients{ other.GetIngredients() };

	if (m_Ingredients.size() != containerIngredients.size())
		return false;
	
	return std::all_of(containerIngredients.begin(), containerIngredients.end(), [this](Ingredient* pIngredient)
		{
			return std::any_of(m_Ingredients.begin(), m_Ingredients.end(), [&pIngredient](const Ingredient::IngredientType& type)
				{
					return pIngredient->GetType() == type;
				});
		});
}


bool Recipe::IsLate() const
{
	return m_isLate;
}

bool Recipe::IsExpired() const
{
	return m_TimeRemaining <= 0.f;
}

int Recipe::GetAmountOfIngredients() const
{
	return m_Ingredients.size();
}

void Recipe::Initialize(const GameContext&)
{
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Arial_20.fnt");

	if(m_Ingredients.size() == 2)
	{
		AddComponent(new SpriteComponent(L"./Resources/Textures/Recipe_2.png"));
	}
	else if(m_Ingredients.size() == 3)
	{
		AddComponent(new SpriteComponent(L"./Resources/Textures/Recipe_3.png"));
	}
	
	DirectX::XMFLOAT2 offset{5,8};
	for(auto& ingredientType : m_Ingredients)
	{
		std::wstring filePath{};
		switch (ingredientType)
		{
		case Ingredient::IngredientType::Cabbage:
			filePath = L"./Resources/Textures/Cabbage.png";
			break;
		case Ingredient::IngredientType::Tomato:
			filePath = L"./Resources/Textures/Tomato.png";
			break;
		case Ingredient::IngredientType::Steak:
		default:
			filePath = L"./Resources/Textures/Steak.png";
		}

		GameObject* pChild = new GameObject();
		pChild->AddComponent(new SpriteComponent(filePath));
		AddChild(pChild);
		pChild->GetTransform()->Translate(offset.x, offset.y, 0.f);

		offset.x += 50.f;
	}

	m_pProgressBar = new ProgressBar();
	AddChild(m_pProgressBar);
}

void Recipe::Update(const GameContext& gameContext)
{
	if (*m_pIsPaused)
		return;
	
	m_TimeRemaining -= gameContext.pGameTime->GetElapsed();
	if (m_TimeRemaining <= 0.f && !m_isLate)
	{
		m_TimeRemaining = m_LateTime;
		m_isLate = true;
		m_pProgressBar->SetBarColor(false);
	}

	if (m_isLate)
		m_pProgressBar->SetPercentage(m_TimeRemaining / m_LateTime);
	else
		m_pProgressBar->SetPercentage(m_TimeRemaining / m_InitialTime);
}

void Recipe::Draw(const GameContext&)
{
}

void Recipe::PostInitialize(const GameContext&)
{
	auto pos = GetTransform()->GetPosition();
	m_pProgressBar->GetTransform()->Translate(pos.x + m_Ingredients.size() / 2.f * 25.f, pos.y + 50.f, 0.95f);
	m_pProgressBar->GetTransform()->Scale(2.f, 1.5f, 1.f);
}
