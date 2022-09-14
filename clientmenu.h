#include <string>
#include <unordered_map>

#include <IDetoursAPI.h>
#include <base_feature.h>
#include <color.h>

#include <vgui2/VGUI2.h>

//-----------------------------------------------------------------------------
// Label Flags
//-----------------------------------------------------------------------------

#define FL_LABEL_INDENT ( 1 << 0 )
#define FL_LABEL_PLAYERNAME ( 1 << 1 )

//-----------------------------------------------------------------------------
// CClientMenuContext
//-----------------------------------------------------------------------------

class CClientMenuContext
{
	friend class CClientMenu;

public:
	CClientMenuContext();
	~CClientMenuContext();

	void Draw(const Color &titleColor, const Color &labelColor, const Color &labelNumberColor);
	void SlotInput(int slot);

public:
	inline const wchar_t *GetTitle() const { return m_pwszTitle; }
	inline const wchar_t *GetLabel(int slot) const { return m_pwszLabels[slot]; }
	inline const char *GetCommand(int slot) const { return m_pszCommands[slot]; }
	inline int GetFlags(int slot) const { return m_fLabelsFlags[slot]; }

	inline bool HasLabel(int slot) const { return m_pwszLabels[slot] != NULL; }

private:
	void DrawPrintText(vgui::HFont font, int x, int y, int r, int g, int b, int alpha, const wchar_t *pwszText);

	void FeedTitle(const wchar_t *pwszTitle);
	void FeedLabel(const wchar_t *pwszLabel, int slot);

	void FeedTitle(const char *pszTitle);
	void FeedLabel(const char *pszLabel, int slot);
	void FeedCommand(const char *pszCommand, int slot);
	void FeedFlags(int flags, int slot);

	void CalcHeight(vgui::HFont font, int &height, int &titleTall, int labelsTall[10]);

private:
	const wchar_t *m_pwszTitle;
	const wchar_t *m_pwszLabels[10];
	const char *m_pszCommands[10];
	int m_fLabelsFlags[10];
};

//-----------------------------------------------------------------------------
// CClientMenu
//-----------------------------------------------------------------------------

class CClientMenu : CBaseFeature
{
public:
	CClientMenu();

	virtual bool Load();
	virtual void PostLoad();

	virtual void Unload();

public:
	void Draw();
	void OnGameFrame();
	void OnVideoInit();

	bool LoadFromFile();

	bool HandleSlotInput(int slot);
	void ShowMenu(const char *pszName);
	void CloseMenu();
	void ResetMenu();

	bool IsMenuVisible();

	void ClearAllContexts();

private:
	CClientMenuContext *m_pCurrentMenu;
	CClientMenuContext *m_pFadeMenu;

	float m_flMenuCloseTime;

	float m_flFadeTime;
	float m_flFadeDuration;

	Color m_TitleColor;
	Color m_LabelColor;
	Color m_LabelNumberColor;

	std::unordered_map<std::string, CClientMenuContext *> m_Menus;

	DetourHandle_t m_hslot10;
	DetourHandle_t m_hslot1;
	DetourHandle_t m_hslot2;
	DetourHandle_t m_hslot3;
	DetourHandle_t m_hslot4;
	DetourHandle_t m_hslot5;
	DetourHandle_t m_hslot6;
	DetourHandle_t m_hslot7;
	DetourHandle_t m_hslot8;
	DetourHandle_t m_hslot9;
};

extern CClientMenu g_ClientMenu;