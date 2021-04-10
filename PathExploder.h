#pragma once
#include "IllustratorSDK.h"
#include "AITransformArt.h"
#include "AIAnnotator.h"
#include "AIAnnotatorDrawer.h"
#include "MathTools.h"

#define kIconID	    23230
#define kToolTitle  "Path Exploder"
#define kToolTip    "Path Explode Tool"
#define kTempHPos   "-HPos"
#define kTempVPos   "-VPos"
#define kExplosionLength 100.0

class PathExploder {
	
	typedef struct {
		AINotifierHandle selectionNotifierHandle;
		AIToolHandle toolHandle;
		AIAnnotatorHandle annotatorHandle;
		AIArtHandle **selectedArt;
		ai::int32 selectedArtCount;
		AIBoolean selectedArtIsExploded;
	} Globals;

	public:
		PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message);
		~PathExploder();
		void AcquireSuites(SPBasicSuite *sSPBasic);
		void ReleaseSuites(SPBasicSuite *sSPBasic);

		ASErr Message(char *caller, char *selector, void *message);
		void FreeSelectedArt(SPBasicSuite * sSPBasic);
		void FreeGlobals(SPInterfaceMessage *message);

	private:
		SPPlugin *plugin;
		Globals *g = nullptr;
		MathTools *mathTools;

		void AddSelectionNotifier(SPBasicSuite * sSPBasic);
		void AddAnnotator(SPBasicSuite * sSPBasic);
		AIErr WriteCurrentPositionToDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art);
		AIErr WriteTempTransformToDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art, AIReal transH, AIReal transV);
		AIErr ReadAndResetTempTransformFromDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art, AIReal *originalHPos, AIReal *originalVPos);
		ASErr CreateTool(SPBasicSuite * sSPBasic);
		ASErr Move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal transH, AIReal transV);
		ASErr Alert(SPBasicSuite *sSPBasic, const char *s);

};


/*

Reload and unload messages
Whenever a plug-in is loaded into memory or unloaded from memory, Illustrator sends it an access
message action:
#define kSPAccessCaller "SP Access"
#define kSPAccessUnloadSelector "Unload"
#define kSPAccessReloadSelector "Reload"
The message action contains the access caller and a reload or unload selector. This is your plug-in’s
opportunity to set up, restore, or save state information. The access caller/selectors bracket all other callers
and selectors.
Access messages bracket all other messages. Reload is the first message your plug-in receives; unload is
the last. At these times, your plug-in should not acquire or release suites other than those built into
Illustrator.

*/