#pragma once
#include "IllustratorSDK.h"
#include "AITransformArt.h"
#include "AIAnnotator.h"
#include "AIAnnotatorDrawer.h"

#define kIconID	    23230
#define kToolTitle  "Path Exploder"
#define kToolTip    "Path Explode Tool"
#define kTempHPos   "-HPos"
#define kTempVPos   "-VPos"

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
		~PathExploder() {};
		void AcquireSuites(SPBasicSuite *sSPBasic);
		void ReleaseSuites(SPBasicSuite *sSPBasic);

		ASErr Message(char *caller, char *selector, void *message);
		void FreeSelectedArt(SPBasicSuite * sSPBasic);
		void FreeGlobals(SPInterfaceMessage *message);

	private:
		SPPlugin *plugin;
		Globals *g = nullptr;

		void AddSelectionNotifier(SPBasicSuite * sSPBasic);
		void AddAnnotator(SPBasicSuite * sSPBasic);
		AIErr WriteCurrentPositionToDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art);
		AIErr WriteTempTransformToDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art, AIReal transH, AIReal transV);
		AIErr ReadAndResetTempTransformFromDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art, AIReal *originalHPos, AIReal *originalVPos);
		ASErr CreateTool(SPBasicSuite * sSPBasic);
		ASErr Move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal transH, AIReal transV);
		ASErr Alert(SPBasicSuite *sSPBasic, const char *s);

};
