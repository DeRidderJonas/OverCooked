#pragma once
#include "GameObject.h"
class PostHighlight;

class Selectable : public GameObject
{
public:
	Selectable(PostHighlight* pPostHighlight);
	virtual ~Selectable() override = default;
	Selectable(const Selectable&) = delete;
	Selectable& operator=(const Selectable&) = delete;
	Selectable(Selectable&&) = delete;
	Selectable& operator=(Selectable&&) = delete;
	
	bool CanBeSelected() const;
	void SetCanBeSelected(bool value);

	void Select();
	void Unselect();
protected:
	virtual float GetMeshWidth() const = 0;
	
	PostHighlight* m_pPH;
private:
	bool m_canBeSelected;
};

