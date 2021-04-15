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
	AIAnnotatorSuite *sAIAnnotator = NULL;

	AITimerSuite *sAITimer = NULL;

	// AIAnnotatorDrawerSuite *sAIAnnotatorDrawer = NULL;
	// AIHitTestSuite *sAIHitTest = NULL;
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
	this->CreateTool();
	this->AddSelectionNotifier();
	this->AddAnnotator();
	this->SetupTimer();
	
	g->selectedArtIsExploded = false;

	this->mathTools = new MathTools(sAIRealMath);

	this->Alert("PathExploder start up!");
}

PathExploder::~PathExploder() {
	delete this->mathTools;
}

void PathExploder::AcquireSuites(SPBasicSuite *sSPBasic) {
	sSPBasic->AcquireSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion, (const void**)&sAIUnicodeString);
	sSPBasic->AcquireSuite(kSPBlocksSuite, kSPBlocksSuiteVersion, (const void**)&sSPBlocks);
	sSPBasic->AcquireSuite(kAIUserSuite, kAIUserSuiteVersion, (const void**)&sAIUser);
	sSPBasic->AcquireSuite(kAIMatchingArtSuite, kAIMatchingArtVersion, (const void**)&sAIMatchingArt);
	sSPBasic->AcquireSuite(kAIArtSuite, kAIArtVersion, (const void**)&sAIArt);
	sSPBasic->AcquireSuite(kAINotifierSuite, kAINotifierVersion, (const void**)&sAINotifier);
	sSPBasic->AcquireSuite(kAIMdMemorySuite, kAIMdMemoryVersion, (const void**)&sAIMemory);
	sSPBasic->AcquireSuite(kAIDictionarySuite, kAIDictionaryVersion, (const void**)&sAIDictionary);
	sSPBasic->AcquireSuite(kAITransformArtSuite, kAITransformArtVersion, (const void **)&sAITransformArt);
	sSPBasic->AcquireSuite(kAIRealMathSuite, kAIRealMathVersion, (const void **)&sAIRealMath);
	sSPBasic->AcquireSuite(kAIToolSuite, kAIToolVersion, (const void**)&sAITool);
	sSPBasic->AcquireSuite(kAIAnnotatorSuite, kAIAnnotatorVersion, (const void **)&sAIAnnotator);
	sSPBasic->AcquireSuite(kAITimerSuite, kAITimerVersion, (const void **)&sAITimer);
}

void PathExploder::ReleaseSuites(SPBasicSuite *sSPBasic) {
	sSPBasic->ReleaseSuite(kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion);
	sSPBasic->ReleaseSuite(kSPBlocksSuite, kSPBlocksSuiteVersion);
	sSPBasic->ReleaseSuite(kAIUserSuite, kAIUserSuiteVersion);
	sSPBasic->ReleaseSuite(kAIMatchingArtSuite, kAIMatchingArtVersion);
	sSPBasic->ReleaseSuite(kAIArtSuite, kAIArtVersion);
	sSPBasic->ReleaseSuite(kAINotifierSuite, kAINotifierVersion);
	sSPBasic->ReleaseSuite(kAIMdMemorySuite, kAIMdMemoryVersion);
	sSPBasic->ReleaseSuite(kAIDictionarySuite, kAIDictionaryVersion);
	sSPBasic->ReleaseSuite(kAITransformArtSuite, kAITransformArtVersion);
	sSPBasic->ReleaseSuite(kAIRealMathSuite, kAIRealMathVersion);
	sSPBasic->ReleaseSuite(kAIToolSuite, kAIToolVersion);
	sSPBasic->ReleaseSuite(kAIAnnotatorSuite, kAIAnnotatorVersion);
	sSPBasic->ReleaseSuite(kAITimerSuite, kAITimerSuiteVersion);
}

void PathExploder::AddAnnotator() {
	sAIAnnotator->AddAnnotator(this->plugin, "PointView Annotator", &(g->annotatorHandle));
}

