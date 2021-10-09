#include "stdafx.h"
#include "SpriteFontLoader.h"
#include "BinaryReader.h"
#include "ContentManager.h"
#include "TextureData.h"

SpriteFont* SpriteFontLoader::LoadContent(const std::wstring& assetFile)
{
	auto pBinReader = new BinaryReader();
	pBinReader->Open(assetFile);

	if (!pBinReader->Exists())
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Failed to read the assetFile!\nPath: \'%s\'", assetFile.c_str());
		return nullptr;
	}

	//See BMFont Documentation for Binary Layout

	//Parse the Identification bytes (B,M,F)
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr
	//...
	char Id0 = pBinReader->Read<char>();
	char Id1 = pBinReader->Read<char>();
	char Id2 = pBinReader->Read<char>();

	if(Id0 != 'B' || Id1 != 'M' || Id2 != 'F')
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoaderContent > Not a valid .fnt font");
		return nullptr;
	}

	//Parse the version (version 3 required)
	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
	//return nullptr
	//...
	int version = pBinReader->Read<char>();
	if(version != 3)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Only .fnt version 3 is supported");
		return nullptr;
	}

	//Valid .fnt file
	auto pSpriteFont = new SpriteFont();
	//SpriteFontLoader is a friend class of SpriteFont
	//That means you have access to its privates (pSpriteFont->m_FontName = ... is valid)

	//**********
	// BLOCK 0 *
	//**********
	//Retrieve the blockId and blockSize
	int blockId = pBinReader->Read<char>();
	if(blockId != 1)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > First block was not of type Info");
		return nullptr;
	}
	int blockSize = pBinReader->Read<int>();
	if(blockSize <= 0)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Info block did not have a valid size");
		return nullptr;
	}
	
	pSpriteFont->m_FontSize = pBinReader->Read<short>();
	//Retrieve the FontSize (will be -25, BMF bug) [SpriteFont::m_FontSize]
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	//Retrieve the FontName [SpriteFont::m_FontName]
	//...
	int unneededBitSize = 1 + 1 + 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1;
	pBinReader->MoveBufferPosition(unneededBitSize);
	pSpriteFont->m_FontName = pBinReader->ReadNullString();


	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve Texture Width & Height [SpriteFont::m_TextureWidth/m_TextureHeight]
	//Retrieve PageCount
	//> if pagecount > 1
	//> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed)
	//Advance to Block2 (Move Reader)
	//...
	blockId = pBinReader->Read<char>();
	if (blockId != 2)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Second block was not of type Common");
		return nullptr;
	}
	blockSize = pBinReader->Read<int>();
	if (blockSize <= 0)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Common block did not have a valid size");
		return nullptr;
	}

	unneededBitSize = 2 + 2;
	pBinReader->MoveBufferPosition(unneededBitSize);
	pSpriteFont->m_TextureWidth = pBinReader->Read<short>();
	pSpriteFont->m_TextureHeight = pBinReader->Read<short>();
	short pageCount = pBinReader->Read<short>();
	if(pageCount > 1)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed");
		return nullptr;
	}

	unneededBitSize = 1 + 1 + 1 + 1 + 1;
	pBinReader->MoveBufferPosition(unneededBitSize);
	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve the PageName (store Local)
	//	> If PageName is empty
	//	> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty])
	//>Retrieve texture filepath from the assetFile path
	//> (ex. c:/Example/somefont.fnt => c:/Example/) [Have a look at: wstring::rfind()]
	//>Use path and PageName to load the texture using the ContentManager [SpriteFont::m_pTexture]
	//> (ex. c:/Example/ + 'PageName' => c:/Example/somefont_0.png)
	//...
	blockId = pBinReader->Read<char>();
	if (blockId != 3)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Third block was not of type Page");
		return nullptr;
	}
	blockSize = pBinReader->Read<int>();
	if (blockSize <= 0)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Page block did not have a valid size");
		return nullptr;
	}

	auto lastSlashIdx = assetFile.rfind(L'/');
	std::wstring path = assetFile.substr(0, lastSlashIdx + 1);
	std::wstring pageName = pBinReader->ReadNullString();
	pSpriteFont->m_pTexture = ContentManager::Load<TextureData>(path + pageName);

	
	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve Character Count (see documentation)
	//Retrieve Every Character, For every Character:
	//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
	//> Check if CharacterId is valid (SpriteFont::IsCharValid), Log Warning and advance to next character if not valid
	//> Retrieve the corresponding FontMetric (SpriteFont::GetMetric) [REFERENCE!!!]
	//> Set IsValid to true [FontMetric::IsValid]
	//> Set Character (CharacterId) [FontMetric::Character]
	//> Retrieve Xposition (store Local)
	//> Retrieve Yposition (store Local)
	//> Retrieve & Set Width [FontMetric::Width]
	//> Retrieve & Set Height [FontMetric::Height]
	//> Retrieve & Set OffsetX [FontMetric::OffsetX]
	//> Retrieve & Set OffsetY [FontMetric::OffsetY]
	//> Retrieve & Set AdvanceX [FontMetric::AdvanceX]
	//> Retrieve & Set Page [FontMetric::Page]
	//> Retrieve Channel (BITFIELD!!!) 
	//	> See documentation for BitField meaning [FontMetric::Channel]
	//> Calculate Texture Coordinates using Xposition, Yposition, TextureWidth & TextureHeight [FontMetric::TexCoord]
	//...
	blockId = pBinReader->Read<char>();
	if (blockId != 4)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Fourth block was not of type Char");
		return nullptr;
	}
	blockSize = pBinReader->Read<int>();
	if (blockSize <= 0)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteLoader::LoadContent > Page block did not have a valid size");
		return nullptr;
	}
	int numChars = blockSize / 20;
	int test = pBinReader->GetBufferPosition(); test;
	for (int i = 0; i < numChars; i++)
	{
		wchar_t characterId = wchar_t(pBinReader->Read<int>());
		if(!pSpriteFont->IsCharValid(characterId))
		{
			Logger::LogFormat(LogLevel::Warning, L"SpriteFontLoader::LoadContent > CharacterId was not valid: " + characterId);
			continue;
		}

		auto& fontMetric = pSpriteFont->GetMetric(characterId);
		fontMetric.IsValid = true;
		fontMetric.Character = characterId;
		int test2 = pBinReader->GetBufferPosition(); test2;
		float xPos = pBinReader->Read<short>();
		float yPos = pBinReader->Read<short>();
		fontMetric.Width = pBinReader->Read<short>();
		fontMetric.Height = pBinReader->Read<short>();
		fontMetric.OffsetX = pBinReader->Read<short>();
		fontMetric.OffsetY = pBinReader->Read<short>();
		fontMetric.AdvanceX = pBinReader->Read<short>();
		fontMetric.Page = pBinReader->Read<char>();
		int channel = pBinReader->Read<char>();
		if (channel == 1) fontMetric.Channel = 2;
		else if (channel == 2) fontMetric.Channel = 1;
		else if (channel == 4) fontMetric.Channel = 0;
		else if (channel == 8) fontMetric.Channel = 3;
		
		fontMetric.TexCoord.x = xPos / float(pSpriteFont->GetTextureWidth());
		fontMetric.TexCoord.y = yPos / float(pSpriteFont->GetTextureHeight());
	}

	//DONE :)

	delete pBinReader;
	return pSpriteFont;
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
