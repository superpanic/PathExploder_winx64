#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "IllustratorSDK.h"
#include "PathExploder.h"

// Tell Xcode to export the following symbols
#if defined(__GNUC__)
#pragma GCC visibility push(default)
#endif

// Plug-in entry point
extern "C" ASAPI ASErr PluginMain(char * caller, char* selector, void* message);

// Tell Xcode to return to default visibility for symbols
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

PathExploder *pathExploder;

extern "C" ASAPI ASErr PluginMain(char* caller, char* selector, void* message) {

	ASErr error = kNoErr;
	SPMessageData *msgData = (SPMessageData *)message;
	SPBasicSuite *sSPBasic = msgData->basic;

	if (sSPBasic->IsEqual(caller, kSPInterfaceCaller)) {
		if (sSPBasic->IsEqual(selector, kSPInterfaceStartupSelector)) {
			pathExploder = new PathExploder(msgData->self, sSPBasic, (SPInterfaceMessage*)message );
		} else if (sSPBasic->IsEqual(selector, kSPInterfaceShutdownSelector)) {
			pathExploder->FreeSelectedArt(sSPBasic);
			pathExploder->FreeGlobals( (SPInterfaceMessage*)message );
			pathExploder->ReleaseSuites(sSPBasic);
			delete pathExploder;
		}
	} else {
		if (pathExploder) pathExploder->Message(caller, selector, message);
	}

	return error;

}

