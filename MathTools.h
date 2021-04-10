#pragma once

#include "IllustratorSDK.h"

class MathTools {


public:
	MathTools(AIRealMathSuite *m);
	~MathTools();

	AIRealPoint  RadianToVector2(AIReal radian);
	AIRealPoint  RadianToVector2(AIReal radian, AIReal length);
	AIRealPoint  DegreeToVector2(AIReal degree);
	AIRealPoint  DegreeToVector2(AIReal degree, AIReal length);

private:
	AIRealMathSuite *sAIRealMath = nullptr;
	
};
