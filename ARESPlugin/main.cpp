#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/GameObjects.h"
#include "textureData.h"
#include <string>
#include <fstream>

byte ddsBytes[2097152];
std::string ddsPath(".\\Data\\Textures\\Characters\\DaughtersOfAres\\Spine\\spine_63.dds");

IDebugLog		gLog("ARESPlugin.log");
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSECommandTableInterface* g_cmdTableInterface{};

#if RUNTIME
NVSEScriptInterface* g_script{};
NVSEStringVarInterface* g_stringInterface{};
NVSEArrayVarInterface* g_arrayInterface{};
NVSEDataInterface* g_dataInterface{};
NVSESerializationInterface* g_serializationInterface{};
NVSEConsoleInterface* g_consoleInterface{};
NVSEEventManagerInterface* g_eventInterface{};
bool (*ExtractArgsEx)(COMMAND_ARGS_EX, ...);
#endif

static ParamInfo kParams_OneFloat_ThreeInts[4] =
{
	{	"float", kParamType_Float,	0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
};

void DrawToArray(int X, int R, int G, int B)
{
	for (int i = X; i <= (X + 10); i++) {
		for (int j = 116; j <= 139; j++) {
			int index = 4 * ((j * 2048) + i);
			ddsBytes[index] = R;
			ddsBytes[index + 1] = G;
			ddsBytes[index + 2] = B;
			ddsBytes[index + 3] = 255;
		}
	}
}

void DrawHP(float HP, int R, int G, int B)
{
	int range = floor(63 * HP);

	for (int i = (64 - range); i <= 63; i++) {
		DrawToArray(xIndicies[i - 1], R, G, B);
	}
}

bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

DEFINE_COMMAND_PLUGIN(CreateSpineTexture, "Creates DDS Texture for Spine Lighting", false, kParams_OneFloat_ThreeInts)
#if RUNTIME
bool Cmd_CreateSpineTexture_Execute(COMMAND_ARGS)
{
	float hpVal;
	int rVal, gVal, bVal;
	*result = 0;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &hpVal, &rVal, &gVal, &bVal)) {

		ddsPath = ".\\Data\\Textures\\Characters\\DaughtersOfAres\\Spine\\spine_" + std::to_string((int)floor(63 * hpVal)) + ".dds";

		if (!fileExists(ddsPath)) {
			std::copy(defaultBytes, defaultBytes + 620480, ddsBytes + 738068);
			DrawHP(hpVal, rVal, gVal, bVal);
			std::ofstream(ddsPath, std::ios::binary).write((char*)&headerBytes, sizeof(headerBytes));
			std::ofstream(ddsPath, std::ios::binary | std::ios_base::app).write((char*)&ddsBytes, sizeof(ddsBytes));
		}

		*result = 1;
	}
	return true;
}
#endif

DEFINE_COMMAND_PLUGIN(ClearSpineCache, "Clears DDS Spine Cache", false, NULL)
#if RUNTIME
bool Cmd_ClearSpineCache_Execute(COMMAND_ARGS)
{
	*result = 0;
	std::string tempPath;
	for (int i = 0; i <= 63; i++) {
		tempPath = ".\\Data\\Textures\\Characters\\DaughtersOfAres\\Spine\\spine_" + std::to_string(i) + ".dds";
		remove(tempPath.c_str());
	}
	*result = 1;
	return true;
}
#endif

#define RegisterScriptCommand(name) 	nvse->RegisterCommand(&kCommandInfo_ ##name)
#define REG_CMD(name) RegisterScriptCommand(name)

void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case NVSEMessagingInterface::kMessage_PostLoad: break;
	case NVSEMessagingInterface::kMessage_ExitGame: break;
	case NVSEMessagingInterface::kMessage_ExitToMainMenu: break;
	case NVSEMessagingInterface::kMessage_LoadGame: break;
	case NVSEMessagingInterface::kMessage_SaveGame: break;
#if EDITOR
	case NVSEMessagingInterface::kMessage_ScriptEditorPrecompile: break;
