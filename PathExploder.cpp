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
	AIAnnotatorDrawerSuite *sAIAnnotatorDrawer = NULL;
	AITimerSuite *sAITimer = NULL;

	AIDocumentViewSuite *sAIDocumentView = NULL;
	AILayerSuite *sAILayer = NULL;

	// AIHitTestSuite *sAIHitTest = NULL;
}


PathExploder::PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message) {
	
	ASErr e = kNoErr;
	this->plugin = pluginRef;
	e = sSPBasic->AllocateBlock(sizeof(Globals), (void **)&(this->g));
	if (!e) message->d.globals = this->g;

	this->AcquireSuites(sSPBasic);
	this->CreateTool();
	this->AddSelectionNotifier();
	this->AddAnnotator();

	g->selectedArtIsExploded = false;

	this->mathTools = new MathTools(sAIRealMath);

	//this->Alert("PathExploder start up!");
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
	sSPBasic->AcquireSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion, (const void **)&sAIAnnotatorDrawer);

	sSPBasic->AcquireSuite(kAIDocumentViewSuite, kAIDocumentViewVersion, (const void **)&sAIDocumentView);
	sSPBasic->AcquireSuite(kAILayerSuite, kAILayerVersion, (const void **)&sAILayer);
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
	sSPBasic->ReleaseSuite(kAIAnnotatorDrawerSuite, kAIAnnotatorDrawerVersion);

	sSPBasic->ReleaseSuite(kAIDocumentSuite, kAIDocumentVersion);
	sSPBasic->ReleaseSuite(kAILayerSuite, kAILayerVersion);
}

void PathExploder::AddAnnotator() {
	sAIAnnotator->AddAnnotator(this->plugin, "PointView Annotator", &(g->annotatorHandle));
}

void PathExploder::AddSelectionNotifier() {
	sAINotifier->AddNotifier(this->plugin, "Selection notifier", kAIArtSelectionChangedNotifier, &(g->selectionNotifierHandle));
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

	if (sSPBasic->IsEqual(caller, kCallerAITool)) {

		AIToolMessage *toolMessage;
		toolMessage = (AIToolMessage *)message;


		if (sSPBasic->IsEqual(selector, kSelectorAISelectTool)) {
			this->FreeSelectedArt();

			sAIMatchingArt->GetSelectedArt(&(g->selectedArt), &(g->selectedArtCount));
			g->selectedPathCount = 0;

			AIArtHandle art;
			short artType;

			for (int i = 0; i < g->selectedArtCount; i++) {
				art = (*g->selectedArt)[i];
				sAIArt->GetArtType(art, &artType);
				if (artType == kPathArt) g->selectedPathCount++;
			}

		}

		else if (sSPBasic->IsEqual(selector, kSelectorAIDeselectTool)) {
			this->FreeSelectedArt(); 
		}

		else if (sSPBasic->IsEqual(selector, kSelectorAIToolMouseDown)) {

			mouseDownPos = toolMessage->cursor;
			bool doubleClick = this->IsDoubleClickEvent();

			if (g->selectedPathCount > 1) { // can't explode just 1 path!
				AIArtHandle art;
				short artType;

				AIReal deltaDegree = 360 / g->selectedPathCount; // division by 0 check above!

				AIRealPoint vector;
				AIRealPoint origin;

				for (int i = 0; i < g->selectedArtCount; i++) {
					art = (*g->selectedArt)[i];
					sAIArt->GetArtType(art, &artType);
					if (artType == kPathArt) {
						if (!g->selectedArtIsExploded) {
							vector = mathTools->DegreeToVector2(deltaDegree * i);
							this->WriteTempOriginAndVectorToDict(art, &vector);
						} else {
							e = this->ReadTempOriginFromDictionary(art, &origin);
							if (e == kNoErr) this->MoveTo(art, &origin);
						}
					}
				}

				g->selectedArtIsExploded = !g->selectedArtIsExploded; // toggle outside loop!
			}

		}

		else if (sSPBasic->IsEqual(selector, kSelectorAIToolMouseDrag)) {
			if (g->selectedArtIsExploded && g->selectedPathCount > 1) {

				AIArtHandle art;
				short artType;

				AIRealPoint origin;
				AIRealPoint vector;
				AIRealPoint mouseDragDistance;
				AIReal maxDistance;

				mouseDragDistance.h = mouseDownPos.h - toolMessage->cursor.h;
				mouseDragDistance.v = mouseDownPos.v - toolMessage->cursor.v;
				maxDistance = (mouseDragDistance.h < mouseDragDistance.v) ? mouseDragDistance.v : mouseDragDistance.h;

				for (int i = 0; i < g->selectedArtCount; i++) {
					art = (*g->selectedArt)[i];
					sAIArt->GetArtType(art, &artType);
					if (artType == kPathArt) {
						this->ReadTempOriginFromDictionary(art, &origin);
						this->ReadTempVectorFromDictionary(art, &vector);
						this->MoveToRelativeDistance(art, &origin, &vector, maxDistance);
					}
				}
			}
		} 
		
		else if (sSPBasic->IsEqual(selector, kSelectorAIToolMouseUp)) {
		} 
		
		else if (sSPBasic->IsEqual(selector, kSelectorAITrackToolCursor)) {
		}

	} else if (sSPBasic->IsEqual(caller, kCallerAIAnnotation)) {

		AIAnnotatorMessage *annotatorMessage = (AIAnnotatorMessage *)message;
		AIAnnotatorDrawer *annotatorDrawer = annotatorMessage->drawer;

		if (sSPBasic->IsEqual(selector, kSelectorAIDrawAnnotation)) {
			// draw lines from art objects to origin
			if (g->selectedArtIsExploded && g->selectedPathCount > 1) {
				AIArtHandle art;
				short artType;
				for (int i = 0; i < g->selectedArtCount; i++) {
					art = (*g->selectedArt)[i];
					sAIArt->GetArtType(art, &artType);
					if (artType == kPathArt) {
						AIRealPoint ori, pos;
						this->ReadTempOriginFromDictionary(art, &ori);
						this->GetCurrentPosition(art, &pos);
						AIPoint a, b;

						AILayerHandle layer;
						sAIArt->GetLayerOfArt(art, &layer);

						AIRGBColor col;
						sAILayer->GetLayerColor(layer, &col);

						sAIAnnotatorDrawer->SetColor(annotatorDrawer, col);
						sAIAnnotatorDrawer->SetLineWidth(annotatorDrawer, kAnnotationLineWidth);

						e = sAIDocumentView->ArtworkPointToViewPoint(NULL, &ori, &a);
						e = sAIDocumentView->ArtworkPointToViewPoint(NULL, &pos, &b);

						sAIAnnotatorDrawer->DrawLine(annotatorDrawer, a, b);

						this->DrawIndexLabel(art, annotatorDrawer, &b, i);
					}
				}
			}
		} 
		
		else if (sSPBasic->IsEqual(selector, kSelectorAIInvalAnnotation)) {
			// clear lines
		}

	}
	
	
	return e;
}

