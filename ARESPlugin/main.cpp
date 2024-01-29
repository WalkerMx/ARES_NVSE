#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/GameObjects.h"
#include "textureData.h"
#include "ColorFunctions.h"
#include <fstream>

std::string ddsPath(".\\Data\\Textures\\Characters\\DaughtersOfAres\\Spine\\spine_63.dds");
byte dxtBytes[262272];
byte ddsBytes[491520];

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

#define REG_CMD(name) 	nvse->RegisterCommand(&kCommandInfo_ ##name)
#define REG_TYPED_CMD(name, type)	nvse->RegisterTypedCommand(&kCommandInfo_##name,kRetnType_##type)

static ParamInfo kParams_OneFloat_ThreeInts[4] =
{
	{	"float", kParamType_Float,	0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
};

static ParamInfo kParams_OneFloat_SixIntegers[7] =
{
	{	"float", kParamType_Float,	0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
	{	"int", kParamType_Integer, 0 },
};

void blockCompress() {

	int dxtIndex = 90240;

	for (int y = 0; y <= 76; y += 4) {
		for (int x = 0; x <= 2044; x += 4) {

			USHORT valueList[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 

			USHORT maxPixelValue = 0x0;
			USHORT minPixelValue = 0xFFFF;

			for (int j = 0; j <= 3; j++) {
				for (int i = 0; i <= 3; i++) {

					int pixelIndex = (j * 4) + i;
					int byteIndex = 3 * (((y + j) * 2048) + (x + i));

					USHORT curValue = rgb888to565(ddsBytes[byteIndex], ddsBytes[byteIndex + 1], ddsBytes[byteIndex + 2]);
					valueList[pixelIndex] = curValue;

					if (curValue > maxPixelValue) { maxPixelValue = curValue; }
					if (curValue < minPixelValue) {	minPixelValue = curValue; }
									
				}
			}

			dxtBytes[dxtIndex] = maxPixelValue & 0xFF;
			dxtBytes[dxtIndex + 1] = (maxPixelValue >> 8) & 0xFF;
			dxtBytes[dxtIndex + 2] = minPixelValue & 0xFF;
			dxtBytes[dxtIndex + 3] = (minPixelValue >> 8) & 0xFF;
			dxtIndex += 4;

			int stepVal = floor((maxPixelValue - minPixelValue) * 0.3333333);

			for (int j = 0; j <= 3; j++) {
				byte bitString = 0b00000000;
				for (int i = 3; i >= 0; i--) {
					int pixelIndex = (j * 4) + i;
					USHORT curValue = valueList[pixelIndex];
					bitString <<= 2;
					if (curValue >= (stepVal * 2) + minPixelValue) {
						bitString |= 0b00000000;
					} else if (curValue >= stepVal + minPixelValue) {
						bitString |= 0b00000010;
					} else if (curValue >= minPixelValue) {
						bitString |= 0b00000011;
					} else {
						bitString |= 0b00000001;
					}
				}
				dxtBytes[dxtIndex] = bitString;
				dxtIndex += 1;
			}

		}
	}

}

void drawToArray(int X, int R, int G, int B)
{
	for (int i = X; i <= (X + 10); i++) {
		for (int j = 28; j <= 51; j++) {
			int index = 3 * ((j * 2048) + i);
			ddsBytes[index] = R;
			ddsBytes[index + 1] = G;
			ddsBytes[index + 2] = B;
		}
	}
}

void drawHP(float HP, int R, int G, int B)
{
	int range = 64 - floor(63 * HP);
	for (int i = range; i <= 63; i++) {
		drawToArray(xIndicies[i - 1], R, G, B);
	}
}

bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

DEFINE_COMMAND_PLUGIN(CreateSpineTexture, "Creates DDS Texture for Spine Lighting", false, kParams_OneFloat_ThreeInts)
bool Cmd_CreateSpineTexture_Execute(COMMAND_ARGS)
{
	float hpVal;
	int rVal, gVal, bVal;
	*result = 0;
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &hpVal, &rVal, &gVal, &bVal)) {

		ddsPath = ".\\Data\\Textures\\Characters\\DaughtersOfAres\\Spine\\spine_" + std::to_string((int)floor(63 * hpVal)) + ".dds";

		if (!fileExists(ddsPath)) {
			std::copy(headerBytes, headerBytes + 128, dxtBytes);
			std::copy(defaultBytes, defaultBytes + 491520, ddsBytes);
			drawHP(hpVal, rVal, gVal, bVal);
			blockCompress();
			std::ofstream(ddsPath, std::ios::binary).write((char*)&dxtBytes, sizeof(dxtBytes));
		}

		*result = 1;
	}
	return true;
}

DEFINE_COMMAND_PLUGIN(ClearSpineCache, "Clears DDS Spine Cache", false, NULL)
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

DEFINE_COMMAND_PLUGIN(GetColorFade, "Interpolates between two colors.", false, kParams_OneFloat_SixIntegers)
bool Cmd_GetColorFade_Execute(COMMAND_ARGS)
{
	*result = 0;
	float t;
	int r1, g1, b1, r2, g2, b2;
	NVSEArrayVarInterface::Array* lerpColor = g_arrayInterface->CreateArray(NULL, 0, scriptObj);
	if (ExtractArgsEx(EXTRACT_ARGS_EX, &t, &r1, &g1, &b1, &r2, &g2, &b2)) {
		colorRGB lerpResult = getColorFade(t, colorRGB(r1, g1, b1), colorRGB(r2, g2, b2));
		g_arrayInterface->AppendElement(lerpColor, NVSEArrayVarInterface::Element(lerpResult.r));
		g_arrayInterface->AppendElement(lerpColor, NVSEArrayVarInterface::Element(lerpResult.g));
		g_arrayInterface->AppendElement(lerpColor, NVSEArrayVarInterface::Element(lerpResult.b));
		g_arrayInterface->AssignCommandResult(lerpColor, result);
	}
	*result = 1;
	return true;
}

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
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ARESPlugin";
	info->version = 240;
	if (nvse->nvseVersion < PACKED_NVSE_VERSION) { _ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, PACKED_NVSE_VERSION); return false; }
	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525) { _ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525); return false; }
		if (nvse->isNogore) { _ERROR("NoGore is not supported"); return false; }
	}
	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{ _ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518); return false; }
	}
	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{
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

	decodeRLE();

	/***************************************************************************
	 *
	 *	See https://geckwiki.com/index.php?title=NVSE_Opcode_Base
	 *
	 **************************************************************************/

	// Using Opcode range 0x3FF0 - 0x3FFF
	UInt32 const ARESOpcodeBase = 0x3FF0;
	nvse->SetOpcodeBase(ARESOpcodeBase);
	/*3FF0*/ REG_CMD(CreateSpineTexture);
	/*3FF1*/ REG_CMD(ClearSpineCache);
	/*3FF2*/ REG_TYPED_CMD(GetColorFade, Array);

	return true;
}
