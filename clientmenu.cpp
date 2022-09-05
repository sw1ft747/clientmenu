#include "clientmenu.h"
#include "keyvalues.h"

#include <hl_sdk/engine/APIProxy.h>

#include <dbg.h>
#include <convar.h>
#include <color.h>

#include <IHooks.h>
#include <IMemoryUtils.h>
#include <ISvenModAPI.h>
#include <IVGUI.h>
#include <vgui2/ISurface.h>

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CClientMenu g_ClientMenu;

static wchar_t localize_buffer[4096];

static const wchar_t *wszLabelNums[10] =
{
	L"0.",
	L"1.",
	L"2.",
	L"3.",
	L"4.",
	L"5.",
	L"6.",
	L"7.",
	L"8.",
	L"9."
};

CommandCallbackFn ORIG_slot10 = NULL;
CommandCallbackFn ORIG_slot1 = NULL;
CommandCallbackFn ORIG_slot2 = NULL;
CommandCallbackFn ORIG_slot3 = NULL;
CommandCallbackFn ORIG_slot4 = NULL;
CommandCallbackFn ORIG_slot5 = NULL;
CommandCallbackFn ORIG_slot6 = NULL;
CommandCallbackFn ORIG_slot7 = NULL;
CommandCallbackFn ORIG_slot8 = NULL;
CommandCallbackFn ORIG_slot9 = NULL;

DECLARE_FUNC_PTR(vgui::HFont, __cdecl, VGUI2_GetCreditsFont);

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar cl_menu_width_fraction("cl_menu_width_fraction", "0.0125", FCVAR_CLIENTDLL, "The screen's fraction of width", true, 0.f, true, 1.f);
ConVar cl_menu_height_fraction("cl_menu_height_fraction", "0.5", FCVAR_CLIENTDLL, "The screen's fraction of height", true, 0.f, true, 1.f);
ConVar cl_menu_title_color("cl_menu_title_color", "255 255 112 255", FCVAR_CLIENTDLL, "Color of the menu's title");
ConVar cl_menu_label_color("cl_menu_label_color", "255 255 255 255", FCVAR_CLIENTDLL, "Color of the menu's label");
ConVar cl_menu_label_number_color("cl_menu_label_number_color", "255 96 96 255", FCVAR_CLIENTDLL, "Color of the menu's number of label");
ConVar cl_menu_fade_duration("cl_menu_fade_duration", "0.5", FCVAR_CLIENTDLL, "Duration of fade when menu is closed", true, 0.f, false, FLT_MAX);
ConVar cl_menu_duration("cl_menu_duration", "10", FCVAR_CLIENTDLL, "Duration of menu before it will autoclose, use value \"-1\" to disable it", true, -1.f, false, FLT_MAX);
ConVar cl_menu_align_center("cl_menu_align_center", "1", FCVAR_CLIENTDLL, "Align menu to center");

CON_COMMAND(cl_menu_show, "Show the client menu")
{
	if (args.ArgC() >= 2)
	{
		g_ClientMenu.ShowMenu( args[1] );
	}
	else
	{
		ConMsg("Usage:  cl_menu_show <name>\n");
	}
}

CON_COMMAND(cl_menu_reload, "Reload client's menu from file \"../svencoop/clientmenu/clientmenu.txt\"")
{
	if ( g_ClientMenu.LoadFromFile() )
	{
		ConMsg("Reloaded file \"../svencoop/clientmenu/clientmenu.txt\"\n");
	}
}

// fucking "play" command only plays a sound in 3D
CON_COMMAND(cl_menu_playsound, "Play a sound file")
{
	if (args.ArgC() >= 2)
	{
		const char *pszExt = NULL;
		std::string sFilePath = args[1];

		const char *pszFilename = args[1];

		while (*pszFilename)
		{
			if (*pszFilename == '.')
			{
				pszExt = pszFilename;
				break;
			}

			pszFilename++;
		}

		if (pszExt != NULL)
		{
			if ( stricmp(pszExt, ".wav") )
			{
				Warning("cl_menu_playsound: wrong file (%s) extension\n", pszFilename);
				return;;
			}
		}
		else
		{
			sFilePath += ".wav";
		}

		g_pEngineFuncs->PlaySoundByName(sFilePath.c_str(), 1.f);
	}
	else
	{
		ConMsg("Usage:  cl_menu_playsound <filename>\n");
	}
}

