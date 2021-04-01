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

const int easyInOutLUTLen = 32;
const double easyInOutLUT[32] = { 0.005, 0.005, 0.01, 0.015, 0.02, 0.025, 0.03, 0.03, 0.035, 0.04, 0.04, 0.045, 0.045, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.045, 0.045, 0.045, 0.04, 0.035, 0.035, 0.03, 0.025, 0.02, 0.015, 0.01, 0.005, 0.005 };


PathExploder::PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message) {
	
	ASErr e = kNoErr;
	this->plugin = pluginRef;
	e = sSPBasic->AllocateBlock(sizeof(Globals), (void **)&(this->g));
	if (!e) message->d.globals = this->g;

	CreateTool(sSPBasic);
	AddSelectionNotifier(sSPBasic);
	AddAnnotator(sSPBasic);
	
	this->Alert(sSPBasic, "PathExploder start up!");
}


void PathExploder::AddAnnotator(SPBasicSuite * sSPBasic) {
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
		
		// select tool
		if (sSPBasic->IsEqual(selector, kSelectorAISelectTool)) {
			// move selected objects apart

		}

		// deselect tool
		else if (sSPBasic->IsEqual(selector, kSelectorAIDeselectTool)) {
			// move selected objects back to original position
		}

		// tool mouse down
		else if (sSPBasic->IsEqual(selector, kSelectorAIToolMouseDown)) {

			sSPBasic->AcquireSuite(kAIMatchingArtSuite, kAIMatchingArtVersion, (const void**)&sAIMatchingArt);
			sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void**)&sAIArt);


			int numberOfArtObjects = 0;
			
			this->FreeSelectedArt(sSPBasic);
			sAIMatchingArt->GetSelectedArt(&(g->selectedArt), &numberOfArtObjects);
			
			
			char str[kMaxStringLength];
			sprintf(str, "selected objects: %d", numberOfArtObjects);
			//this->Alert(sSPBasic, str);
			if (numberOfArtObjects > 1) {
				AIArtHandle art = (*g->selectedArt)[1];
				this->Move(sSPBasic, art, 25, 25);
			}

			sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);
			sSPBasic->ReleaseSuite(kAIMatchingArtSuite, kAIMatchingArtVersion);

		}

	} else if (sSPBasic->IsEqual(caller, kCallerAINotify)) {
		
		if (sSPBasic->IsEqual(selector, kSelectorAINotify)) {
			// selection changed!

			sSPBasic->AcquireSuite(kAIMatchingArtSuite, kAIMatchingArtVersion, (const void**)&sAIMatchingArt);
			sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void**)&sAIArt);
			sSPBasic->AcquireSuite(kAIMdMemorySuite, kAIMdMemoryVersion, (const void**)&sAIMemory);

			int numberOfArtObjects = 0;
			AIArtHandle **artObjects;
			sAIMatchingArt->GetSelectedArt(&artObjects, &numberOfArtObjects);

			AIReal outx = 1.11;
			AIReal outy = 2.22;

			int artCount = 0;
			for (int i = 0; i < numberOfArtObjects; i++) {

				AIArtHandle art = (*artObjects)[i];

				short type;
				sAIArt->GetArtType(art, &type);

				if (type == kPathArt && artCount == 0) {
					artCount++;

					AIRealRect artBounds;
					e = sAIArt->GetArtBounds(art, &artBounds);

					AIRealPoint artCenter;
					artCenter.h = artBounds.left + (artBounds.right - artBounds.left) / 2;
					artCenter.v = artBounds.bottom + (artBounds.top - artBounds.bottom) / 2;

					sSPBasic->AcquireSuite(kAIDictionarySuite, kAIDictionaryVersion, (const void**)&sAIDictionary);

					AIDictionaryRef dictRef;
					sAIArt->GetDictionary(art, &dictRef);

					AIDictKey hPos, vPos;
					hPos = sAIDictionary->Key("-HPos");
					vPos = sAIDictionary->Key("-VPos");

					//TODO: add current h v coordinates to art objects dictionary.
					sAIDictionary->SetRealEntry(dictRef, hPos, artCenter.h);
					sAIDictionary->SetRealEntry(dictRef, vPos, artCenter.v);

					// now try to read the value:
					sAIDictionary->GetRealEntry(dictRef, hPos, &outx);
					sAIDictionary->GetRealEntry(dictRef, vPos, &outy);

					sAIDictionary->Release(dictRef);

					sSPBasic->ReleaseSuite(kAIDictionarySuite, kAIDictionaryVersion);

					//this->Move(sSPBasic, art, 10.0, 10.0); // x right >   y up ^

				}

			}

			// divide 360 degrees with number of objects, to later move the objects apart..

			char str[kMaxStringLength];
			sprintf(str, "x %.2f y %.2f", outx, outy);

			//this->Alert(sSPBasic, str);

			sAIMemory->MdMemoryDisposeHandle((AIMdMemoryHandle)artObjects);

			sSPBasic->ReleaseSuite(kAIMdMemorySuite, kAIMdMemoryVersion);
			sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);
			sSPBasic->ReleaseSuite(kAIMatchingArtSuite, kAIMatchingArtVersion);
		}
	}
	
	return e;
}

void PathExploder::FreeSelectedArt(SPBasicSuite * sSPBasic) {
	if (g->selectedArt) {
		sSPBasic->AcquireSuite(kAIMdMemorySuite, kAIMdMemoryVersion, (const void**)&sAIMemory);
		sAIMemory->MdMemoryDisposeHandle((AIMdMemoryHandle)g->selectedArt);
		sSPBasic->ReleaseSuite(kAIMdMemorySuite, kAIMdMemoryVersion);
		g->selectedArt = nullptr;
	}
}

void PathExploder::FreeGlobals(SPInterfaceMessage *message) {	
	if (this->g != nullptr) {
		message->d.basic->FreeBlock(this->g);
		this->g = nullptr;
		message->d.globals = nullptr;
	}
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

ASErr PathExploder::Move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal tx, AIReal ty) {
	ASErr e = kNoErr;
	
	sSPBasic->AcquireSuite(kAIRealMathSuite, kAIRealMathVersion, (const void **)&sAIRealMath);
	sSPBasic->AcquireSuite(kAITransformArtSuite, kAITransformArtVersion, (const void **)&sAITransformArt);

	short type;
	sAIArt->GetArtType(art, &type);
	if (type == kPathArt) {
		AIRealMatrix transformMarix;
		sAIRealMath->AIRealMatrixSetTranslate(&transformMarix, tx, ty);
		AIReal lineScale = 1.0;
		AITransformArtOptions transformFlags = kTransformObjects;
		e = sAITransformArt->TransformArt(art, &transformMarix, lineScale, transformFlags);
	}

	sSPBasic->ReleaseSuite(kAIRealMathSuite, kAIRealMathVersion);
	sSPBasic->ReleaseSuite(kAITransformArtSuite, kAITransformArtVersion);

	return e;
}