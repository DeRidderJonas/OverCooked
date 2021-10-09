#include "stdafx.h"
#include "CameraComponent.h"
#include "OverlordGame.h"
#include "TransformComponent.h"
#include "PhysxProxy.h"
#include "GameObject.h"
#include "GameScene.h"

CameraComponent::CameraComponent() :
	m_FarPlane(2500.0f),
	m_NearPlane(0.1f),
	m_FOV(DirectX::XM_PIDIV4),
	m_Size(25.0f),
	m_IsActive(true),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, DirectX::XMMatrixIdentity());
}

void CameraComponent::Initialize(const GameContext&) {}

void CameraComponent::Update(const GameContext&)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	DirectX::XMMATRIX projection;

	if (m_PerspectiveProjection)
	{
		projection = DirectX::XMMatrixPerspectiveFovLH(m_FOV, windowSettings.AspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * windowSettings.AspectRatio : windowSettings.Width;
		const float viewHeight = (m_Size > 0) ? m_Size : windowSettings.Height;
		projection = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}

	const DirectX::XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const DirectX::XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const DirectX::XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const DirectX::XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const DirectX::XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::Draw(const GameContext&) {}

void CameraComponent::SetActive()
{
	auto gameObject = GetGameObject();
	if (gameObject == nullptr)
	{
		Logger::LogError(L"[CameraComponent] Failed to set active camera. Parent game object is null");
		return;
	}

	auto gameScene = gameObject->GetScene();
	if (gameScene == nullptr)
	{
		Logger::LogError(L"[CameraComponent] Failed to set active camera. Parent game scene is null");
		return;
	}

	gameScene->SetActiveCamera(this);
}

GameObject* CameraComponent::Pick(const GameContext&, CollisionGroupFlag ignoreGroups) const
{
	using namespace DirectX;

	//Step 1 Convert muose to NDC
	auto mousePos = InputManager::GetMousePosition(false);
	auto window = OverlordGame::GetGameSettings().Window;
	XMFLOAT2 viewportHalfSize{ window.Width / 2.f, window.Height / 2.f };
	XMFLOAT2 NDC{ (mousePos.x - viewportHalfSize.x) / viewportHalfSize.x, (viewportHalfSize.y - mousePos.y) / viewportHalfSize.y };

	//Step 2 Calc near and far point
	XMMATRIX viewProjInv = XMLoadFloat4x4(&m_ViewProjectionInverse);

	XMFLOAT3 nearpoint{ NDC.x, NDC.y, 0 };
	XMVECTOR nearpointV = XMLoadFloat3(&nearpoint);
	nearpointV = XMVector3TransformCoord(nearpointV, viewProjInv);
	XMStoreFloat3(&nearpoint, nearpointV);
	physx::PxVec3 rayStart{ nearpoint.x, nearpoint.y, nearpoint.z };
	
	XMFLOAT3 farPoint{ NDC.x, NDC.y, 1 };
	XMVECTOR farpointV = XMLoadFloat3(&farPoint);
	farpointV = XMVector3TransformCoord(farpointV, viewProjInv);
	XMStoreFloat3(&farPoint, farpointV);
	physx::PxVec3 rayEnd{ farPoint.x, farPoint.y, farPoint.z };

	physx::PxVec3 rayDir{ rayEnd - rayStart };
	rayDir.normalize();

	//Step 3 raycast
	physx::PxQueryFilterData filterData;
	filterData.data.word0 = static_cast<physx::PxU32>(ignoreGroups);

	physx::PxRaycastBuffer hit;
	auto physxProxy = GetGameObject()->GetScene()->GetPhysxProxy();
	if(physxProxy->Raycast(rayStart, rayDir, PX_MAX_F32, hit, physx::PxHitFlag::eDEFAULT, filterData))
	{
		//Step 4 shake, do not stir
		auto firstHit = hit.block;
		
		auto pRigidComponent = reinterpret_cast<BaseComponent*>(firstHit.actor->userData);
		return pRigidComponent->GetGameObject();
	}
	
	return nullptr;
}

DirectX::XMFLOAT2 CameraComponent::GetScreenPositionScreenSpace(const DirectX::XMFLOAT3& worldPosition) const
{
	auto worldPosV = DirectX::XMLoadFloat3(&worldPosition);
	auto viewProj = DirectX::XMLoadFloat4x4(&m_ViewProjection);

	auto projected = DirectX::XMVector3TransformCoord(worldPosV, viewProj);
	DirectX::XMFLOAT3 projectedPos{};
	DirectX::XMStoreFloat3(&projectedPos, projected);

	return DirectX::XMFLOAT2{ projectedPos.x, projectedPos.y };
}

DirectX::XMFLOAT2 CameraComponent::GetScreenPosition(const DirectX::XMFLOAT3& worldPosition) const
{
	auto screenSpace = GetScreenPositionScreenSpace(worldPosition);

	auto window = OverlordGame::GetGameSettings().Window;
	return DirectX::XMFLOAT2{ (screenSpace.x + 1) / 2.f * float(window.Width), (1-(screenSpace.y + 1) / 2.f) * float(window.Height)};
}