//-----------------------------------------------------------------------------
// slotX hooks
//-----------------------------------------------------------------------------

static void HOOKED_slot10()
{
	if ( g_ClientMenu.HandleSlotInput(0) ) // act as zero
		return;

	ORIG_slot10();
}

static void HOOKED_slot1()
{
	if ( g_ClientMenu.HandleSlotInput(1) )
		return;

	ORIG_slot1();
}

static void HOOKED_slot2()
{
	if ( g_ClientMenu.HandleSlotInput(2) )
		return;

	ORIG_slot2();
}

static void HOOKED_slot3()
{
	if ( g_ClientMenu.HandleSlotInput(3) )
		return;

	ORIG_slot3();
}

static void HOOKED_slot4()
{
	if ( g_ClientMenu.HandleSlotInput(4) )
		return;

	ORIG_slot4();
}

static void HOOKED_slot5()
{
	if ( g_ClientMenu.HandleSlotInput(5) )
		return;

	ORIG_slot5();
}

static void HOOKED_slot6()
{
	if ( g_ClientMenu.HandleSlotInput(6) )
		return;

	ORIG_slot6();
}

static void HOOKED_slot7()
{
	if ( g_ClientMenu.HandleSlotInput(7) )
		return;

	ORIG_slot7();
}

static void HOOKED_slot8()
{
	if ( g_ClientMenu.HandleSlotInput(8) )
		return;

	ORIG_slot8();
}

static void HOOKED_slot9()
{
	if ( g_ClientMenu.HandleSlotInput(9) )
		return;

	ORIG_slot9();
}

//-----------------------------------------------------------------------------
// Client Menu Context
//-----------------------------------------------------------------------------

CClientMenuContext::CClientMenuContext()
{
	ZeroMemory( m_pwszLabels, 10 * sizeof(wchar_t *) );
	ZeroMemory( m_pszCommands, 10 * sizeof(char *) );
	ZeroMemory( m_fLabelsFlags, 10 * sizeof(int) );

	m_pwszTitle = NULL;
}

CClientMenuContext::~CClientMenuContext()
{
	if ( m_pwszTitle != NULL )
	{
		free( (void *)m_pwszTitle );
	}

	for (int i = 0; i < 10; i++)
	{
		if ( m_pwszLabels[i] != NULL )
		{
			free( (void *)m_pwszLabels[i] );
		}

		if ( m_pszCommands[i] != NULL )
		{
			free( (void *)m_pszCommands[i] );
		}
	}
}

void CClientMenuContext::CalcHeight(vgui::HFont font, int &height, int &titleTall, int labelsTall[10])
{
	int wide, tall;

	height = 0;

	if ( m_pwszTitle != NULL )
	{
		vgui::surface()->GetTextSize(font, m_pwszTitle, wide, tall);

		titleTall = tall;
		height += 2 * tall;
	}

	for (int i = 1; i < 10; i++)
	{
		if ( m_pwszLabels[i] != NULL )
		{
			vgui::surface()->GetTextSize(font, wszLabelNums[i], wide, tall);

			if ( m_fLabelsFlags[i] & FL_LABEL_INDENT )
			{
				height += tall;
			}

			labelsTall[i] = tall;
			height += tall;
		}
	}

	if ( m_pwszLabels[0] != NULL )
	{
		vgui::surface()->GetTextSize(font, wszLabelNums[0], wide, tall);

		labelsTall[0] = tall;
		height += 2 * tall;
	}
}

