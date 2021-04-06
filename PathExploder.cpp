#include "PathExploder.h"

extern "C" {
	AIUnicodeStringSuite *sAIUnicodeString = NULL;
	SPBlocksSuite *sSPBlocks = NULL;
	AIUserSuite *sAIUser = NULL;
	AIMatchingArtSuite *sAIMatchingArt = NULL;
	AIArtSuite *sAIArt = NULL;
	AINotifierSuite *sAINotifier = NULL;
	AIMdMemorySuite *sAIMemory = NULL;
	AIDictionarySuite *sAIDictionary = NULL;

	AITransformArtSuite *sAITransformArt = NULL;
	AIRealMathSuite *sAIRealMath = NULL;

	AIToolSuite *sAITool = NULL;
	AIHitTestSuite *sAIHitTest = NULL;
	AIAnnotatorSuite *sAIAnnotator = NULL;
	AIAnnotatorDrawerSuite *sAIAnnotatorDrawer = NULL;
}

const int easyDeltaLUTLen = 32;
const double easyDeltaLUT[32] = { 0.005, 0.005, 0.01, 0.015, 0.02, 0.025, 0.03, 0.03, 0.035, 0.04, 0.04, 0.045, 0.045, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.045, 0.045, 0.045, 0.04, 0.035, 0.035, 0.03, 0.025, 0.02, 0.015, 0.01, 0.005, 0.005 };

const int easyInLUTLen = 15;
const double easyInLUT[15] = { 0.10, 0.20, 0.30, 0.39, 0.48, 0.56, 0.64, 0.72, 0.78, 0.84, 0.89, 0.93, 0.96, 0.99, 1.00 };

PathExploder::PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message) {
	
	ASErr e = kNoErr;
	this->plugin = pluginRef;
	e = sSPBasic->AllocateBlock(sizeof(Globals), (void **)&(this->g));
	if (!e) message->d.globals = this->g;


	this->AcquireSuites(sSPBasic);
	this->CreateTool(sSPBasic);
	this->AddSelectionNotifier(sSPBasic);
	this->AddAnnotator(sSPBasic);
	
	g->selectedArtIsExploded = false;

	this->Alert(sSPBasic, "PathExploder start up!");
}

void PathExploder::AcquireSuites(SPBasicSuite *sSPBasic) {

}

void PathExploder::ReleaseSuites(SPBasicSuite *sSPBasic) {

}

void PathExploder::AddAnnotator(SPBasicSuite *sSPBasic) {
	sSPBasic->AcquireSuite(kAIAnnotatorSuite, kAIAnnotatorVersion, (const void **)&sAIAnnotator);
	sAIAnnotator->AddAnnotator(this->plugin, "PointView Annotator", &(g->annotatorHandle));
	sSPBasic->ReleaseSuite(kAIAnnotatorSuite, kAIAnnotatorVersion);
}

void PathExploder::AddSelectionNotifier(SPBasicSuite * sSPBasic) {
	/* add selection notifier */
	sSPBasic->AcquireSuite(kAINotifierSuite, kAINotifierVersion, (const void**)&sAINotifier);
	sAINotifier->AddNotifier(this->plugin, "Selection notifier", kAIArtSelectionChangedNotifier, &(g->selectionNotifierHandle));
	sSPBasic->ReleaseSuite(kAINotifierSuite, kAINotifierVersion);
}

ASErr PathExploder::CreateTool(SPBasicSuite *sSPBasic) {
	ASErr e = kNoErr;
	
	sSPBasic->AcquireSuite(kAIToolSuite, kAIToolVersion, (const void**)&sAITool);
	sSPBasic->AcquireSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion, (const void**)&sAIUnicodeString);

	AIAddToolData toolData;

	char toolTitle[kMaxStringLength];
	sprintf(toolTitle, kToolTitle);
	toolData.title = ai::UnicodeString(toolTitle);

	char toolTip[kMaxStringLength];
	sprintf(toolTip, kToolTip);
	toolData.tooltip = ai::UnicodeString(toolTip);

	toolData.normalIconResID = kIconID;
	toolData.darkIconResID = kIconID;
	toolData.iconType = ai::IconType::kSVG;

	ai::int32 toolOptions = kToolWantsAlternateSelectionTool | kToolWantsToTrackCursorOption;

	e = sAITool->AddTool(plugin, toolTitle, toolData, toolOptions, &(g->toolHandle));

	sSPBasic->ReleaseSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion);
	sSPBasic->ReleaseSuite(kAIToolSuite, kAIToolVersion);

	return e;
}

