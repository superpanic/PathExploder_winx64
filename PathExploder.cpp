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
}

const int easyInOutLUTLen = 32;
const double easyInOutLUT[32] = { 0.005, 0.005, 0.01, 0.015, 0.02, 0.025, 0.03, 0.03, 0.035, 0.04, 0.04, 0.045, 0.045, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.045, 0.045, 0.045, 0.04, 0.035, 0.035, 0.03, 0.025, 0.02, 0.015, 0.01, 0.005, 0.005 };

PathExploder::PathExploder(SPPluginRef pluginRef, SPBasicSuite *sSPBasic, SPInterfaceMessage *message) {
	ASErr e = kNoErr;
	this->plugin = pluginRef;
	// globals
	e = sSPBasic->AllocateBlock(sizeof(Globals), (void **)&(this->g));
	if (!e) message->d.globals = this->g;

	sSPBasic->AcquireSuite(kAINotifierSuite, kAINotifierVersion, (const void**)&sAINotifier);
	sAINotifier->AddNotifier(this->plugin, "Selection notifier", kAIArtSelectionChangedNotifier, &(g->selectionNotifierHandle));
	sSPBasic->ReleaseSuite(kAINotifierSuite, kAINotifierVersion);

	this->Alert(sSPBasic, "PathExploder start up!");

};


ASErr PathExploder::Message(char *caller, char *selector, void *message) {
	ASErr e = kNoErr;

	SPMessageData *msgData = (SPMessageData *)message;
	SPBasicSuite *sSPBasic = msgData->basic;

	if (sSPBasic->IsEqual(caller, kCallerAINotify)) {
		if (sSPBasic->IsEqual(selector, kSelectorAINotify)) {
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
//				if (type == kPathArt) artCount++;

				if (type == kPathArt && artCount == 0) {
					artCount++;

					//TODO: read and save original position into selected art objects dictionary	
					// here are some fake hard coded coordinates for testing purposes

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


					//this->move(sSPBasic, art, 10.0, 10.0); // x right >   y up ^

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

ASErr PathExploder::move(SPBasicSuite *sSPBasic, AIArtHandle art, AIReal tx, AIReal ty) {
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

ASErr PathExploder::transformPathArtWithParameters(AIArtHandle art, AIReal tx, AIReal ty, AIReal theta, AIReal sx, AIReal sy) {
	ASErr result = kNoErr;

	if (!sAIRealMath) return -1;
	if (!sAITransformArt) return -1;

	AIRealRect artBounds;
	AIRealPoint artCenter;
	AIRealMatrix artMatrix;
	AIReal lineScale;
	ai::int32 transformFlags = kTransformObjects | kScaleLines;
	short type;

	lineScale = (sAIRealMath->AIRealSqrt(sx)) * (sAIRealMath->AIRealSqrt(sy));

	sAIArt->GetArtType(art, &type);

	if(type == kPathArt) {
		//result = sAIDocument->GetDocumentMaxArtboardBounds( &artboardBounds );
		result = sAIArt->GetArtBounds(art, &artBounds);
		artCenter.h = artBounds.left + (artBounds.right - artBounds.left) / 2;
		artCenter.v = artBounds.bottom + (artBounds.top - artBounds.bottom) / 2;

		// move object so that the centerpoint is at the origin
		sAIRealMath->AIRealMatrixSetTranslate(&artMatrix, -artCenter.h, -artCenter.v);
		// translate object by tx and ty
		sAIRealMath->AIRealMatrixConcatTranslate(&artMatrix, tx, ty);
		// rotate object by theta
		sAIRealMath->AIRealMatrixConcatRotate(&artMatrix, theta);
		// scale object by sx and sy
		sAIRealMath->AIRealMatrixConcatScale(&artMatrix, sx, sy);
		// move the object back to original position
		sAIRealMath->AIRealMatrixConcatTranslate(&artMatrix, artCenter.h, artCenter.v);

		result = sAITransformArt->TransformArt(art, &artMatrix, lineScale, transformFlags);
	}

	return result;
}
