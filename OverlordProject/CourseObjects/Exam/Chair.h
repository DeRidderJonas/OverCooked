#pragma once
#include "GameObject.h"

class Chair final : public GameObject
{
public:
	Chair(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot);
	~Chair() override = default;
protected:
	void Initialize(const GameContext&) override;
	void PostInitialize(const GameContext&) override;
private:
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rot;
};

