#include "stdafx.h"
#include "PostProcessingMaterial.h"
#include "RenderTarget.h"
#include "OverlordGame.h"
#include "ContentManager.h"

PostProcessingMaterial::PostProcessingMaterial(std::wstring effectFile, unsigned int renderIndex,
                                               std::wstring technique)
	: m_IsInitialized(false), 
	  m_pInputLayout(nullptr),
	  m_pInputLayoutSize(0),
	  m_effectFile(std::move(effectFile)),
	  m_InputLayoutID(0),
	  m_RenderIndex(renderIndex),
	  m_pRenderTarget(nullptr),
	  m_pVertexBuffer(nullptr),
	  m_pIndexBuffer(nullptr),
	  m_NumVertices(0),
	  m_NumIndices(0),
	  m_VertexBufferStride(0),
	  m_pEffect(nullptr),
	  m_pTechnique(nullptr),
	  m_TechniqueName(std::move(technique))
{
}

PostProcessingMaterial::~PostProcessingMaterial()
{
	//delete and/or release necessary objects and/or resources
	delete m_pRenderTarget;

	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pIndexBuffer);
	SafeRelease(m_pInputLayout);
}

void PostProcessingMaterial::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		//1. LoadEffect (LoadEffect(...))
		LoadEffect(gameContext, m_effectFile);
		//2. CreateInputLaytout (CreateInputLayout(...))
		EffectHelper::BuildInputLayout(gameContext.pDevice, m_pTechnique, &m_pInputLayout, m_pInputLayoutDescriptions, m_pInputLayoutSize, m_InputLayoutID);
		//   CreateVertexBuffer (CreateVertexBuffer(...)) > As a TriangleStrip (FullScreen Quad)
		CreateVertexBuffer(gameContext);
		CreateIndexBuffer(gameContext);
		//3. Create RenderTarget (m_pRenderTarget)
		//		Take a look at the class, figure out how to initialize/create a RenderTarget Object
		//		GameSettings > OverlordGame::GetGameSettings()
		RENDERTARGET_DESC desc{};
		desc.EnableColorBuffer = true;
		desc.EnableColorSRV = true;
		desc.GenerateMipMaps_Color = true;
		
		auto& gameSettings{ OverlordGame::GetGameSettings() };
		desc.Width = gameSettings.Window.Width;
		desc.Height = gameSettings.Window.Height;

		m_pRenderTarget = new RenderTarget(gameContext.pDevice);
		auto hresult = m_pRenderTarget->Create(desc);
		if(!SUCCEEDED(hresult))
		{
			Logger::LogWarning(L"PostProcessingMaterial::Initialize >> RenderTarget::Create failed");
			return;
		}
		
		m_IsInitialized = true;
	}
}

bool PostProcessingMaterial::LoadEffect(const GameContext& , const std::wstring& effectFile)
{
	//Load Effect through ContentManager
	m_pEffect = ContentManager::Load<ID3DX11Effect>(effectFile);
	//Check if m_TechniqueName (default constructor parameter) is set
	if(!m_TechniqueName.empty())
	{
		// If SET > Use this Technique (+ check if valid)

		//https://stackoverflow.com/questions/22585326/how-to-convert-stdwstring-to-lpctstr-in-c
		int size_needed = WideCharToMultiByte(CP_ACP, 0, effectFile.c_str(), int(effectFile.length() + 1), 0, 0, 0, 0);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_ACP, 0, effectFile.c_str(), int(effectFile.length() + 1), &strTo[0], size_needed, 0, 0);

		m_pTechnique = m_pEffect->GetTechniqueByName(strTo.c_str());
		if(!m_pTechnique->IsValid())
		{
			Logger::LogWarning(L"PostProcessingMaterial::LoadEffect >> Technique name: \'" + m_TechniqueName + L"\' not available in Effect. Reverting to technique index 0");
			m_pTechnique = nullptr;
		}
	}
	// If !SET > Use Technique with index 0
	if(m_pTechnique == nullptr)
	{
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	}

	//Call LoadEffectVariables
	LoadEffectVariables();

	return true;
}

void PostProcessingMaterial::Draw(const GameContext& gameContext, RenderTarget* previousRendertarget)
{
	//1. Clear the object's RenderTarget (m_pRenderTarget) [Check RenderTarget Class]
	FLOAT clearColor[4]{ 0,0,0,1 };
	m_pRenderTarget->Clear(gameContext, clearColor);
	//2. Call UpdateEffectVariables(...)
	UpdateEffectVariables(previousRendertarget);
	//3. Set InputLayout
	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	//4. Set VertexBuffer
	UINT offset{0};
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &offset);
	gameContext.pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//5. Set PrimitiveTopology (TRIANGLELIST)
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//6. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);
	for (uint32_t p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}

	// Generate Mips
	gameContext.pDeviceContext->GenerateMips(m_pRenderTarget->GetShaderResourceView());
}

void PostProcessingMaterial::CreateVertexBuffer(const GameContext& gameContext)
{
	if (m_pVertexBuffer)
		return;
	
	m_NumVertices = 4;

	//Create vertex array containing three elements in system memory
	VertexPosTex vertices[4]
	{
		VertexPosTex{{-1,1,0}, {0,0}},
		VertexPosTex{{1,1,0}, {1,0}},
		VertexPosTex{{1,-1,0}, {1,1}},
		VertexPosTex{{-1,-1,0}, {0,1}}
	};
	
	//fill a buffer description to copy the vertexdata into graphics memory
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(VertexPosTex) * m_NumVertices;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	//create a ID3D10Buffer in graphics memory containing the vertex info
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;
	gameContext.pDevice->CreateBuffer(&desc, &initData, &m_pVertexBuffer);

	m_VertexBufferStride = sizeof(VertexPosTex);
}

void PostProcessingMaterial::CreateIndexBuffer(const GameContext& gameContext)
{
	if (m_pIndexBuffer)
		return;
	
	m_NumIndices = 6;

	UINT indices[6]{ 0,1,2,0,2,3 };
	
	// Create index buffer
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(UINT) * m_NumIndices;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = indices;

	auto hr = gameContext.pDevice->CreateBuffer(&desc, &initData, &m_pIndexBuffer);
	Logger::LogHResult(hr, L"PostProcessingMaterial::CreateIndexBuffer");
}
