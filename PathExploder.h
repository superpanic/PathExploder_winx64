#pragma once
#include "IllustratorSDK.h"
#include "AITransformArt.h"
#include "AIAnnotator.h"
#include "AIAnnotatorDrawer.h"

#define kIconID	    23230
#define kToolTitle  "Path Exploder"
#define kToolTip    "Path Explode Tool"

class PathExploder {
	
	typedef struct {
		AINotifierHandle selectionNotifierHandle;
		AIToolHandle toolHandle;
		AIArtHandle **selectedArt;
		AIAnnotatorHandle annotatorHandle;
	} Globals;

	public:
		PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message);
		~PathExploder() {};
		ASErr Message(char *caller, char *selector, void *message);
		void FreeSelectedArt(SPBasicSuite * sSPBasic);
		void FreeGlobals(SPInterfaceMessage *message);

	private:
		SPPlugin *plugin;
		Globals *g = nullptr;

		void AddSelectionNotifier(SPBasicSuite * sSPBasic);
		void AddAnnotator(SPBasicSuite * sSPBasic);
		ASErr CreateTool(SPBasicSuite * sSPBasic);
		ASErr Move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal tx, AIReal ty);
		ASErr Alert(SPBasicSuite *sSPBasic, const char *s);

};
