#pragma once
#include "IllustratorSDK.h"
#include "AITransformArt.h"

class PathExploder {
	
	SPPlugin *plugin;

	typedef struct {
		AINotifierHandle selectionNotifierHandle;
	} Globals;

	Globals *g = nullptr;

	public:
		PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message);
		~PathExploder() {};
		void FreeGlobals(SPInterfaceMessage *message);
		ASErr Message(char *caller, char *selector, void *message);
		ASErr Alert(SPBasicSuite *sSPBasic, const char *s);
		ASErr move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal tx, AIReal ty);
		ASErr transformPathArtWithParameters(AIArtHandle art, AIReal tx, AIReal ty, AIReal theta, AIReal sx, AIReal sy);
};
