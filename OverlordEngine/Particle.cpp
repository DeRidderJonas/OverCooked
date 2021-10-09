#include "stdafx.h"
#include "Particle.h"
#include "MathHelper.h"

// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
using namespace DirectX;

Particle::Particle(const ParticleEmitterSettings& emitterSettings):
	m_VertexInfo(ParticleVertex()),
	m_EmitterSettings(emitterSettings),
	m_IsActive(false),
	m_TotalEnergy(0),
	m_CurrentEnergy(0),
	m_SizeGrow(0),
	m_InitSize(0)
{}

void Particle::Update(const GameContext& gameContext)
{
	if (!m_IsActive) return;

	float dt = gameContext.pGameTime->GetElapsed();
	
	m_CurrentEnergy -= dt;
	if(m_CurrentEnergy <= 0.f)
	{
		m_IsActive = false;
		return;
	}

	//Position
	auto pos = DirectX::XMLoadFloat3(&m_VertexInfo.Position);
	auto vel = DirectX::XMLoadFloat3(&m_EmitterSettings.Velocity);

	auto newPos = pos + dt * vel;
	DirectX::XMStoreFloat3(&m_VertexInfo.Position, newPos);

	//Color
	float particleLifePercent{ 1 - m_CurrentEnergy / m_TotalEnergy };
	m_VertexInfo.Color = m_EmitterSettings.Color;
	m_VertexInfo.Color.w = particleLifePercent * 2.f;

	//Size
	if(m_SizeGrow < 1.f)
	{
		m_VertexInfo.Size = m_InitSize - m_SizeGrow * particleLifePercent;
	}
	else if(m_SizeGrow > 1.f)
	{
		m_VertexInfo.Size = m_InitSize + m_SizeGrow * particleLifePercent;
	}
}

void Particle::Init(XMFLOAT3 initPosition)
{
	m_IsActive = true;

	//Lifetime
	m_TotalEnergy = m_CurrentEnergy = randF(m_EmitterSettings.MinEnergy, m_EmitterSettings.MaxEnergy);

	//Position
	DirectX::XMFLOAT3 forward{ 1,0,0 };
	auto randRotMat = DirectX::XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI));
	auto randDir = DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&forward), randRotMat);

	float distance{ randF(m_EmitterSettings.MinEmitterRange, m_EmitterSettings.MaxEmitterRange) };
	auto startPos = DirectX::XMLoadFloat3(&initPosition);
	startPos += randDir * distance;
	DirectX::XMStoreFloat3(&m_VertexInfo.Position, startPos);

	//Size
	m_VertexInfo.Size = m_InitSize = randF(m_EmitterSettings.MinSize, m_EmitterSettings.MaxSize);
	m_SizeGrow = randF(m_EmitterSettings.MinSizeGrow, m_EmitterSettings.MaxSizeGrow);

	//Rotation
	m_VertexInfo.Rotation = randF(-XM_PI, XM_PI);
}
