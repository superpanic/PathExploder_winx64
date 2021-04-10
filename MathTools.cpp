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


/*
AIAPI AIReal(*AIRealSin)(AIReal a)
Computes the sine of a real number in radians.
AIAPI AIReal(*AIRealCos)(AIReal a)
Computes the cosine of a real number in radians.
AIAPI AIReal(*AIRealATan)(AIReal a, AIReal b)
Computes the arctangent of an angle in radians, where a and b are two real numbers such that a / b is the tangent of the angle.
AIAPI AIReal(*DegreeToRadian)(AIReal degree)
Converts an angle expressed in degrees to radians.
AIAPI AIReal(*RadianToDegree)(AIReal radian)
Converts an angle expressed in radians to degrees.

 public static class MathHelpers
 {
     public static Vector2 RadianToVector2(float radian) {
	 return new Vector2(Mathf.Cos(radian), Mathf.Sin(radian));
     }

     public static Vector2 RadianToVector2(float radian, float length) {
	 return RadianToVector2(radian) * length;
     }

     public static Vector2 DegreeToVector2(float degree) {
	 return RadianToVector2(degree * Mathf.Deg2Rad);
     }

     public static Vector2 DegreeToVector2(float degree, float length) {
	 return RadianToVector2(degree * Mathf.Deg2Rad) * length;
     }

 }

*/

