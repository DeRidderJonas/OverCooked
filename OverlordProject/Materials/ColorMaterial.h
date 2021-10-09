#pragma once
#include "Material.h"

class ColorMaterial final : public Material
{
public:
	ColorMaterial(bool enableTransparency = false);
	virtual ~ColorMaterial() override = default;

protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ColorMaterial(const ColorMaterial &obj);
	ColorMaterial& operator=(const ColorMaterial& obj);
};

