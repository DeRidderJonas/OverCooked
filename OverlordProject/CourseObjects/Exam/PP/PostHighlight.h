#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostHighlight final : public PostProcessingMaterial
{
public:
	PostHighlight();
	~PostHighlight() override = default;

	void SetTopLeft(float x, float y);
	void SetBottomRight(float x, float y);
protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;
private:
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariabele;
	ID3DX11EffectVectorVariable* m_pTopLeft;
	ID3DX11EffectVectorVariable* m_pBottomRight;
};