void CClientMenuContext::Draw(const Color &titleColor, const Color &labelColor, const Color &labelNumberColor)
{
	vgui::HFont font = VGUI2_GetCreditsFont();

	Assert( font != 0 );

	int wide, tall;

	int x = int( (float)Utils()->GetScreenWidth() * cl_menu_width_fraction.GetFloat() );
	int y = int( (float)Utils()->GetScreenHeight() * cl_menu_height_fraction.GetFloat() );

	if ( cl_menu_align_center.GetBool() )
	{
		static int labelsTall[10];
		int titleTall;

		int height = 0;

		CalcHeight(font, height, titleTall, labelsTall);

		y -= height / 2;

		if ( m_pwszTitle != NULL )
		{
			DrawPrintText(font, x, y, titleColor.r, titleColor.g, titleColor.b, titleColor.a, m_pwszTitle);

			y += 2 * titleTall;
		}

		for (int i = 1; i < 10; i++)
		{
			if ( m_pwszLabels[i] != NULL )
			{
				if ( m_fLabelsFlags[i] & FL_LABEL_INDENT )
				{
					y += labelsTall[i];
				}

				DrawPrintText(font, x, y, labelNumberColor.r, labelNumberColor.g, labelNumberColor.b, labelNumberColor.a, wszLabelNums[i]);
				
				if ( m_fLabelsFlags[i] & FL_LABEL_PLAYERNAME )
				{
					int index = _wtoi(m_pwszLabels[i]);

					if (index > 0 && index <= MAXCLIENTS)
					{
						if ( g_pEngineFuncs->GetEntityByIndex(index) != NULL )
						{
							player_info_t *pPlayerInfo = g_pEngineStudio->PlayerInfo(index - 1);

							if (pPlayerInfo && pPlayerInfo->name && pPlayerInfo->name[0])
							{
								vgui::localize()->ConvertANSIToUnicode(pPlayerInfo->name, localize_buffer, sizeof(localize_buffer) / sizeof(wchar_t));
								DrawPrintText(font, x + labelsTall[i], y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, localize_buffer);
							}
							else
							{
								DrawPrintText(font, x + labelsTall[i], y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, L"(null)");
							}
						}
					}
					else
					{
						DrawPrintText(font, x + labelsTall[i], y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, L"(null)");
					}
				}
				else
				{
					DrawPrintText(font, x + labelsTall[i], y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, m_pwszLabels[i]);
				}

				y += labelsTall[i];
			}
		}

		if ( m_pwszLabels[0] != NULL )
		{
			y += labelsTall[0];

			DrawPrintText(font, x, y, labelNumberColor.r, labelNumberColor.g, labelNumberColor.b, labelNumberColor.a, wszLabelNums[0]);
			DrawPrintText(font, x + labelsTall[0], y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, m_pwszLabels[0]);
		}
	}
	else
	{
		if ( m_pwszTitle != NULL )
		{
			vgui::surface()->GetTextSize(font, m_pwszTitle, wide, tall);

			DrawPrintText(font, x, y, titleColor.r, titleColor.g, titleColor.b, titleColor.a, m_pwszTitle);

			y += 2 * tall;
		}

		for (int i = 1; i < 10; i++)
		{
			if ( m_pwszLabels[i] != NULL )
			{
				vgui::surface()->GetTextSize(font, wszLabelNums[i], wide, tall);

				if ( m_fLabelsFlags[i] & FL_LABEL_INDENT )
				{
					y += tall;
				}

				DrawPrintText(font, x, y, labelNumberColor.r, labelNumberColor.g, labelNumberColor.b, labelNumberColor.a, wszLabelNums[i]);
				
				if ( m_fLabelsFlags[i] & FL_LABEL_PLAYERNAME )
				{
					int index = _wtoi(m_pwszLabels[i]);

					if (index > 0 && index <= MAXCLIENTS)
					{
						if ( g_pEngineFuncs->GetEntityByIndex(index) != NULL )
						{
							player_info_t *pPlayerInfo = g_pEngineStudio->PlayerInfo(index - 1);

							if (pPlayerInfo && pPlayerInfo->name && pPlayerInfo->name[0])
							{
								vgui::localize()->ConvertANSIToUnicode(pPlayerInfo->name, localize_buffer, sizeof(localize_buffer) / sizeof(wchar_t));
								DrawPrintText(font, x + tall, y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, localize_buffer);
							}
							else
							{
								DrawPrintText(font, x + tall, y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, L"(null)");
							}
						}
					}
					else
					{
						DrawPrintText(font, x + tall, y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, L"(null)");
					}
				}
				else
				{
					DrawPrintText(font, x + tall, y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, m_pwszLabels[i]);
				}

				y += tall;
			}
		}

		if ( m_pwszLabels[0] != NULL )
		{
			vgui::surface()->GetTextSize(font, wszLabelNums[0], wide, tall);

			y += tall;

			DrawPrintText(font, x, y, labelNumberColor.r, labelNumberColor.g, labelNumberColor.b, labelNumberColor.a, wszLabelNums[0]);
			DrawPrintText(font, x + tall, y, labelColor.r, labelColor.g, labelColor.b, labelColor.a, m_pwszLabels[0]);
		}
	}
}

