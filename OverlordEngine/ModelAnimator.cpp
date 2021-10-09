#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
m_pMeshFilter(pMeshFilter),
m_Transforms(std::vector<DirectX::XMFLOAT4X4>()),
m_IsPlaying(false), 
m_Reversed(false),
m_ClipSet(false),
m_TickCount(0),
m_AnimationSpeed(1.0f)
{
	SetAnimation(0);
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if(clipNumber >= m_pMeshFilter->m_AnimationClips.size())
	{
		//If not,
			//	Call Reset
			//	Log a warning with an appropriate message
			//	return
		Reset();
		Logger::LogWarning(L"ModelAnimator::SetAnimation < No Valid clipNumber was given");
		return;
	}

	//else
		//	Retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
		//	Call SetAnimation(AnimationClip clip)
	SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber]);
}

void ModelAnimator::SetAnimation(std::wstring clipName)
{
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	auto findIt = std::find_if(m_pMeshFilter->m_AnimationClips.begin(), m_pMeshFilter->m_AnimationClips.end(), [&clipName](const AnimationClip& clip)
	{
			return clip.Name == clipName;
	});

	if(findIt == m_pMeshFilter->m_AnimationClips.end())
	{
		//Else
		//	Call Reset
		//	Log a warning with an appropriate message
		Reset();
		Logger::LogWarning(L"ModelAnimator::SetAnimation < No valid clipName was given");
		return;
	}
	
	//If found,
	//	Call SetAnimation(Animation Clip) with the found clip
	SetAnimation(*findIt);
}

void ModelAnimator::SetAnimation(AnimationClip clip)
{
	//Set m_ClipSet to true
	m_ClipSet = true;
	//Set m_CurrentClip
	m_CurrentClip = clip;
	
	//Call Reset(false)
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	//If pause is true, set m_IsPlaying to false
	if (pause) m_IsPlaying = false;

	//Set m_TickCount to zero
	m_TickCount = 0.f;
	//Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.f;

	if(m_ClipSet)
	{
		//If m_ClipSet is true
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(m_CurrentClip.Keys[0].BoneTransforms.begin(), m_CurrentClip.Keys[0].BoneTransforms.end());
	}
	else
	{
		//Else
		//	Create an IdentityMatrix 
		auto identityMatrix{ DirectX::XMMatrixIdentity() };
		DirectX::XMFLOAT4X4 identity{};
		DirectX::XMStoreFloat4x4(&identity, identityMatrix);
		//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
		m_Transforms.assign(m_pMeshFilter->m_BoneCount, identity);
	}
}

void ModelAnimator::Update(const GameContext& gameContext)
{
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		float elapsedSec = gameContext.pGameTime->GetElapsed();
		float durationInTicks = m_CurrentClip.Duration;
		float ticksPerSeconds = m_CurrentClip.TicksPerSecond;
		//1. 
		//Calculate the passedTicks (see the lab document)
		auto passedTicks = elapsedSec * ticksPerSeconds * m_AnimationSpeed;
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, durationInTicks);

		//2.
		if(m_Reversed)
		{
			//IF m_Reversed is true
			//	Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
			//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
			if (m_TickCount < 0) m_TickCount += m_CurrentClip.Duration;
		}
		else
		{
			//ELSE
			//	Add passedTicks to m_TickCount
			//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			m_TickCount += passedTicks;
			if (m_TickCount > m_CurrentClip.Duration) m_TickCount -= m_CurrentClip.Duration;
		}

		//3.
		//Find the enclosing keys
		AnimationKey keyA, keyB;
		//Iterate all the keys of the clip and find the following keys:
		float tickCount = m_TickCount;
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		auto beforeIt = std::min_element(m_CurrentClip.Keys.begin(), m_CurrentClip.Keys.end(), 
			[&tickCount](const AnimationKey& left, const AnimationKey& right)
			{
				bool leftIsBefore{ left.Tick < tickCount };
				bool rightIsBefore{ left.Tick < tickCount };

				if (leftIsBefore && !rightIsBefore) return true;
				if (!leftIsBefore && rightIsBefore) return false;

				if(leftIsBefore && rightIsBefore)
					return left.Tick > right.Tick;

				return left.Tick < right.Tick;
			});
		keyA = *beforeIt;
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		auto afterIt = std::min_element(m_CurrentClip.Keys.begin(), m_CurrentClip.Keys.end(),
			[&tickCount](const AnimationKey& left, const AnimationKey& right)
			{
				bool leftIsAfter{ left.Tick > tickCount };
				bool rightIsAfter{ right.Tick > tickCount };

				if (leftIsAfter && !rightIsAfter) return true;
				if (!leftIsAfter && rightIsAfter) return false;

				if (leftIsAfter && rightIsAfter)
					return left.Tick < right.Tick;

				return left.Tick > right.Tick;
			});
		keyB = *afterIt;

		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		float blendFactor = (m_TickCount - keyA.Tick) / (keyB.Tick - keyA.Tick);
		//Clear the m_Transforms vector
		m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (int i = 0; i < m_pMeshFilter->m_BoneCount; ++i)
		{
			//	Retrieve the transform from keyA (transformA)
			auto transformA = DirectX::XMLoadFloat4x4(&keyA.BoneTransforms[i]);
			// 	Retrieve the transform from keyB (transformB)
			auto transformB = DirectX::XMLoadFloat4x4(&keyB.BoneTransforms[i]);
			//	Decompose both transforms
			DirectX::XMVECTOR scaleA{};
			DirectX::XMVECTOR rotA{};
			DirectX::XMVECTOR transA{};
			DirectX::XMMatrixDecompose(&scaleA, &rotA, &transA, transformA);
			
			DirectX::XMVECTOR scaleB{};
			DirectX::XMVECTOR rotB{};
			DirectX::XMVECTOR transB{};
			DirectX::XMMatrixDecompose(&scaleB, &rotB, &transB, transformB);

			//	Lerp between all the transformations (Position, Scale, Rotation)
			auto scaleLerp = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorLerp(scaleA, scaleB, blendFactor));
			auto rotLerp = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(rotA, rotB, blendFactor));
			auto posLerp = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorLerp(transA, transB, blendFactor));
			
			//	Compose a transformation matrix with the lerp-results
			auto transformLerp = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(scaleLerp, rotLerp), posLerp);
			DirectX::XMFLOAT4X4 transform{};
			DirectX::XMStoreFloat4x4(&transform, transformLerp);
			
			//	Add the resulting matrix to the m_Transforms vector
			m_Transforms.push_back(transform);
		}
	}
}