ASErr PathExploder::Message(char *caller, char *selector, void *message) {
	ASErr e = kNoErr;

	SPMessageData *msgData = (SPMessageData *)message;
	SPBasicSuite *sSPBasic = msgData->basic;

	if (sSPBasic->IsEqual(caller, kCallerAITool)) {
		
		
		if (sSPBasic->IsEqual(selector, 
			kSelectorAISelectTool)) {

			sSPBasic->AcquireSuite(kAIMatchingArtSuite, kAIMatchingArtVersion, (const void**)&sAIMatchingArt);
			sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void**)&sAIArt);

			this->FreeSelectedArt(sSPBasic);
			sAIMatchingArt->GetSelectedArt( &(g->selectedArt), &(g->selectedArtCount) );

			sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);
			sSPBasic->ReleaseSuite(kAIMatchingArtSuite, kAIMatchingArtVersion);
		}
		
		else if (sSPBasic->IsEqual(selector, 
			kSelectorAIDeselectTool)) {

			if (g->selectedArtIsExploded) {

				sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void**)&sAIArt);

				AIArtHandle art;
				short artType;

				for (int i = 0; i < g->selectedArtCount; i++) {
					art = (*g->selectedArt)[i];
					sAIArt->GetArtType(art, &artType);
					if (artType == kPathArt) {
						AIReal h, v;
						e = this->ReadAndResetTempTransformFromDictionary(sSPBasic, art, &h, &v);
						if (e == kNoErr) this->Move(sSPBasic, art, -h, -v);
					}
				}

				sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);

				g->selectedArtIsExploded = false;
			}

			this->FreeSelectedArt(sSPBasic);

		}

		else if (sSPBasic->IsEqual(selector, 
			kSelectorAIToolMouseDown)) {
			sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void**)&sAIArt);

			AIArtHandle art;
			short artType;

			for (int i = 0; i < g->selectedArtCount; i++) {
				art = (*g->selectedArt)[i];
				sAIArt->GetArtType(art, &artType);
				if (artType == kPathArt) {
					if (!g->selectedArtIsExploded) {
						AIReal h, v;
						h = 5.0 * i;
						v = 2.5 * i;
						this->WriteTempTransformToDictionary(sSPBasic, art, h, v);
						if(e == kNoErr) this->Move(sSPBasic, art, h, v);
					} else {
						AIReal h, v;
						e = this->ReadAndResetTempTransformFromDictionary(sSPBasic, art, &h, &v);
						if(e == kNoErr) this->Move(sSPBasic, art, -h, -v);
					}
				}
			}

			g->selectedArtIsExploded = !g->selectedArtIsExploded; // toggle

			sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);
		}

		else if (sSPBasic->IsEqual(selector, 
			kSelectorAIToolMouseUp)) {
			
		}
	
		else if (sSPBasic->IsEqual(selector, 
			kSelectorAITrackToolCursor)) {
		
		}

	}
	
	return e;
}

AIErr PathExploder::WriteCurrentPositionToDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art) {
	
	AIErr e = kNoErr;

	if (!sAIArt) { 
		e = kCantHappenErr; 
		goto error;
	}

	sSPBasic->AcquireSuite(kAIDictionarySuite, kAIDictionaryVersion, (const void**)&sAIDictionary);

	short type;
	sAIArt->GetArtType(art, &type);

	if (type == kPathArt) {

		AIRealRect artBounds;
		sAIArt->GetArtBounds(art, &artBounds);

		AIRealPoint artCenter;
		artCenter.h = artBounds.left + (artBounds.right - artBounds.left) / 2;
		artCenter.v = artBounds.bottom + (artBounds.top - artBounds.bottom) / 2;

		AIDictionaryRef dictRef;
		sAIArt->GetDictionary(art, &dictRef);

		AIDictKey hPos, vPos;
		hPos = sAIDictionary->Key(kTempHPos);
		vPos = sAIDictionary->Key(kTempVPos);

		sAIDictionary->SetRealEntry(dictRef, hPos, artCenter.h);
		sAIDictionary->SetRealEntry(dictRef, vPos, artCenter.v);

		sAIDictionary->Release(dictRef);
	}
	sSPBasic->ReleaseSuite(kAIDictionarySuite, kAIDictionaryVersion);

error:
	return e;
}

AIErr PathExploder::WriteTempTransformToDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art, AIReal h, AIReal v) {

	AIErr e = kNoErr;

	if (!sAIArt) {
		e = kCantHappenErr;
		goto error;
	}

	sSPBasic->AcquireSuite(kAIDictionarySuite, kAIDictionaryVersion, (const void**)&sAIDictionary);

	AIDictionaryRef dictRef;
	sAIArt->GetDictionary(art, &dictRef);

	AIDictKey hPos, vPos;
	hPos = sAIDictionary->Key(kTempHPos);
	vPos = sAIDictionary->Key(kTempVPos);

	sAIDictionary->SetRealEntry(dictRef, hPos, h);
	sAIDictionary->SetRealEntry(dictRef, vPos, v);

	sAIDictionary->Release(dictRef);

	sSPBasic->ReleaseSuite(kAIDictionarySuite, kAIDictionaryVersion);

error:
	return e;
}


