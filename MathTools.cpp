#include "MathTools.h"

MathTools::MathTools(AIRealMathSuite *realMathSuite) {
	sAIRealMath = realMathSuite;
}

MathTools::~MathTools() {
	sAIRealMath = nullptr;
}

AIRealPoint MathTools::RadianToVector2(AIReal radian) {
	return { sAIRealMath->AIRealCos(radian), sAIRealMath->AIRealSin(radian) };
}

AIRealPoint MathTools::RadianToVector2(AIReal radian, AIReal length) {
	AIRealPoint vector2 = this->RadianToVector2(radian);
	return { vector2.h * length, vector2.v * length };
}

AIRealPoint MathTools::DegreeToVector2(AIReal degree) {
	AIRealPoint vector2 = this->RadianToVector2(sAIRealMath->DegreeToRadian(degree));
	return vector2;
}

AIRealPoint MathTools::DegreeToVector2(AIReal degree, AIReal length) {
	AIRealPoint vector2 = this->DegreeToVector2(degree);
	return { vector2.h*length, vector2.v * length };
}

