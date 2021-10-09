#pragma once
#include <GameObject.h>

class ModelComponent;

class Table final : public GameObject
{
public:
	Table(const DirectX::XMFLOAT3& pos);
	~Table() override = default;
protected:
	void Initialize(const GameContext&) override;
private:
	DirectX::XMFLOAT3 m_pos;

	ModelComponent* m_pModel;
};

