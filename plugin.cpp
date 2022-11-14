#include <IClientPlugin.h>
#include <interface.h>
#include <base_feature.h>

#include <convar.h>
#include <dbg.h>

#include "clientmenu.h"
#include "client_hooks.h"

//-----------------------------------------------------------------------------
// Client Menu plugin interface
//-----------------------------------------------------------------------------

class CClientMenuPlugin : public IClientPlugin
{
public:
	virtual api_version_s GetAPIVersion();

	virtual bool Load(CreateInterfaceFn pfnSvenModFactory, ISvenModAPI *pSvenModAPI, IPluginHelpers *pPluginHelpers);

	virtual void PostLoad(bool bGlobalLoad);

	virtual void Unload(void);

	virtual bool Pause(void);

	virtual void Unpause(void);

	virtual void OnFirstClientdataReceived(client_data_t *pcldata, float flTime);

	virtual void OnBeginLoading(void);

	virtual void OnEndLoading(void);

	virtual void OnDisconnect(void);

	virtual void GameFrame(client_state_t state, double frametime, bool bPostRunCmd);

	virtual void Draw(void);

	virtual void DrawHUD(float time, int intermission);

	virtual const char *GetName(void);

	virtual const char *GetAuthor(void);

	virtual const char *GetVersion(void);

	virtual const char *GetDescription(void);

	virtual const char *GetURL(void);

	virtual const char *GetDate(void);

	virtual const char *GetLogTag(void);
};

//-----------------------------------------------------------------------------
// Plugin's implementation
//-----------------------------------------------------------------------------

api_version_s CClientMenuPlugin::GetAPIVersion()
{
	return SVENMOD_API_VER;
}

bool CClientMenuPlugin::Load(CreateInterfaceFn pfnSvenModFactory, ISvenModAPI *pSvenModAPI, IPluginHelpers *pPluginHelpers)
{
	BindApiToGlobals(pSvenModAPI);

	if ( !LoadFeatures() )
	{
		Warning("Failed to initialize ClientMenu feature\n");
		return false;
	}

	ConVar_Register();

	g_pEngineFuncs->ClientCmd("exec clientmenu.cfg\n");

	ConColorMsg({ 40, 255, 40, 255 }, "[ClientMenu] Successfully loaded\n");
	return true;
}

void CClientMenuPlugin::PostLoad(bool bGlobalLoad)
{
	if (bGlobalLoad)
	{
	}
	else
	{
	}

	Hooks()->RegisterClientHooks( &g_ClientHooks );

	PostLoadFeatures();
}

void CClientMenuPlugin::Unload(void)
{
	Hooks()->UnregisterClientHooks(&g_ClientHooks);

	UnloadFeatures();

	ConVar_Unregister();
}

bool CClientMenuPlugin::Pause(void)
{
	return true;
}

void CClientMenuPlugin::Unpause(void)
{

}

void CClientMenuPlugin::GameFrame(client_state_t state, double frametime, bool bPostRunCmd)
{
	if ( !bPostRunCmd && state == CLS_ACTIVE )
	{
		g_ClientMenu.OnGameFrame();
	}
}

void CClientMenuPlugin::OnFirstClientdataReceived(client_data_t *pcldata, float flTime)
{
}

void CClientMenuPlugin::OnBeginLoading(void)
{
}

void CClientMenuPlugin::OnEndLoading(void)
{
}

void CClientMenuPlugin::OnDisconnect(void)
{
}

void CClientMenuPlugin::Draw(void)
{
	g_ClientMenu.Draw();
}

void CClientMenuPlugin::DrawHUD(float time, int intermission)
{
}

const char *CClientMenuPlugin::GetName(void)
{
	return "Client Menu";
}

const char *CClientMenuPlugin::GetAuthor(void)
{
	return "Sw1ft";
}

const char *CClientMenuPlugin::GetVersion(void)
{
	return "1.1.0";
}

const char *CClientMenuPlugin::GetDescription(void)
{
	return "Customizable client-side menu";
}

const char *CClientMenuPlugin::GetURL(void)
{
	return "https://github.com/sw1ft747/clientmenu";
}

const char *CClientMenuPlugin::GetDate(void)
{
	return SVENMOD_BUILD_TIMESTAMP;
}

const char *CClientMenuPlugin::GetLogTag(void)
{
	return "CLMENU";
}

//-----------------------------------------------------------------------------
// Export the interface
//-----------------------------------------------------------------------------

EXPOSE_SINGLE_INTERFACE(CClientMenuPlugin, IClientPlugin, CLIENT_PLUGIN_INTERFACE_VERSION);