#pragma once
#include "IllustratorSDK.h"
#include "AITransformArt.h"

#define kIconID	    23230
#define kToolTitle  "Path Exploder"
#define kToolTip    "Path Explode Tool"

class PathExploder {
	
	SPPlugin *plugin;

	typedef struct {
		AINotifierHandle selectionNotifierHandle;
		AIToolHandle toolHandle;
	} Globals;

	Globals *g = nullptr;

	public:
		PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message);
		void AddSelectionNotifier(SPBasicSuite * sSPBasic);
		ASErr CreateTool(SPBasicSuite * sSPBasic);
		~PathExploder() {};
		void FreeGlobals(SPInterfaceMessage *message);
		ASErr Message(char *caller, char *selector, void *message);
		ASErr Alert(SPBasicSuite *sSPBasic, const char *s);
		ASErr move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal tx, AIReal ty);
		ASErr transformPathArtWithParameters(AIArtHandle art, AIReal tx, AIReal ty, AIReal theta, AIReal sx, AIReal sy);
};
