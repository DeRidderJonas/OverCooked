#pragma once
#include "Material.h"

class TextureData;

class DiffuseMaterial final : public Material
{
public:
	DiffuseMaterial();
	virtual ~DiffuseMaterial() override;

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent) override;

	TextureData* m_pDiffuseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseSRVvariable;
private:


	DiffuseMaterial(const DiffuseMaterial&) = delete;
	DiffuseMaterial& operator=(const DiffuseMaterial&) = delete;
	DiffuseMaterial(DiffuseMaterial&&) = delete;
	DiffuseMaterial& operator=(DiffuseMaterial&&) = delete;
};