void CClientMenuContext::SlotInput(int slot)
{
	if ( HasLabel(slot) )
	{
		g_pEngineFuncs->ClientCmd( GetCommand(slot) );
	}
}

void CClientMenuContext::DrawPrintText(vgui::HFont font, int x, int y, int r, int g, int b, int alpha, const wchar_t *pwszText)
{
	vgui::surface()->DrawSetTextFont(font);
	vgui::surface()->DrawSetTextPos(x, y);
	vgui::surface()->DrawSetTextColor(r, g, b, alpha);
	vgui::surface()->DrawPrintText(pwszText, wcslen(pwszText));
}

void CClientMenuContext::FeedTitle(const char *pszTitle)
{
	if ( m_pwszTitle == NULL )
	{
		//const size_t length = strlen(pszTitle) + 1;
		//m_pwszTitle = new wchar_t[length];

		vgui::localize()->ConvertANSIToUnicode(pszTitle, localize_buffer, sizeof(localize_buffer) / sizeof(wchar_t));
		m_pwszTitle = wcsdup(localize_buffer);

		//mbstowcs( (wchar_t *)m_pwszTitle, pszTitle, length );
	}
}

void CClientMenuContext::FeedLabel(const char *pszLabel, int slot)
{
	if ( m_pwszLabels[slot] == NULL )
	{
		//const size_t length = strlen(pszLabel) + 1;
		//m_pwszLabels[slot] = new wchar_t[length];

		vgui::localize()->ConvertANSIToUnicode(pszLabel, localize_buffer, sizeof(localize_buffer) / sizeof(wchar_t));
		m_pwszLabels[slot] = wcsdup(localize_buffer);

		//mbstowcs( (wchar_t *)m_pwszLabels[slot], pszLabel, length );
	}
}

void CClientMenuContext::FeedCommand(const char *pszCommand, int slot)
{
	if ( m_pszCommands[slot] == NULL )
	{
		m_pszCommands[slot] = strdup(pszCommand);
	}
}

void CClientMenuContext::FeedFlags(int flags, int slot)
{
	if ( m_fLabelsFlags[slot] == 0 )
	{
		m_fLabelsFlags[slot] = flags;
	}
}

//-----------------------------------------------------------------------------
// Client Menu
//-----------------------------------------------------------------------------