void PathExploder::AddSelectionNotifier() {
	/* add selection notifier */
	sAINotifier->AddNotifier(this->plugin, "Selection notifier", kAIArtSelectionChangedNotifier, &(g->selectionNotifierHandle));
}

void PathExploder::SetupTimer() {
	sAITimer->AddTimer(this->plugin, "Ease Timer", kAnimationSpeed, &(g->timerHandle));
	sAITimer->SetTimerActive(g->timerHandle, false);
}

ASErr PathExploder::CreateTool() {
	ASErr e = kNoErr;
	
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

	return e;
}

ASErr PathExploder::Message(char *caller, char *selector, void *message) {
	ASErr e = kNoErr;

	SPMessageData *msgData = (SPMessageData *)message;
	SPBasicSuite *sSPBasic = msgData->basic;

	if (sSPBasic->IsEqual(caller, kCallerAITimer)) {

		if (sSPBasic->IsEqual(selector, kSelectorAIGoTimer)) {
			if (g->selectedArtIsExploded && g->selectedPathCount > 0) {
				
				if (g->animationStep < easyInLUTLen) {
					AIReal frac = easyInLUT[g->animationStep];

					AIArtHandle art;
					short artType;

					AIRealPoint origin;
					AIRealPoint transform;

					for (int i = 0; i < g->selectedArtCount; i++) {
						art = (*g->selectedArt)[i];
						sAIArt->GetArtType(art, &artType);
						if (artType == kPathArt) {
							this->ReadTempOriginFromDictionary(art, &origin);
							this->ReadTempTransformFromDict(art, &transform);
							this->MoveToFraction(art, &origin, &transform, frac);
						}
					}

					g->animationStep++;
				} else {
					this->DeactivateTimer();
					g->animationStep = 0;
				}

			}

		}

	}

	if (sSPBasic->IsEqual(caller, kCallerAITool)) {
		
		if (sSPBasic->IsEqual(selector, kSelectorAISelectTool)) {
			this->FreeSelectedArt();

			sAIMatchingArt->GetSelectedArt( &(g->selectedArt), &(g->selectedArtCount) );
			g->selectedPathCount = 0;

			AIArtHandle art;
			short artType;

			for (int i = 0; i < g->selectedArtCount; i++) {
				art = (*g->selectedArt)[i];
				sAIArt->GetArtType(art, &artType);
				if(artType == kPathArt) g->selectedPathCount++;
			}

		}
		
		else if (sSPBasic->IsEqual(selector, kSelectorAIDeselectTool)) {
			if (g->selectedPathCount > 0) {
				if (g->selectedArtIsExploded) {

					AIArtHandle art;
					short artType;

					for (int i = 0; i < g->selectedArtCount; i++) {
						art = (*g->selectedArt)[i];
						sAIArt->GetArtType(art, &artType);
						if (artType == kPathArt) {
							AIRealPoint origin;
							e = this->ReadTempOriginFromDictionary(art, &origin);
							if (e == kNoErr) this->MoveTo(art, &origin);
							sAITimer->SetTimerActive(g->timerHandle, false);
						}
					}

					g->selectedArtIsExploded = false;
				}
			}
			this->FreeSelectedArt(); // art objects might still be in selection list, even though it's not path objects
		}

		else if (sSPBasic->IsEqual(selector, kSelectorAIToolMouseDown)) {
			if (g->selectedPathCount > 0) {
				AIArtHandle art;
				short artType;

				AIReal deltaDegree = 360 / g->selectedPathCount; // division by 0 check above!
				AIRealPoint transform;

				for (int i = 0; i < g->selectedArtCount; i++) {
					art = (*g->selectedArt)[i];
					sAIArt->GetArtType(art, &artType);
					if (artType == kPathArt) {
						if (!g->selectedArtIsExploded) {
							transform = mathTools->DegreeToVector2(deltaDegree * i, kExplosionLength);
							this->WriteTempOriginAndTransformToDict(art, &transform);
							g->animationStep = 0;
							sAITimer->SetTimerActive(g->timerHandle, true);
						} else {
							AIRealPoint origin;
							e = this->ReadTempOriginFromDictionary(art, &origin);
							if (e == kNoErr) this->MoveTo(art, &origin);
							sAITimer->SetTimerActive(g->timerHandle, false);
						}
					}
				}

				g->selectedArtIsExploded = !g->selectedArtIsExploded; // toggle
			}
		}

		else if (sSPBasic->IsEqual(selector, 
			kSelectorAIToolMouseUp)) { }
	
		else if (sSPBasic->IsEqual(selector, 
			kSelectorAITrackToolCursor)) { }
	}
	
	return e;
}

