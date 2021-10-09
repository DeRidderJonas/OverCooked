#include "stdafx.h"
#include "ShadowMapRenderer.h"
#include "ContentManager.h"
#include "ShadowMapMaterial.h"
#include "RenderTarget.h"
#include "MeshFilter.h"
#include "SceneManager.h"
#include "OverlordGame.h"

ShadowMapRenderer::~ShadowMapRenderer()
{
	delete m_pShadowRT;
	delete m_pShadowMat;
}

void ShadowMapRenderer::Initialize(const GameContext& gameContext)
{
	if (m_IsInitialized)
		return;

	//create shadow generator material + initialize it
	m_pShadowMat = new ShadowMapMaterial();
	m_pShadowMat->Initialize(gameContext);

	//create a rendertarget with the correct settings (hint: depth only) for the shadow generator using a RENDERTARGET_DESC
	RENDERTARGET_DESC rtDesc{};
	rtDesc.EnableDepthBuffer = true;
	rtDesc.EnableDepthSRV = true;
	rtDesc.Height = 720;
	rtDesc.Width = 1280;
	m_pShadowRT = new RenderTarget(gameContext.pDevice);
	auto result = m_pShadowRT->Create(rtDesc);
	if(!SUCCEEDED(result))
	{
		Logger::LogWarning(L"ShadowMapRenderer::Initialize >> Create RenderTarger did not succeed");
	}
	
	m_IsInitialized = true;
}

void ShadowMapRenderer::SetLight(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 direction)
{
	//store the input parameters in the appropriate datamembers
	m_LightPosition = position;
	m_LightDirection = direction;
	//calculate the Light VP matrix (Directional Light only ;)) and store it in the appropriate datamember

	using namespace DirectX;
	
	auto pos = DirectX::XMLoadFloat3(&position);
	auto dir = DirectX::XMLoadFloat3(&direction);
	DirectX::XMFLOAT3 up{ 0,1,0 };
	auto upVec = DirectX::XMLoadFloat3(&up);
	
	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	
	const float viewWidth = (m_Size > 0) ? m_Size * windowSettings.AspectRatio : windowSettings.Width;
	const float viewHeight = (m_Size > 0) ? m_Size : windowSettings.Height;

	auto projection = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, 0.1f, 500.f);
	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, pos + dir, upVec);

	auto vp = view * projection;
	DirectX::XMStoreFloat4x4(&m_LightVP, vp);
}

void ShadowMapRenderer::Begin(const GameContext& gameContext) const
{
	//Reset Texture Register 5 (Unbind)
	ID3D11ShaderResourceView *const pSRV[] = { nullptr };
	gameContext.pDeviceContext->PSSetShaderResources(1, 1, pSRV);

	UNREFERENCED_PARAMETER(gameContext);
	//set the appropriate render target that our shadow generator will write to (hint: use the OverlordGame::SetRenderTarget function through SceneManager)
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(m_pShadowRT);

	//clear this render target
	FLOAT rgba[4] = { 0.f,0.f,0.f,1.f };
	m_pShadowRT->Clear(gameContext, rgba);

	//set the shader variables of this shadow generator material
	m_pShadowMat->SetLightVP(m_LightVP);
}

void ShadowMapRenderer::End(const GameContext& ) const
{
	//restore default render target (hint: passing nullptr to OverlordGame::SetRenderTarget will do the trick)
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(nullptr);
}

void ShadowMapRenderer::Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, DirectX::XMFLOAT4X4 world, const std::vector<DirectX::XMFLOAT4X4>& bones) const
{
	const ShadowMapMaterial::ShadowGenType genType{ pMeshFilter->m_ShadowGenType };
	
	//update shader variables in material
	m_pShadowMat->SetLightVP(m_LightVP);
	m_pShadowMat->SetWorld(world);
	if (genType == ShadowMapMaterial::ShadowGenType::Skinned) m_pShadowMat->SetBones(&bones[0]._11, bones.size());
	
	int genIdx{ genType == ShadowMapMaterial::ShadowGenType::Static ? 0 : 1 };
	//set the correct input layout, buffers, topology (some variables are set based on the generation type Skinned or Static)
	gameContext.pDeviceContext->IASetInputLayout(m_pShadowMat->m_pInputLayouts[genIdx]);

	UINT offset{ 0 };
	auto vertexBuffer = pMeshFilter->GetVertexBufferData(gameContext, m_pShadowMat->m_InputLayoutIds[genIdx]);
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.pVertexBuffer, &vertexBuffer.VertexStride, &offset);

	gameContext.pDeviceContext->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//invoke draw call
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pShadowMat->m_pShadowTechs[genIdx]->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pShadowMat->m_pShadowTechs[genIdx]->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->DrawIndexed(pMeshFilter->m_IndexCount, 0, 0);
	}
}

void ShadowMapRenderer::UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter)
{
	//based on the type (Skinned or Static) build the correct vertex buffers for the MeshFilter (Hint use MeshFilter::BuildVertexBuffer)
	int genIdx{ pMeshFilter->m_ShadowGenType == ShadowMapMaterial::ShadowGenType::Static ? 0 : 1 };
	pMeshFilter->BuildVertexBuffer(gameContext, m_pShadowMat->m_InputLayoutIds[genIdx], m_pShadowMat->m_InputLayoutSizes[genIdx], m_pShadowMat->m_InputLayoutDescriptions[genIdx]);
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowMap() const
{
	//return the depth shader resource view of the shadow generator render target
	return m_pShadowRT->GetDepthShaderResourceView();
}
