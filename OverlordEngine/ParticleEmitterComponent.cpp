#include "stdafx.h"
#include "ParticleEmitterComponent.h"
 #include <utility>
#include "EffectHelper.h"
#include "ContentManager.h"
#include "TextureDataLoader.h"
#include "Particle.h"
#include "TransformComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent(std::wstring  assetFile, int particleCount):
	m_pVertexBuffer(nullptr),
	m_pEffect(nullptr),
	m_pParticleTexture(nullptr),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_Settings(ParticleEmitterSettings()),
	m_ParticleCount(particleCount),
	m_ActiveParticles(0),
	m_LastParticleInit(0.0f),
	m_AssetFile(std::move(assetFile)),
	m_IsActive(false)
{
	for (int i = 0; i < particleCount; ++i)
	{
		m_Particles.push_back(new Particle(m_Settings));
	}
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	for(Particle* pParticle : m_Particles)
	{
		delete pParticle;
	}
	m_Particles.clear();

	m_pInputLayout->Release();
	m_pVertexBuffer->Release();
}

void ParticleEmitterComponent::Start()
{
	m_IsActive = true;
}

void ParticleEmitterComponent::Stop()
{
	m_IsActive = false;
}

void ParticleEmitterComponent::Initialize(const GameContext& gameContext)
{
	LoadEffect(gameContext);
	CreateVertexBuffer(gameContext);

	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}

void ParticleEmitterComponent::LoadEffect(const GameContext& gameContext)
{
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ParticleRenderer.fx");
	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);

	m_pWvpVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if(!m_pWvpVariable->IsValid())
	{
		Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect >> \'gWorldViewProj\' variable not found");
		m_pWvpVariable = nullptr;
	}

	m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if(!m_pViewInverseVariable->IsValid())
	{
		Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect >> \'gViewInverse\' variable not found");
		m_pViewInverseVariable = nullptr;
	}

	m_pTextureVariable = m_pEffect->GetVariableByName("gParticleTexture")->AsShaderResource();
	if(!m_pTextureVariable->IsValid())
	{
		Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect >> \'gParticleTexture\' variable not found");
		m_pTextureVariable = nullptr;
	}

	EffectHelper::BuildInputLayout(gameContext.pDevice, m_pDefaultTechnique, &m_pInputLayout, m_pInputLayoutSize);
}

void ParticleEmitterComponent::CreateVertexBuffer(const GameContext& gameContext)
{
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_ParticleCount * sizeof(ParticleVertex);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;

	HRESULT result = gameContext.pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer);
	if(!SUCCEEDED(result))
	{
		Logger::LogWarning(L"ParticleEmitterComponent::CreateVertexBuffer >> CreateBuffer failed");
		m_pVertexBuffer = nullptr;
	}
	
}

void ParticleEmitterComponent::Update(const GameContext& gameContext)
{
	float particleInterval{ (m_Settings.MinEnergy + m_Settings.MaxEnergy) / 2.f / m_ParticleCount };
	m_LastParticleInit += gameContext.pGameTime->GetElapsed();

	m_ActiveParticles = 0;
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	gameContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = (ParticleVertex*) mappedResource.pData;
	for(Particle* pParticle : m_Particles)
	{
		pParticle->Update(gameContext);
		if(pParticle->IsActive())
		{
			pBuffer[m_ActiveParticles] = pParticle->GetVertexInfo();
			m_ActiveParticles++;
		}
		else if(m_LastParticleInit >= particleInterval && m_IsActive)
		{
			pParticle->Init(GetTransform()->GetWorldPosition());
			pBuffer[m_ActiveParticles] = pParticle->GetVertexInfo();
			m_ActiveParticles++;
			m_LastParticleInit = 0.f;
		}
	}
	
	gameContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::Draw(const GameContext& )
{}

void ParticleEmitterComponent::PostDraw(const GameContext& gameContext)
{
	if (m_ActiveParticles <= 0) return;
	
	m_pWvpVariable->SetMatrix(&gameContext.pCamera->GetViewProjection()._11);
	m_pViewInverseVariable->SetMatrix(&gameContext.pCamera->GetViewInverse()._11);
	m_pTextureVariable->SetResource(m_pParticleTexture->GetShaderResourceView());

	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
	unsigned int stride = sizeof(ParticleVertex);
	unsigned int offset{ 0 };
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pDefaultTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		m_pDefaultTechnique->GetPassByIndex(i)->Apply(0, gameContext.pDeviceContext);
		gameContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}