bool CClientMenu::LoadFromFile()
{
	int result_code;

	ClearAllContexts();

	KeyValuesParser::UsesEscapeSequences(true);
	KeyValuesParser::KeyValues *kv_filemanager = KeyValuesParser::LoadFromFile("svencoop/clientmenu/clientmenu.txt", &result_code);

	if ( result_code == KeyValuesParser::PARSE_FAILED )
	{
		Warning("[ClientMenu] Failed to parse file \"../svencoop/clientmenu/clientmenu.txt\". Reason: %s (%d)\n", KeyValuesParser::GetLastErrorMessage(), KeyValuesParser::GetLastErrorLine());
		return false;
	}

	if ( kv_filemanager == NULL )
	{
		Warning("[ClientMenu] File \"../svencoop/clientmenu/clientmenu.txt\" is empty\n");
		return false;
	}

	if ( kv_filemanager->Key() != "clientmenu/clientmenu.txt" )
	{
		Warning("[ClientMenu] Expected \"clientmenu/clientmenu.txt\" as main section in the file \"../svencoop/clientmenu/clientmenu.txt\"\n");

		delete kv_filemanager;
		return false;
	}

	// fucking mess
	for (size_t i = 0; i < kv_filemanager->GetList().size(); i++)
	{
		KeyValuesParser::KeyValues *file = kv_filemanager->GetList()[i];

		if ( !file->IsSection() )
		{
			if ( !(file->Value() == "0" || file->Value() == "false") )
			{
				const char *pszExt = NULL;
				std::string sPath = "svencoop/clientmenu/";

				const char *pszFilename = file->Key().c_str();

				while (*pszFilename)
				{
					if (*pszFilename == '.')
					{
						pszExt = pszFilename;
						break;
					}

					pszFilename++;
				}

				if (pszExt != NULL)
				{
					if ( stricmp(pszExt, ".txt") )
					{
						Warning("[ClientMenu] Extension of file \"%s\" isn't supported. Use \"*.txt\" extension\n", pszFilename);
						continue;
					}

					sPath += file->Key();
				}
				else
				{
					sPath += file->Key() + ".txt";
				}

				KeyValuesParser::UsesEscapeSequences(true);
				KeyValuesParser::KeyValues *kv_clientmenu = KeyValuesParser::LoadFromFile(sPath.c_str(), &result_code);

				if ( result_code == KeyValuesParser::PARSE_FAILED )
				{
					Warning("[ClientMenu] Failed to parse file \"../%s\". Reason: %s (%d)\n", sPath.c_str(), KeyValuesParser::GetLastErrorMessage(), KeyValuesParser::GetLastErrorLine());
					continue;
				}

				if ( kv_clientmenu == NULL )
				{
					Warning("[ClientMenu] File \"../%s\" is empty\n", sPath.c_str());
					continue;
				}

				if ( kv_clientmenu->Key() == "ClientMenu" )
				{
					for (size_t j = 0; j < kv_clientmenu->GetList().size(); j++)
					{
						KeyValuesParser::KeyValues *menu = kv_clientmenu->GetList()[j];

						if ( menu->IsSection() && menu->Key().size() > 0 )
						{
							CClientMenuContext *pMenuContext = new CClientMenuContext();

							for (size_t k = 0; k < menu->GetList().size(); k++)
							{
								KeyValuesParser::KeyValues *menu_item = menu->GetList()[k];

								if ( menu_item->IsSection() )
								{
									if ( menu_item->Key().size() == 1 && menu_item->Key()[0] >= '0' && menu_item->Key()[0] <= '9' )
									{
										int label_num = menu_item->Key()[0] - '0';

										for (size_t l = 0; l < menu_item->GetList().size(); l++)
										{
											KeyValuesParser::KeyValues *label_item = menu_item->GetList()[l];

											if ( label_item->Key() == "command" )
											{
												pMenuContext->FeedCommand( label_item->Value().c_str(), label_num );
											}
											else if ( label_item->Key() == "label" )
											{
												pMenuContext->FeedLabel( label_item->Value().c_str(), label_num );
											}
											else if ( label_item->Key() == "flags" )
											{
												pMenuContext->FeedFlags( atoi( label_item->Value().c_str() ), label_num );
											}
										}
									}
								}
								else if ( menu_item->Key() == "Title" )
								{
									pMenuContext->FeedTitle( menu_item->Value().c_str() );
								}
							}

							m_Menus.insert( std::pair<std::string, CClientMenuContext *>(menu->Key(), pMenuContext) );
						}
					}

					Msg("Loaded client menu \"%s\"\n", file->Key().c_str());
				}
				else
				{
					Warning("[ClientMenu] Expected \"ClientMenu\" as main section in the file \"../%s\"\n", sPath.c_str());
				}

				delete kv_clientmenu;
			}
		}
	}

	delete kv_filemanager;

	return true;
}