ASErr PathExploder::Alert(const char *s) {
	ASErr e = kNoErr; // not really used
	if (sAIUser) sAIUser->MessageAlert(ai::UnicodeString(s));
	else e = kCantHappenErr;
	return e;
}

AIErr PathExploder::GetCurrentPosition(const AIArtHandle &art, AIRealPoint *currentPos) {
	AIErr e = kNoErr;

	short type;
	sAIArt->GetArtType(art, &type);

	if (type == kPathArt) {
		AIRealRect artBounds;
		sAIArt->GetArtBounds(art, &artBounds);

		currentPos->h = artBounds.left + (artBounds.right - artBounds.left) / 2;
		currentPos->v = artBounds.bottom + (artBounds.top - artBounds.bottom) / 2;
	} else {
		e = kCantHappenErr;
	}

	return e;
}

ASErr PathExploder::Move(AIArtHandle art, AIReal transH, AIReal transV) {
	ASErr e = kNoErr;
	
	short type;
	sAIArt->GetArtType(art, &type);
	if (type == kPathArt) {
		AIRealMatrix transformMarix;
		sAIRealMath->AIRealMatrixSetTranslate(&transformMarix, transH, transV);
		AIReal lineScale = 1.0;
		AITransformArtOptions transformFlags = kTransformObjects;
		e = sAITransformArt->TransformArt(art, &transformMarix, lineScale, transformFlags);
	}

	return e;
}

AIErr PathExploder::MoveToFraction(const AIArtHandle &art, AIRealPoint *origin, AIRealPoint *transform, AIReal fraction) {
	AIErr e = kNoErr;

	AIRealPoint absPos;
	absPos.h = origin->h + (transform->h * fraction);
	absPos.v = origin->v + (transform->v * fraction);

	this->MoveTo(art, &absPos);

	return e;
}

AIErr PathExploder::MoveTo(const AIArtHandle &art, AIRealPoint *absPos) {
	AIErr e = kNoErr;
	AIRealPoint currentPos;
	e = this->GetCurrentPosition(art, &currentPos);
	AIRealPoint transform;
	transform.h = absPos->h - currentPos.h;
	transform.v = absPos->v - currentPos.v;
	e = this->Move(art, transform.h, transform.v);
	return e;
}


AIErr PathExploder::ReadTempTransformFromDict(const AIArtHandle &art, AIRealPoint *transform) {
	AIErr e = kNoErr;

	if (!sAIArt) {
		e = kCantHappenErr;
		goto error;
	}

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
	hPosKey = sAIDictionary->Key(kTempHTransform);
	vPosKey = sAIDictionary->Key(kTempVTransform);

	// does key entries exist?
	isKnown = sAIDictionary->IsKnown(dictRef, hPosKey);
	if (!isKnown) e = kNoSuchKey;
	isKnown = sAIDictionary->IsKnown(dictRef, hPosKey);
	if (!isKnown) e = kNoSuchKey;

	if (e == kNoErr) {
		AIReal h, v;
		sAIDictionary->GetRealEntry(dictRef, hPosKey, &h);
		sAIDictionary->GetRealEntry(dictRef, vPosKey, &v);
		transform->h = h;
		transform->v = v;
	}

	sAIDictionary->Release(dictRef);

error:
	return e;
}