const void RealPointToPoint(AIRealPoint *rpnt, AIPoint *pnt) {
	pnt->h = (ai::int32)rpnt->h;
	pnt->v = (ai::int32)rpnt->v;
}

ASErr PathExploder::DrawIndexLabel(const AIArtHandle &art, AIAnnotatorDrawer *drawer, const AIPoint *point, const int index) {
	AIErr error = kNoErr;
	char indexString[16];
	if (index > 1) sprintf(indexString, "%d", index);
	else sprintf(indexString, "1 (top)");

	AIPoint annotatorPoint;
	annotatorPoint.h = point->h + 5;
	annotatorPoint.v = point->v - 5;
	AIRect annotatorRect;
	error = sAIAnnotatorDrawer->GetTextBounds(drawer, ai::UnicodeString(indexString), &annotatorPoint, false, annotatorRect, false);
	annotatorRect.left -= 5;
	annotatorRect.right += 5;
	annotatorRect.top -= 5;
	annotatorRect.bottom += 5;

	// draw filled text frame (using currently set color)
	error = sAIAnnotatorDrawer->DrawRect(drawer, annotatorRect, true);

	// draw black frame border
	unsigned short black = 0;
	AIRGBColor blackColor = { black, black, black };
	sAIAnnotatorDrawer->SetColor(drawer, blackColor);
	sAIAnnotatorDrawer->SetLineWidth(drawer, 0.5);
	error = sAIAnnotatorDrawer->DrawRect(drawer, annotatorRect, false);

	// Draw index text
	error = sAIAnnotatorDrawer->SetFontPreset(drawer, kAIAFSmall);
	error = sAIAnnotatorDrawer->DrawTextAligned(drawer, ai::UnicodeString(indexString), kAICenter, kAIMiddle, annotatorRect, false);

	return kNoErr;
}

ASErr PathExploder::Alert(const char *s) {
	ASErr e = kNoErr;
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

bool PathExploder::IsDoubleClickEvent() {
	bool dc = false;
	DWORD currentTime = GetTickCount();
	if (currentTime - lastClick < (DWORD)GetDoubleClickTime()) dc = true;
	lastClick = currentTime;
	return dc;
}

ASErr PathExploder::Move(const AIArtHandle art, AIReal transH, AIReal transV) {
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

AIErr PathExploder::MoveToRelativeDistance(const AIArtHandle &art, AIRealPoint *origin, AIRealPoint *vector, AIReal distance) {
	AIErr e = kNoErr;

	AIRealPoint absPos;
	absPos.h = origin->h + (distance * vector->h);
	absPos.v = origin->v + (distance * vector->v);

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


AIErr PathExploder::ReadTempVectorFromDictionary(const AIArtHandle &art, AIRealPoint *vector) {
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
	hPosKey = sAIDictionary->Key(kTempHVector);
	vPosKey = sAIDictionary->Key(kTempVVector);

	// does key entries exist?
	isKnown = sAIDictionary->IsKnown(dictRef, hPosKey);
	if (!isKnown) e = kNoSuchKey;
	isKnown = sAIDictionary->IsKnown(dictRef, hPosKey);
	if (!isKnown) e = kNoSuchKey;

	if (e == kNoErr) {
		AIReal h, v;
		sAIDictionary->GetRealEntry(dictRef, hPosKey, &h);
		sAIDictionary->GetRealEntry(dictRef, vPosKey, &v);
		vector->h = h;
		vector->v = v;
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

AIErr PathExploder::WriteTempOriginToDict(const AIArtHandle &art) {
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

		sAIDictionary->Release(dictionaryRef);
	}

error:
	return e;
}

AIErr PathExploder::WriteTempOriginAndVectorToDict(const AIArtHandle &art, AIRealPoint *vector) {
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

		AIDictKey hVector, vVector;
		hVector = sAIDictionary->Key(kTempHVector);
		vVector = sAIDictionary->Key(kTempVVector);

		sAIDictionary->SetRealEntry(dictionaryRef, hVector, vector->h);
		sAIDictionary->SetRealEntry(dictionaryRef, vVector, vector->v);

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
	hPos = sAIDictionary->Key(kTempHVector);
	vPos = sAIDictionary->Key(kTempVVector);

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