void CClientMenu::Draw()
{
	if ( m_pCurrentMenu != NULL )
	{
		m_pCurrentMenu->Draw( cl_menu_title_color.GetColor(), cl_menu_label_color.GetColor(), cl_menu_label_number_color.GetColor() );
	}
	else if ( m_pFadeMenu != NULL )
	{
		float flTime = g_pEngineFuncs->GetClientTime();

		if ( flTime > m_flFadeTime || m_flFadeDuration <= 0.f )
		{
			ResetMenu();
			return;
		}

		float fraction = ( m_flFadeTime - flTime ) / m_flFadeDuration;

		Color titleColor = m_TitleColor;
		Color labelColor = m_LabelColor;
		Color labelNumberColor = m_LabelNumberColor;

		titleColor.a *= fraction;
		labelColor.a *= fraction;
		labelNumberColor.a *= fraction;

		m_pFadeMenu->Draw( titleColor, labelColor, labelNumberColor );
	}
}

void CClientMenu::OnGameFrame()
{
	cl_menu_width_fraction.Clamp();
	cl_menu_height_fraction.Clamp();
	cl_menu_duration.Clamp();
	cl_menu_fade_duration.Clamp();

	if ( cl_menu_duration.GetInt() != -1 )
	{
		float flTime = g_pEngineFuncs->GetClientTime();

		if ( flTime > m_flMenuCloseTime )
		{
			CloseMenu();
		}
	}
}

void CClientMenu::OnVideoInit()
{
	ResetMenu();
}

bool CClientMenu::HandleSlotInput(int slot)
{
	if ( IsMenuVisible() )
	{
		m_pCurrentMenu->SlotInput(slot);

		CloseMenu();

		return true;
	}

	return false;
}

void CClientMenu::ShowMenu(const char *pszName)
{
	ResetMenu();

	auto found = m_Menus.find( pszName );

	if ( found != m_Menus.end() )
	{
		m_pCurrentMenu = m_Menus.at( pszName );
		m_flMenuCloseTime = g_pEngineFuncs->GetClientTime() + cl_menu_duration.GetFloat();
	}
}

void CClientMenu::CloseMenu()
{
	if ( IsMenuVisible() )
	{
		m_pFadeMenu = m_pCurrentMenu;
		m_pCurrentMenu = NULL;

		m_flMenuCloseTime = -1.f;

		m_flFadeDuration = cl_menu_fade_duration.GetFloat();
		m_flFadeTime = g_pEngineFuncs->GetClientTime() + m_flFadeDuration;

		m_TitleColor = cl_menu_title_color.GetColor();
		m_LabelColor = cl_menu_label_color.GetColor();
		m_LabelNumberColor = cl_menu_label_number_color.GetColor();
	}
}

void CClientMenu::ResetMenu()
{
	m_pCurrentMenu = NULL;
	m_pFadeMenu = NULL;

	m_flMenuCloseTime = -1.f;

	m_flFadeTime = -1.f;
	m_flFadeDuration = 0.f;
}

bool CClientMenu::IsMenuVisible()
{
	return m_pCurrentMenu != NULL;
}

void CClientMenu::ClearAllContexts()
{
	ResetMenu();

	for (const std::pair<std::string, CClientMenuContext *> &pair : m_Menus)
	{
		const_cast<std::string *>(&pair.first)->clear();
		delete pair.second;
	}

	m_Menus.clear();
}

//-----------------------------------------------------------------------------
// CClientMenu feature
//-----------------------------------------------------------------------------

