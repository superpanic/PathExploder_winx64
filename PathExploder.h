#pragma once
#include "IllustratorSDK.h"
#include "AITransformArt.h"
#include "AIAnnotator.h"
#include "AIAnnotatorDrawer.h"
#include "MathTools.h"

#define kIconID			23230
#define kToolTitle		"Path Exploder"
#define kToolTip		"Path Explode Tool"
#define kTempHPos		"-HPos"
#define kTempVPos		"-VPos"
#define kTempHTransform		"-HTransform"
#define kTempVTransform		"-VTransform"
#define kTempOriginH		"-HOrigin"
#define kTempOriginV		"-VOrigin"


constexpr ai::int32 kAnimationSpeed = 1;
constexpr AIReal kExplosionLength = 100.0;

class PathExploder {
	
	typedef struct {
		AINotifierHandle selectionNotifierHandle;
		AIToolHandle toolHandle;
		AIAnnotatorHandle annotatorHandle;
		AIArtHandle **selectedArt;
		ai::int32 selectedArtCount; // number of total selected art
		ai::int32 selectedPathCount; // number of selected art that is of type path
		AIBoolean selectedArtIsExploded;
		short animationStep;
		AITimerHandle timerHandle;
	} Globals;

	public:
		PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message);
		~PathExploder();

		void AcquireSuites(SPBasicSuite *sSPBasic);
		void ReleaseSuites(SPBasicSuite *sSPBasic);

		ASErr Message(char *caller, char *selector, void *message);

		void FreeSelectedArt();
		void DeactivateTimer();
		void FreeGlobals(SPInterfaceMessage *message);

	private:
		SPPlugin *plugin;
		Globals *g = nullptr;
		MathTools *mathTools;

		void AddSelectionNotifier();
		void AddAnnotator();
		void SetupTimer();

		ASErr CreateTool();

		ASErr Alert(const char *s);

		AIErr GetCurrentPosition(const AIArtHandle &art, AIRealPoint *currentPos);

		ASErr Move(AIArtHandle art, AIReal transH, AIReal transV);
		AIErr MoveTo(const AIArtHandle &art, AIRealPoint *absPos);
		AIErr MoveToFraction(const AIArtHandle &art, AIRealPoint *origin, AIRealPoint *destination, AIReal fraction);

		AIErr ReadTempOriginFromDictionary(const AIArtHandle &art, AIRealPoint *origin);
		AIErr ReadTempTransformFromDict(const AIArtHandle &art, AIRealPoint *transform);

		AIErr WriteTempOriginAndTransformToDict(const AIArtHandle &art, AIRealPoint *transform);
		AIErr WriteTempTransformToDict(const AIArtHandle &art, AIReal transH, AIReal transV);

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