#endif
	case NVSEMessagingInterface::kMessage_PreLoadGame: break;
	case NVSEMessagingInterface::kMessage_ExitGame_Console: break;
	case NVSEMessagingInterface::kMessage_PostLoadGame: break;
	case NVSEMessagingInterface::kMessage_PostPostLoad: break;
	case NVSEMessagingInterface::kMessage_RuntimeScriptError: break;
	case NVSEMessagingInterface::kMessage_DeleteGame: break;
	case NVSEMessagingInterface::kMessage_RenameGame: break;
	case NVSEMessagingInterface::kMessage_RenameNewGame: break;
	case NVSEMessagingInterface::kMessage_NewGame: break;
	case NVSEMessagingInterface::kMessage_DeleteGameName: break;
	case NVSEMessagingInterface::kMessage_RenameGameName: break;
	case NVSEMessagingInterface::kMessage_RenameNewGameName: break;
	case NVSEMessagingInterface::kMessage_DeferredInit: break;
	case NVSEMessagingInterface::kMessage_ClearScriptDataCache: break;
	case NVSEMessagingInterface::kMessage_MainGameLoop: break;
	case NVSEMessagingInterface::kMessage_ScriptCompile: break;
	case NVSEMessagingInterface::kMessage_EventListDestroyed: break;
	case NVSEMessagingInterface::kMessage_PostQueryPlugins: break;
	default: break;
	}
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ARESPlugin";
	info->version = 100;
	if (nvse->nvseVersion < PACKED_NVSE_VERSION)
	{
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, PACKED_NVSE_VERSION);
		return false;
	}
	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}
		if (nvse->isNogore)
		{
			_ERROR("NoGore is not supported");
			return false;
		}
	}
	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	}
	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{
	_MESSAGE("load");
	g_pluginHandle = nvse->GetPluginHandle();
	g_nvseInterface = nvse;
	g_messagingInterface = static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(kInterface_Messaging));
	g_messagingInterface->RegisterListener(g_pluginHandle, "NVSE", MessageHandler);
	if (!nvse->isEditor)
	{
#if RUNTIME
		g_script = static_cast<NVSEScriptInterface*>(nvse->QueryInterface(kInterface_Script));
		g_stringInterface = static_cast<NVSEStringVarInterface*>(nvse->QueryInterface(kInterface_StringVar));
		g_arrayInterface = static_cast<NVSEArrayVarInterface*>(nvse->QueryInterface(kInterface_ArrayVar));
		g_dataInterface = static_cast<NVSEDataInterface*>(nvse->QueryInterface(kInterface_Data));
		g_eventInterface = static_cast<NVSEEventManagerInterface*>(nvse->QueryInterface(kInterface_EventManager));
		g_serializationInterface = static_cast<NVSESerializationInterface*>(nvse->QueryInterface(kInterface_Serialization));
		g_consoleInterface = static_cast<NVSEConsoleInterface*>(nvse->QueryInterface(kInterface_Console));
		ExtractArgsEx = g_script->ExtractArgsEx;
#endif
	}

	/***************************************************************************
	 *
	 *	READ THIS!
	 *
	 *	Before releasing your plugin, you need to request an opcode range from
	 *	the NVSE team and set it in your first SetOpcodeBase call. If you do not
	 *	do this, your plugin will create major compatibility issues with other
	 *	plugins, and will not load in release versions of NVSE. See
	 *	nvse_readme.txt for more information.
	 *
	 *	See https://geckwiki.com/index.php?title=NVSE_Opcode_Base
	 *
	 **************************************************************************/

	// Using Opcode range 0x3FF0 - 0x3FFF for testing
	UInt32 const ARESOpcodeBase = 0x3FF0;
	nvse->SetOpcodeBase(ARESOpcodeBase);
	/*3FF0*/ REG_CMD(CreateSpineTexture);
	/*3FF1*/ REG_CMD(ClearSpineCache);

	return true;
}
