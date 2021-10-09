#pragma once
#include "../Selectable.h"

class PostHighlight;
class ModelComponent;

class PickUp : public Selectable
{
public:
	PickUp(PostHighlight* pHighlight);
	~PickUp() override = default;

protected:
	void Initialize(const GameContext&) override = 0;
	void Draw(const GameContext&) override = 0;
	void Update(const GameContext&) override = 0;

	ModelComponent* m_pModel;
};