AIErr PathExploder::ReadAndResetTempTransformFromDictionary(SPBasicSuite * sSPBasic, const AIArtHandle &art, AIReal *readHPos, AIReal *readVPos) {
	AIErr e = kNoErr;

	if (!sAIArt) {
		e = kCantHappenErr;
		goto error;
	}

	sSPBasic->AcquireSuite(kAIDictionarySuite, kAIDictionaryVersion, (const void**)&sAIDictionary);

	AIBoolean isKnown;

	// does dictionary exist?
	isKnown = sAIArt->HasDictionary(art);
	if (!isKnown) {
		e = kCantHappenErr;
		goto error;
	}

	AIDictionaryRef dictRef;
	sAIArt->GetDictionary(art, &dictRef);


	AIDictKey hPosKey, vPosKey;
	hPosKey = sAIDictionary->Key(kTempHPos);
	vPosKey = sAIDictionary->Key(kTempVPos);

	// does key entries exist?
	isKnown = sAIDictionary->IsKnown(dictRef, hPosKey);
	if (!isKnown) e = kNoSuchKey;
	isKnown = sAIDictionary->IsKnown(dictRef, hPosKey);
	if (!isKnown) e = kNoSuchKey;
	if (e != kNoErr) goto error;


	// check if position entry exists in dictionary
	sAIDictionary->GetRealEntry(dictRef, hPosKey, readHPos);
	sAIDictionary->GetRealEntry(dictRef, vPosKey, readVPos);

	sAIDictionary->Release(dictRef);

	sSPBasic->ReleaseSuite(kAIDictionarySuite, kAIDictionaryVersion);

error:
	return e;
}

ASErr PathExploder::Alert(SPBasicSuite *sSPBasic, const char *s) {
	ASErr e = kNoErr; // not really used
	
	sSPBasic->AcquireSuite(kAIUserSuite, kAIUserSuiteVersion, (const void**)&sAIUser);
	sSPBasic->AcquireSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion, (const void**)&sAIUnicodeString);
	sSPBasic->AcquireSuite(kSPBlocksSuite, kSPBlocksSuiteVersion, (const void**)&sSPBlocks);

	sAIUser->MessageAlert(ai::UnicodeString(s));

	sSPBasic->ReleaseSuite(kSPBlocksSuite, kSPBlocksSuiteVersion);
	sSPBasic->ReleaseSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion);
	sSPBasic->ReleaseSuite(kAIUserSuite, kAIUserSuiteVersion);

	return e;
}

ASErr PathExploder::Move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal transH, AIReal transV) {
	ASErr e = kNoErr;
	
	sSPBasic->AcquireSuite(kAIRealMathSuite, kAIRealMathVersion, (const void **)&sAIRealMath);
	sSPBasic->AcquireSuite(kAITransformArtSuite, kAITransformArtVersion, (const void **)&sAITransformArt);

	short type;
	sAIArt->GetArtType(art, &type);
	if (type == kPathArt) {
		AIRealMatrix transformMarix;
		sAIRealMath->AIRealMatrixSetTranslate(&transformMarix, transH, transV);
		AIReal lineScale = 1.0;
		AITransformArtOptions transformFlags = kTransformObjects;
		e = sAITransformArt->TransformArt(art, &transformMarix, lineScale, transformFlags);
	}

	sSPBasic->ReleaseSuite(kAIRealMathSuite, kAIRealMathVersion);
	sSPBasic->ReleaseSuite(kAITransformArtSuite, kAITransformArtVersion);

	return e;
}

void PathExploder::FreeSelectedArt(SPBasicSuite * sSPBasic) {
	if (g->selectedArt) {
		sSPBasic->AcquireSuite(kAIMdMemorySuite, kAIMdMemoryVersion, (const void**)&sAIMemory);
		sAIMemory->MdMemoryDisposeHandle((AIMdMemoryHandle)g->selectedArt);
		sSPBasic->ReleaseSuite(kAIMdMemorySuite, kAIMdMemoryVersion);
		g->selectedArt = nullptr;
	}
	g->selectedArtCount = 0;
}

void PathExploder::FreeGlobals(SPInterfaceMessage *message) {
	if (this->g != nullptr) {
		message->d.basic->FreeBlock(this->g);
		this->g = nullptr;
		message->d.globals = nullptr;
	}
}