AIErr PathExploder::ReadTempOriginFromDictionary(const AIArtHandle &art, AIRealPoint *origin) {
	AIErr e = kNoErr;

	if (!sAIArt) {
		e = kCantHappenErr;
		goto error;
	}

	AIBoolean isKnown;

	// does dictionary exist?
	isKnown = sAIArt->HasDictionary(art);
	if (!isKnown) {
		e = kCantHappenErr;
		goto error;
	}

	AIDictionaryRef dictRef;
	sAIArt->GetDictionary(art, &dictRef);

	AIDictKey hKey, vKey;
	hKey = sAIDictionary->Key(kTempOriginH);
	vKey = sAIDictionary->Key(kTempOriginV);

	// does key entries exist?
	isKnown = sAIDictionary->IsKnown(dictRef, hKey);
	if (!isKnown) e = kNoSuchKey;
	isKnown = sAIDictionary->IsKnown(dictRef, vKey);
	if (!isKnown) e = kNoSuchKey;

	if (e == kNoErr) {
		AIReal h, v;
		sAIDictionary->GetRealEntry(dictRef, hKey, &h);
		sAIDictionary->GetRealEntry(dictRef, vKey, &v);
		origin->h = h;
		origin->v = v;
	}

	sAIDictionary->Release(dictRef);

error:
	return e;


}

AIErr PathExploder::WriteTempOriginAndTransformToDict(const AIArtHandle &art, AIRealPoint *transform) {
	AIErr e = kNoErr;

	short type;
	sAIArt->GetArtType(art, &type);

	if (type == kPathArt) {

		AIRealRect artBounds;
		e = sAIArt->GetArtBounds(art, &artBounds);
		if (e != kNoErr) goto error;

		AIRealPoint artCenter;
		artCenter.h = artBounds.left + (artBounds.right - artBounds.left) / 2;
		artCenter.v = artBounds.bottom + (artBounds.top - artBounds.bottom) / 2;

		AIDictionaryRef dictionaryRef = nullptr;
		e = sAIArt->GetDictionary(art, &dictionaryRef);
		if (e != kNoErr && dictionaryRef == nullptr) goto error;

		AIDictKey hOrigin, vOrigin;
		hOrigin = sAIDictionary->Key(kTempOriginH);
		vOrigin = sAIDictionary->Key(kTempOriginV);

		sAIDictionary->SetRealEntry(dictionaryRef, hOrigin, artCenter.h);
		sAIDictionary->SetRealEntry(dictionaryRef, vOrigin, artCenter.v);

		AIDictKey hTransform, vTransform;
		hTransform = sAIDictionary->Key(kTempHTransform);
		vTransform = sAIDictionary->Key(kTempVTransform);

		sAIDictionary->SetRealEntry(dictionaryRef, hTransform, transform->h);
		sAIDictionary->SetRealEntry(dictionaryRef, vTransform, transform->v);

		sAIDictionary->Release(dictionaryRef);

	}

error:
	return e;
}

AIErr PathExploder::WriteTempTransformToDict(const AIArtHandle &art, AIReal h, AIReal v) {

	AIErr e = kNoErr;

	if (!sAIArt) {
		e = kCantHappenErr;
		goto error;
	}

	AIDictionaryRef dictRef;
	sAIArt->GetDictionary(art, &dictRef);

	AIDictKey hPos, vPos;
	hPos = sAIDictionary->Key(kTempHTransform);
	vPos = sAIDictionary->Key(kTempVTransform);

	sAIDictionary->SetRealEntry(dictRef, hPos, h);
	sAIDictionary->SetRealEntry(dictRef, vPos, v);

	sAIDictionary->Release(dictRef);

error:
	return e;
}

void PathExploder::DeactivateTimer() {
	if (sAITimer) {
		if (g->timerHandle) sAITimer->SetTimerActive(g->timerHandle, false);
	}
}

void PathExploder::FreeSelectedArt() {
	if (g->selectedArt) {
		sAIMemory->MdMemoryDisposeHandle((AIMdMemoryHandle)g->selectedArt);
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