CClientMenu::CClientMenu() : m_TitleColor(255, 255, 255, 255), m_LabelColor(255, 255, 255, 255), m_LabelNumberColor(255, 255, 255, 255)
{
	m_pCurrentMenu = NULL;
	m_pFadeMenu = NULL;

	m_flMenuCloseTime = -1.f;

	m_flFadeTime = -1.f;
	m_flFadeDuration = 0.f;

	m_hslot10 = 0;
	m_hslot1 = 0;
	m_hslot2 = 0;
	m_hslot3 = 0;
	m_hslot4 = 0;
	m_hslot5 = 0;
	m_hslot6 = 0;
	m_hslot7 = 0;
	m_hslot8 = 0;
	m_hslot9 = 0;
}

bool CClientMenu::Load()
{
	ud_t inst;
	unsigned char *pCallOpcode;

	int iDisassembledBytes = 0;
	void *pfnDrawCharacter = g_pEngineFuncs->DrawCharacter;

	if ( *(unsigned char *)pfnDrawCharacter == 0xE9 ) // JMP
	{
		pfnDrawCharacter = MemoryUtils()->CalcAbsoluteAddress(pfnDrawCharacter);
	}
	else
	{
		Warning("Failed to locate JMP op-code for function \"DrawCharacter\"\n");
		return false;
	}

	MemoryUtils()->InitDisasm(&inst, pfnDrawCharacter, 32, 48);
	
	pCallOpcode = (unsigned char *)pfnDrawCharacter;

	do
	{
		if ( inst.mnemonic == UD_Icall )
		{
			VGUI2_GetCreditsFont = (VGUI2_GetCreditsFontFn)MemoryUtils()->CalcAbsoluteAddress( pCallOpcode );
			break;
		}

		pCallOpcode += iDisassembledBytes;

	} while ( iDisassembledBytes = MemoryUtils()->Disassemble(&inst) );

	if ( VGUI2_GetCreditsFont == NULL )
	{
		Warning("Failed to get function \"VGUI2_GetCreditsFont\"\n");
		return false;
	}

	return true;
}

void CClientMenu::PostLoad()
{
	if ( LoadFromFile() )
	{
		Msg("Loaded file \"../svencoop/clientmenu/clientmenu.txt\"\n");
	}

	m_hslot10 = Hooks()->HookConsoleCommand( "slot10", HOOKED_slot10, &ORIG_slot10 );
	m_hslot1 = Hooks()->HookConsoleCommand( "slot1", HOOKED_slot1, &ORIG_slot1 );
	m_hslot2 = Hooks()->HookConsoleCommand( "slot2", HOOKED_slot2, &ORIG_slot2 );
	m_hslot3 = Hooks()->HookConsoleCommand( "slot3", HOOKED_slot3, &ORIG_slot3 );
	m_hslot4 = Hooks()->HookConsoleCommand( "slot4", HOOKED_slot4, &ORIG_slot4 );
	m_hslot5 = Hooks()->HookConsoleCommand( "slot5", HOOKED_slot5, &ORIG_slot5 );
	m_hslot6 = Hooks()->HookConsoleCommand( "slot6", HOOKED_slot6, &ORIG_slot6 );
	m_hslot7 = Hooks()->HookConsoleCommand( "slot7", HOOKED_slot7, &ORIG_slot7 );
	m_hslot8 = Hooks()->HookConsoleCommand( "slot8", HOOKED_slot8, &ORIG_slot8 );
	m_hslot9 = Hooks()->HookConsoleCommand( "slot9", HOOKED_slot9, &ORIG_slot9 );
}

void CClientMenu::Unload()
{
	ClearAllContexts();

	Hooks()->UnhookConsoleCommand( m_hslot10 );
	Hooks()->UnhookConsoleCommand( m_hslot1 );
	Hooks()->UnhookConsoleCommand( m_hslot2 );
	Hooks()->UnhookConsoleCommand( m_hslot3 );
	Hooks()->UnhookConsoleCommand( m_hslot4 );
	Hooks()->UnhookConsoleCommand( m_hslot5 );
	Hooks()->UnhookConsoleCommand( m_hslot6 );
	Hooks()->UnhookConsoleCommand( m_hslot7 );
	Hooks()->UnhookConsoleCommand( m_hslot8 );
	Hooks()->UnhookConsoleCommand( m_hslot9 );
}