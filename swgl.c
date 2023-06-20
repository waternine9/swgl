#include "swgl.h"

#include <memory.h> // Comment this line out for freestanding, you'll have to include your header files though that should allow malloc, memcpy, memset, and free.

/*
* HELPER CONSTANTS
*/

#define SWGL_BIGNUM 0x7FFFFFFF

/*
* HELPER FUNCS AND STRUCTS
*/

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

double atof(const char* str) {
    double result = 0.0;
    double factor = 1.0;
    uint8_t decimalPointSeen = 0;
	double sign = 1.0;

    // Handle negative numbers
    if (*str == '-') 
	{
		sign = -1.0;
        str++;
    }

    for (; *str; str++) 
	{
        if (*str == '.') 
		{
            decimalPointSeen = 1; 
            continue;
        }
        
        int digit = *str - '0';
        if (digit >= 0 && digit <= 9) 
		{
            if (decimalPointSeen) 
			{
                factor /= 10.0;
                result += factor * digit;
            } else 
			{
                result = result * 10.0 + digit;
            }
        } 
		else 
		{
            break;  // Invalid character, stop parsing
        }
    }
    
    return result * sign;
}

int atoi(const char* str) 
{
    int result = 0;
    int sign = 1;

    // Skip leading white space, if any
    while (*str == ' ') 
	{
        ++str;
    }

    // Check for sign
    if (*str == '-' || *str == '+') 
	{
        sign = (*str == '-') ? -1 : 1;
        ++str;
    }

    // Process characters of string
    while (*str >= '0' && *str <= '9') {

        result = result * 10 + (*str - '0');
        ++str;
    }

    // Return the resultant integer
    return sign * result;
}

typedef struct
{
	void* Data;
	int Cap;
	int Size;
	size_t ElemSize;
} _Vector;

_Vector* NewVector(size_t _ElemSize)
{
	_Vector* _Vec = (_Vector*)malloc(sizeof(_Vector));
	_Vec->ElemSize = _ElemSize;
	_Vec->Cap = 128;
	_Vec->Size = 0;
	_Vec->Data = malloc(_Vec->ElemSize * _Vec->Cap);
	return _Vec;
}

void _VectorVerify(_Vector* _Vec)
{
	if (_Vec->Size >= _Vec->Cap)
	{
		_Vec->Cap += 128;
		void* NewData = malloc(_Vec->ElemSize * _Vec->Cap);
		memcpy(NewData, _Vec->Data, _Vec->ElemSize * _Vec->Size);
		_Vec->Data = NewData;
	}
}

void VectorPushBack(_Vector* _Vec, void* _Data)
{
	memcpy((uint8_t*)_Vec->Data + _Vec->ElemSize * _Vec->Size, _Data, _Vec->ElemSize);
	_Vec->Size++;
	_VectorVerify(_Vec);
}

void VectorPopBack(_Vector* _Vec)
{
	_Vec->Size--;
	if (_Vec->Size < 0)
	{
		_Vec->Size = 0;
	}
}

void VectorRead(_Vector* _Vec, void* _Out, int _Index)
{
	memcpy(_Out, (uint8_t*)_Vec->Data + _Vec->ElemSize * _Index, _Vec->ElemSize);
}

void VectorFree(_Vector* _Vec)
{
	free(_Vec->Data);
}

void VectorCopy(_Vector* _Dest, _Vector* _Src)
{
	VectorFree(_Dest);
	_Dest->Cap = _Src->Cap;
	_Dest->Size = _Src->Size;
	_Dest->ElemSize = _Src->ElemSize;
	_Dest->Data = malloc(_Dest->Cap * _Dest->ElemSize);
	memcpy(_Dest->Data, _Src->Data, _Src->Size * _Src->ElemSize);
}

typedef struct
{
	void* First;
	size_t FirstSize;
	void* Second;
	size_t SecondSize;
} _Pair;

_Pair* MakePair(void* _FirstData, size_t _FirstSize, void* _SecondData, size_t _SecondSize)
{
	_Pair* _MyPair = (_Pair*)malloc(sizeof(_Pair));
	_MyPair->First = malloc(_FirstSize);
	memcpy(_MyPair->First, _FirstData, _FirstSize);
	_MyPair->Second = malloc(_SecondSize);
	memcpy(_MyPair->Second, _SecondData, _SecondSize);

	_MyPair->FirstSize = _FirstSize;
	_MyPair->SecondSize = _SecondSize;
}

void PairSetFirst(_Pair* Pair, void* _Data)
{
	memcpy(Pair->First, _Data, Pair->FirstSize);
}

void PairSetSecond(_Pair* Pair, void* _Data)
{
	memcpy(Pair->Second, _Data, Pair->SecondSize);
}

void PairGetFirst(_Pair* Pair, void* _Out)
{
	memcpy(_Out, Pair->First, Pair->FirstSize);
}

void PairGetSecond(_Pair* Pair, void* _Out)
{
	memcpy(_Out, Pair->Second, Pair->SecondSize);
}

typedef struct
{
	char* Data;
	int Cap;
	int Size;
} _String;

_String* NewString()
{
	_String* String = (_String*)malloc(sizeof(_String));
	String->Cap = 128;
	String->Size = 0;
	String->Data = (char*)malloc(String->Cap);
	memset(String->Data, 0, String->Cap);
	return String;
}

char StringGet(_String* _Str, int i)
{
	return _Str->Data[i];
}

void StringPush(_String* _Str, char c)
{
	_Str->Data[_Str->Size++] = c;
	if (_Str->Size >= _Str->Cap - 2)
	{
		_Str->Cap += 128;
		char* NewData = (char*)malloc(_Str->Cap);
		memset(NewData, 0, _Str->Cap);
		memcpy(NewData, _Str->Data, _Str->Size);
		_Str->Data = NewData;
	}
}

void StringAppend(_String* _Str, _String* _ToAppend)
{
	for (int i = 0; i < _ToAppend->Size; i++)
	{
		StringPush(_Str, _ToAppend->Data[i]);
	}
}

char* String2CString(_String* _Str)
{
	return _Str->Data;
}

_String* CString2String(const char* _Str)
{
	_String* String = NewString();
	while (*_Str)
	{
		StringPush(String, *_Str);
		_Str++;
	}
	return String;
}

uint8_t StringEquals(_String* _A, const char* _B)
{
	for (int i = 0; i < _A->Size; i++)
	{
		if (!_B[i]) return 0;
		if (_A->Data[i] != _B[i]) return 0;
	}
	if (_B[_A->Size]) return 0;
	return 1;
}

void StringCopy(_String* _Dst, _String* _Src)
{
	free(_Dst->Data);
	_Dst->Cap = _Src->Cap;
	_Dst->Size = _Src->Size;
	_Dst->Data = (char*)malloc(_Dst->Cap);
	memcpy(_Dst->Data, _Src->Data, _Src->Cap);
}

/*
* /HELPER FUNCS AND STRUCTS
*/

typedef enum 
{
	GLSL_VEC3,
	GLSL_VEC2,
	GLSL_VEC4,
	GLSL_FLOAT,
	GLSL_INT,
	GLSL_MAT4,
	GLSL_MAT3,
	GLSL_MAT2,
	GLSL_SAMPLER2D,
	GLSL_UNKNOWN
} glslType;

typedef enum 
{
	GLSL_TOK_ADD,
	GLSL_TOK_SUB,
	GLSL_TOK_MUL,
	GLSL_TOK_DIV,
	GLSL_TOK_LT,
	GLSL_TOK_GT,
	GLSL_TOK_EQ,
	GLSL_TOK_ASSIGN,
	GLSL_TOK_VAR,
	GLSL_TOK_VAR_DECL,
	GLSL_TOK_CONST,
	GLSL_TOK_SWIZZLE,
	
	GLSL_TOK_FLOAT_CONSTRUCT,
	GLSL_TOK_VEC2_CONSTRUCT,
	GLSL_TOK_VEC3_CONSTRUCT,
	GLSL_TOK_VEC4_CONSTRUCT,
	GLSL_TOK_INT_CONSTRUCT
} glslTokenType;

typedef struct 
{
	float m00;
	float m01;
	float m02;
	float m03;

	float m10;
	float m11;
	float m12;
	float m13;

	float m20;
	float m21;
	float m22;
	float m23;

	float m30;
	float m31;
	float m32;
	float m33;
} glslMat4;

typedef struct 
{
	float m00;
	float m01;
	float m02;

	float m10;
	float m11;
	float m12;

	float m20;
	float m21;
	float m22;
} glslMat3;

typedef struct 
{
	float m00;
	float m01;

	float m10;
	float m11;
} glslMat2;

typedef struct 
{
	float x;
	float y;
	float z;
	float w;
} glslVec4;

typedef struct 
{
	float x;
	float y;
	float z;
} glslVec3;

typedef struct 
{
	float x;
	float y;
} glslVec2;

typedef struct 
{
	glslType Type;

	float x;
	float y;
	float z;
	float w;

	int i;

	glslMat2 Mat2;
	glslMat3 Mat3;
	glslMat4 Mat4;
} glslExValue;

typedef struct
{
	uint8_t IsFloat;

	float Fval;
	int Ival;
} glslConst;

typedef struct
{
	int Location;
} glslLayout;

typedef struct
{
	void* Data;
	uint8_t Alloc;
} glslValue;

typedef struct _glslVariable
{
	_String* Name;
	glslType Type;

	uint8_t isUniform;
	uint8_t isLayout;

	uint8_t isIn;
	uint8_t isOut;

	glslLayout* Layout;

	glslValue Value;
} glslVariable;

uint8_t GLSLIsDigit(char c)
{
	return (c >= '0' && c <= '9') || c == '-';
}

typedef struct
{
	glslVec4 Verts[3];
	_Vector* TriangleVertexData[3];
} Triangle;

glslVec4 IntersectNearPlane(glslVec4 a, glslVec4 b, float* t)
{
	*t = (a.z + a.w) / (a.w - b.w + a.z - b.z);

	glslVec4 result;
	result.x = a.x + *t * (b.x - a.x);
	result.y = a.y + *t * (b.y - a.y);
	result.z = a.z + *t * (b.z - a.z);
	result.w = a.w + *t * (b.w - a.w);

	return result;
}

glslExValue InterpolateExValue(glslExValue Val0, glslExValue Val1, float t)
{
	glslExValue Out;
	if (Val0.Type == GLSL_FLOAT)
	{
		glslExValue Val = { GLSL_FLOAT, Val0.x + t * (Val1.x - Val0.x) };
		Out = Val;
	}
	else if (Val0.Type == GLSL_VEC2)
	{
		glslExValue Val = { GLSL_VEC2, Val0.x + t * (Val1.x - Val0.x), Val0.y + t * (Val1.y - Val0.y) };
		Out = Val;
	}
	else if (Val0.Type == GLSL_VEC3)
	{
		glslExValue Val = { GLSL_VEC3, Val0.x + t * (Val1.x - Val0.x), Val0.y + t * (Val1.y - Val0.y), Val0.z + t * (Val1.z - Val0.z) };
		Out = Val;
	}
	else if (Val0.Type == GLSL_VEC4)
	{
		glslExValue Val = { GLSL_VEC4, Val0.x + t * (Val1.x - Val0.x), Val0.y + t * (Val1.y - Val0.y), Val0.z + t * (Val1.z - Val0.z), Val0.w + t * (Val1.w - Val0.w) };
		Out = Val;
	}
	else if (Val0.Type == GLSL_INT)
	{
		glslExValue Val = { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, (int)(Val0.i + t * (Val1.i - Val0.i)) };
		Out = Val;
	}
}

int ClipTriangleAgainstNearPlane(Triangle* tri, Triangle *outTri) 
{

	for (int i = 0; i < 3; i++)
	{
		outTri[0].Verts[i] = tri->Verts[i];
		outTri[1].Verts[i] = tri->Verts[i];
		outTri[0].TriangleVertexData[i] = NewVector(sizeof(_Pair*));
		outTri[1].TriangleVertexData[i] = NewVector(sizeof(_Pair*));
		for (int j = 0; j < tri->TriangleVertexData[i]->Size; j++)
		{
			_Pair* Pair;

			VectorRead(tri->TriangleVertexData[i], &Pair, j);

			glslExValue PairFirst;
			glslVariable* PairSecond;

			PairGetFirst(Pair, &PairFirst);
			PairGetSecond(Pair, &PairSecond);

			VectorPushBack(outTri[0].TriangleVertexData[i], MakePair(&PairFirst, sizeof(glslExValue), &PairSecond, sizeof(glslVariable*)));
			VectorPushBack(outTri[1].TriangleVertexData[i], MakePair(&PairFirst, sizeof(glslExValue), &PairSecond, sizeof(glslVariable*)));
		}
	}

	glslVec4* InsidePoints[3];  int nInsidePointCount = 0;
	_Vector* InExValues[3];
	glslVec4* OutsidePoints[3]; int nOutsidePointCount = 0;
	_Vector* OutExValues[3];

	InExValues[0] = NewVector(sizeof(_Pair));
	InExValues[1] = NewVector(sizeof(_Pair));
	InExValues[2] = NewVector(sizeof(_Pair));

	OutExValues[0] = NewVector(sizeof(_Pair));
	OutExValues[1] = NewVector(sizeof(_Pair));
	OutExValues[2] = NewVector(sizeof(_Pair));

	if (tri->Verts[0].z >= -tri->Verts[0].w)
	{
		VectorCopy(InExValues[nInsidePointCount], tri->TriangleVertexData[0]);
		InsidePoints[nInsidePointCount++] = &tri->Verts[0];
	}
	else 
	{
		VectorCopy(OutExValues[nOutsidePointCount], tri->TriangleVertexData[0]);
		OutsidePoints[nOutsidePointCount++] = &tri->Verts[0];
	}
	if (tri->Verts[1].z >= -tri->Verts[1].w)
	{
		VectorCopy(InExValues[nInsidePointCount], tri->TriangleVertexData[1]);
		InsidePoints[nInsidePointCount++] = &tri->Verts[1];
	}
	else 
	{
		VectorCopy(OutExValues[nOutsidePointCount], tri->TriangleVertexData[1]);
		OutsidePoints[nOutsidePointCount++] = &tri->Verts[1];
	}
	if (tri->Verts[2].z >= -tri->Verts[2].w)
	{
		VectorCopy(InExValues[nInsidePointCount], tri->TriangleVertexData[2]);
		InsidePoints[nInsidePointCount++] = &tri->Verts[2];
	}
	else 
	{
		VectorCopy(OutExValues[nOutsidePointCount], tri->TriangleVertexData[2]);
		OutsidePoints[nOutsidePointCount++] = &tri->Verts[2];
	}

	if (nInsidePointCount == 0)
	{
		return 0;
	}

	if (nInsidePointCount == 3)
	{
		outTri[0] = *tri;

		return 1;
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		outTri[0].Verts[0] = *InsidePoints[0];
		outTri[0].TriangleVertexData[0] = InExValues[0];

		float t;
		outTri[0].Verts[1] = IntersectNearPlane(*InsidePoints[0], *OutsidePoints[0], &t);
		for (int i = 0; i < tri->TriangleVertexData[1]->Size; i++)
		{
			_Pair Pair0, Pair1;
			VectorRead(InExValues[0], &Pair0, i);
			VectorRead(OutExValues[0], &Pair1, i);
			glslExValue FirstExVal, SecondExVal;
			PairGetFirst(&Pair0, &FirstExVal);
			PairGetSecond(&Pair1, &SecondExVal);
			glslExValue Interpolated = InterpolateExValue(FirstExVal, SecondExVal, t);
			_Pair OutPair;
			VectorRead(outTri[0].TriangleVertexData[1], &OutPair, i);
			PairSetFirst(&OutPair, &Interpolated);
		}

		outTri[0].Verts[2] = IntersectNearPlane(*InsidePoints[0], *OutsidePoints[1], &t);
		for (int i = 0; i < tri->TriangleVertexData[2]->Size; i++)
		{
			_Pair Pair0, Pair1;
			VectorRead(InExValues[0], &Pair0, i);
			VectorRead(OutExValues[1], &Pair1, i);
			glslExValue FirstExVal, SecondExVal;
			PairGetFirst(&Pair0, &FirstExVal);
			PairGetSecond(&Pair1, &SecondExVal);
			glslExValue Interpolated = InterpolateExValue(FirstExVal, SecondExVal, t);
			_Pair OutPair;
			VectorRead(outTri[0].TriangleVertexData[2], &OutPair, i);
			PairSetFirst(&OutPair, &Interpolated);
		}

		VectorFree(InExValues[0]);
		VectorFree(InExValues[1]);
		VectorFree(InExValues[2]);

		VectorFree(OutExValues[0]);
		VectorFree(OutExValues[1]);
		VectorFree(OutExValues[2]);

		return 1;
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		outTri[0].Verts[0] = *InsidePoints[0];
		outTri[0].TriangleVertexData[0] = InExValues[0];
		outTri[0].Verts[1] = *InsidePoints[1];
		outTri[0].TriangleVertexData[1] = InExValues[1];

		float t;
		outTri[0].Verts[2] = IntersectNearPlane(*InsidePoints[0], *OutsidePoints[0], &t);
		for (int i = 0; i < tri->TriangleVertexData[2]->Size; i++)
		{
			_Pair Pair0, Pair1;
			VectorRead(InExValues[0], &Pair0, i);
			VectorRead(OutExValues[0], &Pair1, i);
			glslExValue FirstExVal, SecondExVal;
			PairGetFirst(&Pair0, &FirstExVal);
			PairGetSecond(&Pair1, &SecondExVal);
			glslExValue Interpolated = InterpolateExValue(FirstExVal, SecondExVal, t);
			_Pair OutPair;
			VectorRead(outTri[0].TriangleVertexData[2], &OutPair, i);
			PairSetFirst(&OutPair, &Interpolated);
		}

		outTri[1].Verts[0] = *InsidePoints[1];
		outTri[1].TriangleVertexData[0] = InExValues[1];
		outTri[1].Verts[1] = outTri[0].Verts[2];
		outTri[1].TriangleVertexData[1] = outTri[0].TriangleVertexData[2];
		outTri[1].Verts[2] = IntersectNearPlane(*InsidePoints[1], *OutsidePoints[0], &t);
		for (int i = 0; i < tri->TriangleVertexData[2]->Size; i++)
		{
			_Pair Pair0, Pair1;
			VectorRead(InExValues[1], &Pair0, i);
			VectorRead(OutExValues[0], &Pair1, i);
			glslExValue FirstExVal, SecondExVal;
			PairGetFirst(&Pair0, &FirstExVal);
			PairGetSecond(&Pair1, &SecondExVal);
			glslExValue Interpolated = InterpolateExValue(FirstExVal, SecondExVal, t);
			_Pair OutPair;
			VectorRead(outTri[1].TriangleVertexData[2], &OutPair, i);
			PairSetFirst(&OutPair, &Interpolated);
		}

		VectorFree(InExValues[0]);
		VectorFree(InExValues[1]);
		VectorFree(InExValues[2]);

		VectorFree(OutExValues[0]);
		VectorFree(OutExValues[1]);
		VectorFree(OutExValues[2]);

		return 2;
	}
}

glslMat4 MatMulMat4(glslMat4* a, glslMat4* b)
{
	glslMat4 result;

	result.m00 = a->m00 * b->m00 + a->m01 * b->m10 + a->m02 * b->m20 + a->m03 * b->m30;
	result.m01 = a->m00 * b->m01 + a->m01 * b->m11 + a->m02 * b->m21 + a->m03 * b->m31;
	result.m02 = a->m00 * b->m02 + a->m01 * b->m12 + a->m02 * b->m22 + a->m03 * b->m32;
	result.m03 = a->m00 * b->m03 + a->m01 * b->m13 + a->m02 * b->m23 + a->m03 * b->m33;

	result.m10 = a->m10 * b->m00 + a->m11 * b->m10 + a->m12 * b->m20 + a->m13 * b->m30;
	result.m11 = a->m10 * b->m01 + a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31;
	result.m12 = a->m10 * b->m02 + a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32;
	result.m13 = a->m10 * b->m03 + a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33;

	result.m20 = a->m20 * b->m00 + a->m21 * b->m10 + a->m22 * b->m20 + a->m23 * b->m30;
	result.m21 = a->m20 * b->m01 + a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31;
	result.m22 = a->m20 * b->m02 + a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32;
	result.m23 = a->m20 * b->m03 + a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33;

	result.m30 = a->m30 * b->m00 + a->m31 * b->m10 + a->m32 * b->m20 + a->m33 * b->m30;
	result.m31 = a->m30 * b->m01 + a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31;
	result.m32 = a->m30 * b->m02 + a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32;
	result.m33 = a->m30 * b->m03 + a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33;

	return result;
}

glslMat3 MatMulMat3(glslMat3* a, glslMat3* b)
{
	glslMat3 result;

	result.m00 = a->m00 * b->m00 + a->m01 * b->m10 + a->m02 * b->m20;
	result.m01 = a->m00 * b->m01 + a->m01 * b->m11 + a->m02 * b->m21;
	result.m02 = a->m00 * b->m02 + a->m01 * b->m12 + a->m02 * b->m22;

	result.m10 = a->m10 * b->m00 + a->m11 * b->m10 + a->m12 * b->m20;
	result.m11 = a->m10 * b->m01 + a->m11 * b->m11 + a->m12 * b->m21;
	result.m12 = a->m10 * b->m02 + a->m11 * b->m12 + a->m12 * b->m22;

	result.m20 = a->m20 * b->m00 + a->m21 * b->m10 + a->m22 * b->m20;
	result.m21 = a->m20 * b->m01 + a->m21 * b->m11 + a->m22 * b->m21;
	result.m22 = a->m20 * b->m02 + a->m21 * b->m12 + a->m22 * b->m22;

	return result;
}

glslMat2 MatMulMat2(glslMat2* a, glslMat2* b)
{
	glslMat2 result;

	result.m00 = a->m00 * b->m00 + a->m01 * b->m10;
	result.m01 = a->m00 * b->m01 + a->m01 * b->m11;

	result.m10 = a->m10 * b->m00 + a->m11 * b->m10;
	result.m11 = a->m10 * b->m01 + a->m11 * b->m11;

	return result;
}

glslVec4 MatMulMat4Vec(glslMat4* mat, glslVec4* vec)
{
	glslVec4 result;

	result.x = mat->m00 * vec->x + mat->m01 * vec->y + mat->m02 * vec->z + mat->m03 * vec->w;
	result.y = mat->m10 * vec->x + mat->m11 * vec->y + mat->m12 * vec->z + mat->m13 * vec->w;
	result.z = mat->m20 * vec->x + mat->m21 * vec->y + mat->m22 * vec->z + mat->m23 * vec->w;
	result.w = mat->m30 * vec->x + mat->m31 * vec->y + mat->m32 * vec->z + mat->m33 * vec->w;

	return result;
}

glslVec3 MatMulMat3Vec(glslMat3* mat, glslVec3* vec)
{
	glslVec3 result;

	result.x = mat->m00 * vec->x + mat->m01 * vec->y + mat->m02 * vec->z;
	result.y = mat->m10 * vec->x + mat->m11 * vec->y + mat->m12 * vec->z;
	result.z = mat->m20 * vec->x + mat->m21 * vec->y + mat->m22 * vec->z;

	return result;
}

glslVec2 MatMulMat2Vec(glslMat2* mat, glslVec2* vec)
{
	glslVec2 result;

	result.x = mat->m00 * vec->x + mat->m01 * vec->y;
	result.y = mat->m10 * vec->x + mat->m11 * vec->y;

	return result;
}

glslType GetTypeFromStr(_String* Type)
{
	if (StringEquals(Type, "vec2")) return GLSL_VEC2;
	if (StringEquals(Type, "vec3")) return GLSL_VEC3;
	if (StringEquals(Type, "vec4")) return GLSL_VEC4;
	if (StringEquals(Type, "float")) return GLSL_FLOAT;
	if (StringEquals(Type, "int")) return GLSL_INT;
	if (StringEquals(Type, "mat2")) return GLSL_MAT2;
	if (StringEquals(Type, "mat3")) return GLSL_MAT3;
	if (StringEquals(Type, "mat4")) return GLSL_MAT4;
	if (StringEquals(Type, "sampler2D")) return GLSL_SAMPLER2D;
	return GLSL_UNKNOWN;
}



typedef struct _glslToken
{
	glslTokenType Type;

	struct _glslToken* First;
	struct _glslToken* Second;
	struct _glslToken* Third;

	glslConst Const;
	glslVariable* Var;
	
	_Vector* Swizzle;

	_Vector* Args;
} glslToken;

typedef struct _glslScope
{
	_Vector* Variables;
	struct _glslScope* ParentScope;
	_Vector* Lines;
} glslScope;

typedef struct
{
	glslType ReturnType;

	_String* Name;
	glslScope* RootScope;
	int ParamCount;
} glslFunction;

typedef struct
{
	_Vector* Funcs;
	_Vector* GlobalVars;
} glslTokenized;

typedef struct
{
	int At;
	_String* Code;
	_Vector* GlobalVars;
} glslTokenizer;

int GLSLTellNext(glslTokenizer* Tokenizer, char c)
{
	for (int i = Tokenizer->At; i < Tokenizer->Code->Size; i++)
	{
		if (StringGet(Tokenizer->Code, i) == c)
		{
			return i;
		}
	}
	return 0x7FFFFFFF;
}
int GLSLTellNextWithEnd(glslTokenizer* Tokenizer, char c, int end)
{
	for (int i = Tokenizer->At; i < end; i++)
	{
		if (StringGet(Tokenizer->Code, i) == c)
		{
			return i;
		}
	}
	return 0x7FFFFFFF;
}
int GLSLTellNextMatching(glslTokenizer* Tokenizer, char Inc, char Dec)
{
	int counter = 0;
	for (int i = Tokenizer->At; i < Tokenizer->Code->Size; i++)
	{
		if (StringGet(Tokenizer->Code, i) == Inc) counter++;
		if (StringGet(Tokenizer->Code, i) == Dec)
		{
			counter--;
			if (counter == 0) return i;
		}
	}
	return 0x7FFFFFFF;
}
int GLSLTellNextArgStart(glslTokenizer* Tokenizer)
{
	int counter = 0;
	for (int i = Tokenizer->At; i < Tokenizer->Code->Size; i++)
	{
		if (StringGet(Tokenizer->Code, i) == '(') counter++;
		if (StringGet(Tokenizer->Code, i) == ')') counter--;
		if (counter == 0 && StringGet(Tokenizer->Code, i) == ',') return i;
	}
	return 0x7FFFFFFF;
}
uint8_t GLSLIsOpChar(char c)
{
	if (c == '+') return 1;
	if (c == '-') return 1;
	if (c == '*') return 1;
	if (c == '/') return 1;
	if (c == '<') return 1;
	if (c == '>') return 1;
	if (c == '=') return 1;
	return 0;
}
_Pair* GLSLTellNextOperator(glslTokenizer* Tokenizer, glslTokenType *OutOp)
{
	int counter = 0;
	for (int i = Tokenizer->At; i < Tokenizer->Code->Size; i++)
	{
		if (counter == 0)
		{
			if (GLSLIsOpChar(StringGet(Tokenizer->Code, i)))
			{
				_String* Operator = NewString();
				StringPush(Operator, StringGet(Tokenizer->Code, i));
				if (GLSLIsOpChar(StringGet(Tokenizer->Code, i + 1)))
				{
					StringPush(Operator, StringGet(Tokenizer->Code, i + 1));
				}

				int iplus1 = i + 1;

				if (StringEquals(Operator, "+"))
				{
					*OutOp = GLSL_TOK_ADD;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, "-") && !GLSLIsDigit(StringGet(Tokenizer->Code, i + 1)))
				{
					*OutOp = GLSL_TOK_SUB;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, "*"))
				{
					*OutOp = GLSL_TOK_MUL;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, "/"))
				{
					*OutOp = GLSL_TOK_DIV;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, "="))
				{
					*OutOp = GLSL_TOK_ASSIGN;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, "<"))
				{
					*OutOp = GLSL_TOK_LT;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, ">"))
				{
					*OutOp = GLSL_TOK_GT;
					return MakePair(&i, sizeof(int), &iplus1, sizeof(int));
				}

				if (StringEquals(Operator, "=="))
				{
					int iplus2 = i + 2;
					*OutOp = GLSL_TOK_EQ;
					return MakePair(&i, sizeof(int), &iplus2, sizeof(int));
				}
			}
		}
		if (StringGet(Tokenizer->Code, i) == '(') counter++;
		if (StringGet(Tokenizer->Code, i) == ')') counter--;
	}
	int BigNum = SWGL_BIGNUM;
	return MakePair(&BigNum, sizeof(int), &BigNum, sizeof(int));;
}
uint8_t GLSLFindVariableInScope(glslScope* Scope, _String* Name, glslVariable** Out)
{
	for (int i = 0;i < Scope->Variables->Size;i++)
	{
		glslVariable* Var;
		VectorRead(Scope->Variables, &Var, i);
		if (StringEquals(Var->Name, Name->Data))
		{
			*Out = Var;
			return 1;
		}
	}
	if (Scope->ParentScope) return GLSLFindVariableInScope(Scope->ParentScope, Name, Out);
	return 0;
}
uint8_t GLSLFindVariable(glslTokenizer* Tokenizer, glslScope* Scope, _String* Name, glslVariable** Out)
{
	for (int i = 0;i < Tokenizer->GlobalVars->Size;i++)
	{
		glslVariable* Var;
		VectorRead(Tokenizer->GlobalVars, &Var, i);
		if (StringEquals(Var->Name, Name->Data))
		{
			*Out = Var;
			return 1;
		}
	}
	return GLSLFindVariableInScope(Scope, Name, Out);
}

_String* GLSLTellStringUntil(glslTokenizer* Tokenizer, int idx)
{
	if (idx == 0x7FFFFFFF) return CString2String("ERROR");

	_String* Out = NewString();
	for (int i = Tokenizer->At; i < idx; i++)
	{
		StringPush(Out, StringGet(Tokenizer->Code, i));
	}
	return Out;
}

glslToken* GLSLTokenizeExpr(glslTokenizer* Tokenizer, glslScope* Scope, int EndAt);

_String* GLSLTellStringUntilNWS(glslTokenizer* Tokenizer, int idx)
{
	if (idx == 0x7FFFFFFF) return CString2String("ERROR");

	_String* Out = NewString();
	for (int i = Tokenizer->At; i < idx; i++)
	{
		if (StringGet(Tokenizer->Code, i) == ' ') return Out;
		StringPush(Out, StringGet(Tokenizer->Code, i));
	}
	return Out;
}
glslToken* GLSLTokenizeArgs(glslTokenizer* Tokenizer, glslScope* Scope, int EndAt)
{
	glslToken* Tok = (glslToken*)malloc(sizeof(glslToken));

	Tok->Args = NewVector(sizeof(glslToken*));

	while (Tokenizer->At < EndAt)
	{
		while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;
		int NextArgStart = MIN(EndAt, GLSLTellNextArgStart(Tokenizer));
		glslToken* ArgTok = GLSLTokenizeExpr(Tokenizer, Scope, NextArgStart);
		VectorPushBack(Tok->Args, &ArgTok);

		if (NextArgStart == EndAt)
		{
			break;
		}

		Tokenizer->At = NextArgStart + 1;

		while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;
	}

	Tokenizer->At = EndAt + 1;

	return Tok;
}


glslToken* GLSLTokenizeSubExpr(glslTokenizer* Tokenizer, glslScope* Scope, int EndAt)
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	if (StringGet(Tokenizer->Code, Tokenizer->At) == '(')
	{
		int MatchingParam = GLSLTellNextMatching(Tokenizer, '(', ')');

		Tokenizer->At++;
		glslToken* EnclosedTok = GLSLTokenizeExpr(Tokenizer, Scope, MatchingParam);

		int Swizzle = -1;
		for (int i = Tokenizer->At; i < EndAt; i++)
		{
			if (StringGet(Tokenizer->Code, i) == ' ') break;
			if (StringGet(Tokenizer->Code, i) == '.')
			{
				Swizzle = i + 1;
				break;
			}
		}

		if (Swizzle != -1)
		{
			glslToken* SwizzleTok = (glslToken*)malloc(sizeof(glslToken));

			SwizzleTok->Swizzle = NewVector(sizeof(int));

			SwizzleTok->Type = GLSL_TOK_SWIZZLE;
			SwizzleTok->First = EnclosedTok;
			for (int i = Swizzle; i < EndAt; i++)
			{
				char SwizzleChar = StringGet(Tokenizer->Code, i);

				if (SwizzleChar == 'x' || SwizzleChar == 's')
				{
					int Zero = 0;
					VectorPushBack(SwizzleTok->Swizzle, &Zero);
				}
				else if (SwizzleChar == 'y' || SwizzleChar == 't')
				{
					int One = 1;
					VectorPushBack(SwizzleTok->Swizzle, &One);
				}
				else if (SwizzleChar == 'z')
				{
					int Two = 2;
					VectorPushBack(SwizzleTok->Swizzle, &Two);
				}
				else if (SwizzleChar == 'w')
				{
					int Three = 3;
					VectorPushBack(SwizzleTok->Swizzle, &Three);
				}
				else if (SwizzleChar == ' ')
				{
					break;
				}
				else
				{
					return 0;
				}
			}
			EnclosedTok = SwizzleTok;
		}

		Tokenizer->At = EndAt + 1;
		return EnclosedTok;
	}

	int NextBeginParen = GLSLTellNext(Tokenizer, '(');
	_String* ParenStr = GLSLTellStringUntilNWS(Tokenizer, NextBeginParen);

	glslType TypeFromStr = GetTypeFromStr(ParenStr);

	if (TypeFromStr != GLSL_UNKNOWN)
	{
		Tokenizer->At = NextBeginParen;

		int NextCloseParen = GLSLTellNextMatching(Tokenizer, '(', ')');

		Tokenizer->At++;
		glslToken* OutTok = GLSLTokenizeArgs(Tokenizer, Scope, NextCloseParen);
			
		if (TypeFromStr == GLSL_FLOAT)
		{
			if (OutTok->Args->Size != 1)
			{
				return 0;
			}
			OutTok->Type = GLSL_TOK_FLOAT_CONSTRUCT;
		}
		else if (TypeFromStr == GLSL_VEC2)
		{
			if (OutTok->Args->Size != 2)
			{
				return 0;
			}
			OutTok->Type = GLSL_TOK_VEC2_CONSTRUCT;
		}
		else if (TypeFromStr == GLSL_VEC3)
		{
			if (OutTok->Args->Size != 3)
			{
				return 0;
			}
			OutTok->Type = GLSL_TOK_VEC3_CONSTRUCT;
		}
		else if (TypeFromStr == GLSL_VEC4)
		{
			if (OutTok->Args->Size != 4)
			{
				return 0;
			}
			OutTok->Type = GLSL_TOK_VEC4_CONSTRUCT;
		}
		else if (TypeFromStr == GLSL_INT)
		{
			if (OutTok->Args->Size != 1)
			{
				return 0;
			}
			OutTok->Type = GLSL_TOK_INT_CONSTRUCT;
		}
		return OutTok;
	}

	if (GLSLIsDigit(StringGet(Tokenizer->Code, Tokenizer->At)))
	{
		glslToken* ConstTok = (glslToken*)malloc(sizeof(glslToken));
		ConstTok->Type = GLSL_TOK_CONST;

		uint8_t IsFloat = 0;
		for (int i = Tokenizer->At; i < EndAt; i++)
		{
			if (StringGet(Tokenizer->Code, i) == '.')
			{
				IsFloat = 1;
				break;
			}
		}

		_String* NumStr = GLSLTellStringUntilNWS(Tokenizer, EndAt);

		if (IsFloat)
		{
			ConstTok->Const.IsFloat = 1;
			ConstTok->Const.Fval = atof(String2CString(NumStr));
		}
		else
		{
			ConstTok->Const.IsFloat = 0;
			ConstTok->Const.Ival = atoi(String2CString(NumStr));
		}
		Tokenizer->At = EndAt + 1;
		return ConstTok;
	}
	else
	{
		_String* ProbeVarName = NewString();
		int Swizzle = -1;
		for (int i = Tokenizer->At; i < EndAt; i++)
		{
			if (StringGet(Tokenizer->Code, i) == ' ') break;
			if (StringGet(Tokenizer->Code, i) == '.')
			{
				Swizzle = i + 1;
				break;
			}
			StringPush(ProbeVarName, StringGet(Tokenizer->Code, i));
		}

		glslVariable* ProbeVar;
		if (GLSLFindVariable(Tokenizer, Scope, ProbeVarName, &ProbeVar))
		{
			glslToken* VarTok = (glslToken*)malloc(sizeof(glslToken));
				
			VarTok->Type = GLSL_TOK_VAR;
			VarTok->Var = ProbeVar;
			ProbeVar->Value.Alloc = 0;

			if (Swizzle != -1)
			{
				glslToken* SwizzleTok = (glslToken*)malloc(sizeof(glslToken));
				SwizzleTok->Type = GLSL_TOK_SWIZZLE;
				SwizzleTok->First = VarTok;
				SwizzleTok->Swizzle = NewVector(sizeof(int));
				for (int i = Swizzle; i < EndAt; i++)
				{
					char SwizzleChar = StringGet(Tokenizer->Code, i);

					if (SwizzleChar == 'x' || SwizzleChar == 's')
					{
						int Zero = 0;
						VectorPushBack(SwizzleTok->Swizzle, &Zero);
					}
					else if (SwizzleChar == 'y' || SwizzleChar == 't')
					{
						int One = 1;
						VectorPushBack(SwizzleTok->Swizzle, &One);
					}
					else if (SwizzleChar == 'z')
					{
						int Two = 2;
						VectorPushBack(SwizzleTok->Swizzle, &Two);
					}
					else if (SwizzleChar == 'w')
					{
						int Three = 3;
						VectorPushBack(SwizzleTok->Swizzle, &Three);
					}
					else if (SwizzleChar == ' ')
					{
						break;
					}
				}
				VarTok = SwizzleTok;
			}

			Tokenizer->At = EndAt + 1;
			return VarTok;
		}
		else
		{
			return 0;
		}
	}
}

glslToken* GLSLTokenizeExpr(glslTokenizer* Tokenizer, glslScope* Scope, int EndAt)
{
	glslToken* Tok = (glslToken*)malloc(sizeof(glslToken));

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	if (Tokenizer->At == EndAt)
	{
		return 0;
	}

	glslTokenType OpType;
	_Pair* NextOp = GLSLTellNextOperator(Tokenizer, &OpType);

	int NextOpFirst, NextOpSecond;

	PairGetFirst(NextOp, &NextOpFirst);
	PairGetSecond(NextOp, &NextOpSecond);

	Tok = GLSLTokenizeSubExpr(Tokenizer, Scope, MIN(NextOpFirst, EndAt));



	while (Tokenizer->At < EndAt && NextOpFirst != 0x7FFFFFFF)
	{
		glslToken* SurroundToken = (glslToken*)malloc(sizeof(glslToken));
		SurroundToken->Type = OpType;
		SurroundToken->First = Tok;

		Tokenizer->At = NextOpSecond;

		NextOp = GLSLTellNextOperator(Tokenizer, &OpType);

		PairGetFirst(NextOp, &NextOpFirst);
		PairGetSecond(NextOp, &NextOpSecond);

		SurroundToken->Second = GLSLTokenizeSubExpr(Tokenizer, Scope, MIN(NextOpFirst, EndAt));

		Tok = SurroundToken;
	}

	Tokenizer->At = EndAt + 1; 

	return Tok;
}

glslToken* GLSLTokenizeLine(glslTokenizer* Tokenizer, glslScope* Scope)
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;
	int NextBlank = GLSLTellNext(Tokenizer, ' ');
	int NextSemi = GLSLTellNext(Tokenizer, ';');
	glslTokenType OpType;
	_Pair* NextOperator = GLSLTellNextOperator(Tokenizer, &OpType);

	int NextOperatorFirst, NextOperatorSecond;

	PairGetFirst(NextOperator, &NextOperatorFirst);
	PairGetSecond(NextOperator, &NextOperatorSecond);

	uint8_t Uninitialized = 0;
	if (NextOperatorFirst >= NextSemi)
	{
		Uninitialized = 1;
	}

	glslType DeclType = GetTypeFromStr(GLSLTellStringUntil(Tokenizer, NextBlank));

	if (DeclType != GLSL_UNKNOWN)
	{
		if (OpType != GLSL_TOK_ASSIGN && !Uninitialized)
		{
			return 0;
		}
	}
	else
	{
		if (OpType == GLSL_TOK_ASSIGN)
		{
			glslToken* First = GLSLTokenizeSubExpr(Tokenizer, Scope, NextOperatorFirst);
			Tokenizer->At = NextOperatorSecond;
			glslToken* Result = GLSLTokenizeExpr(Tokenizer, Scope, NextSemi);
			glslToken* Final = (glslToken*)malloc(sizeof(glslToken));
			Final->Type = OpType;
			Final->First = First;
			Final->Second = Result;
			return Final;
		}
		return GLSLTokenizeExpr(Tokenizer, Scope, NextSemi);
	}

	glslToken* OutToken = (glslToken*)malloc(sizeof(glslToken));
	OutToken->Type = Uninitialized ? GLSL_TOK_VAR : GLSL_TOK_VAR_DECL;


	if (DeclType == GLSL_UNKNOWN)
	{
		return 0;
	}

	Tokenizer->At = NextBlank + 1;
	NextBlank = GLSLTellNext(Tokenizer, ' ');

	_String* DeclName = GLSLTellStringUntilNWS(Tokenizer, MIN(NextOperatorFirst, NextSemi));

	glslVariable* DeclVar = (glslVariable*)malloc(sizeof(glslVariable));

	DeclVar->Name = DeclName;
	DeclVar->Type = DeclType;
	DeclVar->isIn = 0;
	DeclVar->isOut = 0;
	DeclVar->isLayout = 0;
	DeclVar->isUniform = 0;
	
	DeclVar->Value.Alloc = 0;

	VectorPushBack(Scope->Variables, &DeclVar);

	if (NextOperatorFirst > NextSemi)
	{
		Tokenizer->At = NextSemi + 1;
		return 0;
	}

	OutToken->Var = DeclVar;

	Tokenizer->At = NextOperatorSecond;
	OutToken->Second = GLSLTokenizeExpr(Tokenizer, Scope, NextSemi);

	return OutToken;
}

glslFunction* GLSLTokenizeFunction(glslTokenizer* Tokenizer)
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	glslFunction* MyFunc = (glslFunction*)malloc(sizeof(glslFunction));

	int NextBlank = GLSLTellNext(Tokenizer, ' ');
	_String* ReturnTypeName = GLSLTellStringUntil(Tokenizer, NextBlank);
	MyFunc->ReturnType = GetTypeFromStr(ReturnTypeName);

	Tokenizer->At = NextBlank + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	int NextParen = GLSLTellNext(Tokenizer, '(');
	int NextClosingParen = GLSLTellNext(Tokenizer, ')');
	MyFunc->Name = GLSLTellStringUntil(Tokenizer, NextParen);

	MyFunc->RootScope = (glslScope*)malloc(sizeof(glslScope));
	MyFunc->RootScope->Lines = NewVector(sizeof(glslToken*));
	MyFunc->RootScope->ParentScope = 0;
	MyFunc->RootScope->Variables = NewVector(sizeof(glslVariable*));

	MyFunc->ParamCount = 0;

	Tokenizer->At = NextParen + 1;

	while (Tokenizer->At < NextClosingParen)
	{
		while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

		int NextBlank = GLSLTellNext(Tokenizer, ' ');

		_String* ParamTypeName = GLSLTellStringUntil(Tokenizer, NextBlank);
		glslType ParamType = GetTypeFromStr(ParamTypeName);

		if (ParamType == GLSL_UNKNOWN)
		{
			return 0;
		}

		Tokenizer->At = NextBlank + 1;

		while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

		int NextParamEnd = MIN(GLSLTellNext(Tokenizer, ','), NextClosingParen);
		_String* ParamName = GLSLTellStringUntilNWS(Tokenizer, NextParamEnd);

		glslVariable* Param = (glslVariable*)malloc(sizeof(glslVariable));
		Param->Type = ParamType;
		Param->Name = ParamName;
		Param->isIn = 0;
		Param->isOut = 0;
		Param->isLayout = 0;
		Param->isUniform = 0;
		Param->Value.Alloc = 0;
		VectorPushBack(MyFunc->RootScope->Variables, &Param);
		MyFunc->ParamCount++;

		Tokenizer->At = NextParamEnd + 1;
	}

	Tokenizer->At = NextClosingParen + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	if (StringGet(Tokenizer->Code, Tokenizer->At) != '{')
	{
		return 0;
	}

	int NextCodeBlockEnd = GLSLTellNextMatching(Tokenizer, '{', '}');

	Tokenizer->At++;

	while (Tokenizer->At < NextCodeBlockEnd)
	{
		glslToken* LineTok = GLSLTokenizeLine(Tokenizer, MyFunc->RootScope);
		VectorPushBack(MyFunc->RootScope->Lines, &LineTok);
	}

	Tokenizer->At = NextCodeBlockEnd + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	return MyFunc;
}

void GLSLTokenizeUniform(glslTokenizer* Tokenizer) 
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	int NextSemi = GLSLTellNext(Tokenizer, ';');
	int NextBlank = GLSLTellNext(Tokenizer, ' ');
	
	if (NextBlank > NextSemi || NextBlank == 0x7FFFFFFF)
	{
		return;
	}

	_String* UniformTypeName = GLSLTellStringUntil(Tokenizer, NextBlank);
	glslType UniformType = GetTypeFromStr(UniformTypeName);

	if (UniformType == GLSL_UNKNOWN)
	{
		return;
	}

	Tokenizer->At = NextBlank + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	_String* UniformName = GLSLTellStringUntilNWS(Tokenizer, NextSemi);

	glslVariable* Uniform = (glslVariable*)malloc(sizeof(glslVariable));

	Uniform->Type = UniformType;
	Uniform->Name = UniformName;
	Uniform->isIn = 0;
	Uniform->isOut = 0;
	Uniform->isLayout = 0;

	Uniform->Value.Alloc = 0;

	Uniform->isUniform = 1;

	Tokenizer->At = NextSemi + 1;

	VectorPushBack(Tokenizer->GlobalVars, &Uniform);
}

void GLSLTokenizeOut(glslTokenizer* Tokenizer)
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	int NextSemi = GLSLTellNext(Tokenizer, ';');
	int NextBlank = GLSLTellNext(Tokenizer, ' ');

	if (NextBlank > NextSemi || NextBlank == 0x7FFFFFFF)
	{
		return;
	}

	_String* OutTypeName = GLSLTellStringUntil(Tokenizer, NextBlank);
	glslType OutType = GetTypeFromStr(OutTypeName);

	if (OutType == GLSL_UNKNOWN)
	{
		return;
	}

	Tokenizer->At = NextBlank + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	_String* OutName = GLSLTellStringUntilNWS(Tokenizer, NextSemi);

	glslVariable* Out = (glslVariable*)malloc(sizeof(glslVariable*));

	Out->Type = OutType;
	Out->Name = OutName;
	Out->isUniform = 0;
	Out->isIn = 0;
	Out->isLayout = 0;
	Out->isUniform = 0;

	Out->Value.Alloc = 0;

	Out->isOut = 1;

	Tokenizer->At = NextSemi + 1;

	VectorPushBack(Tokenizer->GlobalVars, &Out);
}

void GLSLTokenizeIn(glslTokenizer* Tokenizer)
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	int NextSemi = GLSLTellNext(Tokenizer, ';');
	int NextBlank = GLSLTellNext(Tokenizer, ' ');

	if (NextBlank > NextSemi || NextBlank == 0x7FFFFFFF)
	{
		return;
	}

	_String* InTypeName = GLSLTellStringUntil(Tokenizer, NextBlank);
	glslType InType = GetTypeFromStr(InTypeName);

	if (InType == GLSL_UNKNOWN)
	{
		return;
	}

	Tokenizer->At = NextBlank + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	_String* InName = GLSLTellStringUntilNWS(Tokenizer, NextSemi);

	glslVariable* In = (glslVariable*)malloc(sizeof(glslVariable));

	In->Type = InType;
	In->Name = InName;

	In->isOut = 0;
	In->isLayout = 0;
	In->isUniform = 0;

	In->Value.Alloc = 0;

	In->isIn = 1;

	Tokenizer->At = NextSemi + 1;

	VectorPushBack(Tokenizer->GlobalVars, &In);
}

void GLSLTokenizeLayout(glslTokenizer* Tokenizer)
{
	// For now, only accepts location.

	int NextSemi = GLSLTellNext(Tokenizer, ';');

	int NextEq = GLSLTellNext(Tokenizer, '=');
	int NextParen = GLSLTellNext(Tokenizer, ')');

	if (NextParen > NextSemi)
	{
		return;
	}

	if (NextEq > NextParen)
	{
		return;
	}

	_String* ParamName = GLSLTellStringUntilNWS(Tokenizer, NextEq);

	if (!StringEquals(ParamName, "location"))
	{
		return;
	}

	Tokenizer->At = NextEq + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	if (!GLSLIsDigit(StringGet(Tokenizer->Code, Tokenizer->At)))
	{
		return;
	}

	_String* ParamValStr = GLSLTellStringUntilNWS(Tokenizer, NextParen);

	int ParamVal = atoi(String2CString(ParamValStr));

	Tokenizer->At = NextParen + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	int NextBlank = GLSLTellNext(Tokenizer, ' ');

	if (NextBlank > NextSemi)
	{
		return;
	}

	_String* TypeName = GLSLTellStringUntilNWS(Tokenizer, NextBlank);
	glslType Type = GetTypeFromStr(TypeName);

	if (Type == GLSL_UNKNOWN)
	{
		return;
	}

	Tokenizer->At = NextBlank + 1;

	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	_String* Name = GLSLTellStringUntilNWS(Tokenizer, NextSemi);
	
	glslVariable* Variable = (glslVariable*)malloc(sizeof(glslVariable));
	Variable->Type = Type;
	Variable->Name = Name;
	Variable->isIn = 0;
	Variable->isOut = 0;
	Variable->isUniform = 0;
	Variable->isLayout = 1;
	Variable->Layout = (glslLayout*)malloc(sizeof(glslLayout));
	Variable->Layout->Location = ParamVal;
	Variable->Value.Alloc = 0;
	VectorPushBack(Tokenizer->GlobalVars, &Variable);

	Tokenizer->At = NextSemi + 1;
}

glslFunction* GLSLDispatchTokenize(glslTokenizer* Tokenizer)
{
	while (StringGet(Tokenizer->Code, Tokenizer->At) == ' ') Tokenizer->At++;

	int NextBlank = GLSLTellNext(Tokenizer, ' ');
	int NextParen = GLSLTellNext(Tokenizer, '(');

	_String* BlankStr = GLSLTellStringUntilNWS(Tokenizer, NextBlank);
	_String* ParenStr = GLSLTellStringUntilNWS(Tokenizer, NextParen);

	if (StringEquals(BlankStr, "uniform"))
	{
		Tokenizer->At = NextBlank + 1;
		GLSLTokenizeUniform(Tokenizer);
		return 0;
	}

	if (StringEquals(BlankStr, "in"))
	{
		Tokenizer->At = NextBlank + 1;
		GLSLTokenizeIn(Tokenizer);
		return 0;
	}

	if (StringEquals(BlankStr, "out"))
	{
		Tokenizer->At = NextBlank + 1;
		GLSLTokenizeOut(Tokenizer);
		return 0;
	}

	if (StringEquals(ParenStr, "layout"))
	{
		Tokenizer->At = NextParen + 1;
		GLSLTokenizeLayout(Tokenizer);
		return 0;
	}

	return GLSLTokenizeFunction(Tokenizer);
}

glslTokenized GLSLTokenize(_String* ToTokenize)
{
	glslTokenizer* Tokenizer = (glslTokenizer*)malloc(sizeof(glslTokenizer));

	Tokenizer->At = 0;
	Tokenizer->Code = NewString();
	Tokenizer->GlobalVars = NewVector(sizeof(glslVariable*));

	_Vector* OutFuncs = NewVector(sizeof(glslFunction*));

	for (int i = 0; i < ToTokenize->Size; i++)
	{
		char CurChar = StringGet(ToTokenize, i);
		if (CurChar != '\n' && CurChar != '\t') StringPush(Tokenizer->Code, CurChar);
	}
	glslVariable* PositionVariable = (glslVariable*)malloc(sizeof(glslVariable));
	PositionVariable->Name = CString2String("gl_Position");
	PositionVariable->Type = GLSL_VEC4;
	PositionVariable->isIn = 0;
	PositionVariable->isOut = 0;
	PositionVariable->isLayout = 0;
	PositionVariable->isUniform = 0;

	PositionVariable->Value.Alloc = 0;

	VectorPushBack(Tokenizer->GlobalVars, &PositionVariable);

	while (Tokenizer->At < Tokenizer->Code->Size - 2)
	{
		glslFunction* DispatchResult = GLSLDispatchTokenize(Tokenizer);
		if (DispatchResult) VectorPushBack(OutFuncs, &DispatchResult);
	}

	glslTokenized OutputTokenized;
	OutputTokenized.Funcs = OutFuncs;
	OutputTokenized.GlobalVars = Tokenizer->GlobalVars;

	return OutputTokenized;
}

typedef struct
{
	GLenum Type;

	_String* MyCode;
	uint8_t Compiled;
	glslTokenized CompiledData;
} RawShader;

_Vector* GlobalShaders;

void VerifyVar(glslVariable* Var)
{
	if (Var->Value.Alloc == 1) return;

	if (Var->Type == GLSL_FLOAT) Var->Value.Data = malloc(sizeof(float));
	else if (Var->Type == GLSL_VEC2) Var->Value.Data = malloc(sizeof(float) * 2);
	else if (Var->Type == GLSL_VEC3) Var->Value.Data = malloc(sizeof(float) * 3);
	else if (Var->Type == GLSL_VEC4) Var->Value.Data = malloc(sizeof(float) * 4);
	else if (Var->Type == GLSL_INT) Var->Value.Data = malloc(sizeof(int));
	else if (Var->Type == GLSL_MAT2) Var->Value.Data = malloc(sizeof(float) * 4);
	else if (Var->Type == GLSL_MAT3) Var->Value.Data = malloc(sizeof(float) * 9);
	else if (Var->Type == GLSL_MAT4) Var->Value.Data = malloc(sizeof(float) * 16);
	Var->Value.Alloc = 1;
}

void AssignToExVal(glslVariable* AssignTo, glslExValue Val)
{
	VerifyVar(AssignTo);

	if (AssignTo->Type != Val.Type)
	{
		return;
	}

	if (Val.Type == GLSL_FLOAT)
	{
		((float*)AssignTo->Value.Data)[0] = Val.x;
	}

	else if (Val.Type == GLSL_VEC2)
	{
		((float*)AssignTo->Value.Data)[0] = Val.x;
		((float*)AssignTo->Value.Data)[1] = Val.y;
	}

	else if (Val.Type == GLSL_VEC3)
	{
		((float*)AssignTo->Value.Data)[0] = Val.x;
		((float*)AssignTo->Value.Data)[1] = Val.y;
		((float*)AssignTo->Value.Data)[2] = Val.z;
	}

	else if (Val.Type == GLSL_VEC4)
	{
		((float*)AssignTo->Value.Data)[0] = Val.x;
		((float*)AssignTo->Value.Data)[1] = Val.y;
		((float*)AssignTo->Value.Data)[2] = Val.z;
		((float*)AssignTo->Value.Data)[3] = Val.w;
	}

	else if (Val.Type == GLSL_INT)
	{
		((int*)AssignTo->Value.Data)[0] = Val.i;
	}

	else if (Val.Type == GLSL_MAT2)
	{
		((float*)AssignTo->Value.Data)[0] = Val.Mat2.m00;
		((float*)AssignTo->Value.Data)[1] = Val.Mat2.m01;
		((float*)AssignTo->Value.Data)[2] = Val.Mat2.m10;
		((float*)AssignTo->Value.Data)[3] = Val.Mat2.m11;
	}

	else if (Val.Type == GLSL_MAT3)
	{
		((float*)AssignTo->Value.Data)[0] = Val.Mat3.m00;
		((float*)AssignTo->Value.Data)[1] = Val.Mat3.m01;
		((float*)AssignTo->Value.Data)[2] = Val.Mat3.m02;
		((float*)AssignTo->Value.Data)[3] = Val.Mat3.m10;
		((float*)AssignTo->Value.Data)[4] = Val.Mat3.m11;
		((float*)AssignTo->Value.Data)[5] = Val.Mat3.m12;
		((float*)AssignTo->Value.Data)[6] = Val.Mat3.m20;
		((float*)AssignTo->Value.Data)[7] = Val.Mat3.m21;
		((float*)AssignTo->Value.Data)[8] = Val.Mat3.m22;
	}

	else if (Val.Type == GLSL_MAT4)
	{
		((float*)AssignTo->Value.Data)[0] = Val.Mat4.m00;
		((float*)AssignTo->Value.Data)[1] = Val.Mat4.m01;
		((float*)AssignTo->Value.Data)[2] = Val.Mat4.m02;
		((float*)AssignTo->Value.Data)[3] = Val.Mat4.m03;
		((float*)AssignTo->Value.Data)[4] = Val.Mat4.m10;
		((float*)AssignTo->Value.Data)[5] = Val.Mat4.m11;
		((float*)AssignTo->Value.Data)[6] = Val.Mat4.m12;
		((float*)AssignTo->Value.Data)[7] = Val.Mat4.m13;
		((float*)AssignTo->Value.Data)[8] = Val.Mat4.m20;
		((float*)AssignTo->Value.Data)[9] = Val.Mat4.m21;
		((float*)AssignTo->Value.Data)[10] = Val.Mat4.m22;
		((float*)AssignTo->Value.Data)[11] = Val.Mat4.m23;
		((float*)AssignTo->Value.Data)[12] = Val.Mat4.m30;
		((float*)AssignTo->Value.Data)[13] = Val.Mat4.m31;
		((float*)AssignTo->Value.Data)[14] = Val.Mat4.m32;
		((float*)AssignTo->Value.Data)[15] = Val.Mat4.m33;
	}
}

glslExValue VarToExVal(glslVariable* Var)
{
	glslExValue Out;
	Out.Type = Var->Type;

	if (Var->Type == GLSL_FLOAT)
	{
		Out.x = ((float*)Var->Value.Data)[0];
	}

	else if (Var->Type == GLSL_VEC2)
	{
		Out.x = ((float*)Var->Value.Data)[0];
		Out.y = ((float*)Var->Value.Data)[1];
	}

	else if (Var->Type == GLSL_VEC3)
	{
		Out.x = ((float*)Var->Value.Data)[0];
		Out.y = ((float*)Var->Value.Data)[1];
		Out.z = ((float*)Var->Value.Data)[2];
	}

	else if (Var->Type == GLSL_VEC4)
	{
		Out.x = ((float*)Var->Value.Data)[0];
		Out.y = ((float*)Var->Value.Data)[1];
		Out.z = ((float*)Var->Value.Data)[2];
		Out.w = ((float*)Var->Value.Data)[3];
	}

	else if (Var->Type == GLSL_INT)
	{
		Out.i = ((int*)Var->Value.Data)[0];
	}

	return Out;
}

glslExValue ExecuteGLSLToken(glslToken* Token)
{
	if (Token->Type == GLSL_TOK_VAR)
	{
		VerifyVar(Token->Var);

		if (Token->Var->Type == GLSL_FLOAT)
		{
			glslExValue ExOutput = { GLSL_FLOAT, ((float*)Token->Var->Value.Data)[0] };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_VEC2)
		{
			glslExValue ExOutput = { GLSL_VEC2, ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1] };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_VEC3)
		{
			glslExValue ExOutput = { GLSL_VEC3, ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2] };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_VEC4)
		{
			glslExValue ExOutput = { GLSL_VEC4, ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2], ((float*)Token->Var->Value.Data)[3] };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_INT)
		{
			glslExValue ExOutput = { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, ((int*)Token->Var->Value.Data)[0] };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_MAT2)
		{
			glslMat2 MatVal = { ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1],
				  ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2] };
			glslExValue ExOutput = { GLSL_MAT2, 0.0f, 0.0f, 0.0f, 0.0f, 0, MatVal };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_MAT3)
		{
			glslMat3 MatVal = { ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2],
				  ((float*)Token->Var->Value.Data)[3], ((float*)Token->Var->Value.Data)[4],  ((float*)Token->Var->Value.Data)[5],
				  ((float*)Token->Var->Value.Data)[6], ((float*)Token->Var->Value.Data)[7],  ((float*)Token->Var->Value.Data)[8]
			};
			glslExValue ExOutput = { GLSL_MAT3, 0.0f, 0.0f, 0.0f, 0.0f, 0, { 0.0f }, MatVal };
			return ExOutput;
		}
		else if (Token->Var->Type == GLSL_MAT4)
		{
			glslMat4 MatVal = { ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2], ((float*)Token->Var->Value.Data)[3],
				  ((float*)Token->Var->Value.Data)[4], ((float*)Token->Var->Value.Data)[5],  ((float*)Token->Var->Value.Data)[6], ((float*)Token->Var->Value.Data)[7],
				  ((float*)Token->Var->Value.Data)[8], ((float*)Token->Var->Value.Data)[9],  ((float*)Token->Var->Value.Data)[10],  ((float*)Token->Var->Value.Data)[11],
				  ((float*)Token->Var->Value.Data)[10], ((float*)Token->Var->Value.Data)[13],  ((float*)Token->Var->Value.Data)[14],  ((float*)Token->Var->Value.Data)[15],
			};
			glslExValue ExOutput = { GLSL_MAT4, 0.0f, 0.0f, 0.0f, 0.0f, 0, { 0.0f }, { 0.0f }, MatVal };
			return ExOutput;
		}
	}
	else if (Token->Type == GLSL_TOK_CONST)
	{
		if (Token->Const.IsFloat)
		{
			glslExValue ExOutput = { GLSL_FLOAT, Token->Const.Fval };
			return ExOutput;
		}
		else
		{
			glslExValue ExOutput = { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, Token->Const.Ival };
			return ExOutput;
		}
	}
	else if (Token->Type == GLSL_TOK_VAR_DECL)
	{
		VerifyVar(Token->Var);

		glslExValue Result = ExecuteGLSLToken(Token->Second);

		AssignToExVal(Token->Var, Result);
		glslExValue ExOutput = { GLSL_UNKNOWN };
		return ExOutput;
	}
	else if (Token->Type == GLSL_TOK_ASSIGN)
	{
		VerifyVar(Token->First->Var);

		glslExValue Result = ExecuteGLSLToken(Token->Second);

		AssignToExVal(Token->First->Var, Result);
		return Result;
	}
	else if (Token->Type == GLSL_TOK_ADD)
	{
		glslExValue FirstResult = ExecuteGLSLToken(Token->First);
		glslExValue SecondResult = ExecuteGLSLToken(Token->Second);

		if (FirstResult.Type != SecondResult.Type)
		{
			glslExValue ExOutput = { GLSL_UNKNOWN };
			return ExOutput;
		}

		FirstResult.x += SecondResult.x;
		FirstResult.y += SecondResult.y;
		FirstResult.z += SecondResult.z;
		FirstResult.w += SecondResult.w;
		FirstResult.i += SecondResult.i;

		if (FirstResult.Type == GLSL_MAT2)
		{
			FirstResult.Mat2.m00 += SecondResult.Mat2.m00;
			FirstResult.Mat2.m01 += SecondResult.Mat2.m01;

			FirstResult.Mat2.m10 += SecondResult.Mat2.m10;
			FirstResult.Mat2.m11 += SecondResult.Mat2.m11;
		}
		else if (FirstResult.Type == GLSL_MAT3)
		{
			FirstResult.Mat3.m00 += SecondResult.Mat3.m00;
			FirstResult.Mat3.m01 += SecondResult.Mat3.m01;
			FirstResult.Mat3.m02 += SecondResult.Mat3.m02;

			FirstResult.Mat3.m10 += SecondResult.Mat3.m10;
			FirstResult.Mat3.m11 += SecondResult.Mat3.m11;
			FirstResult.Mat3.m12 += SecondResult.Mat3.m12;

			FirstResult.Mat3.m20 += SecondResult.Mat3.m20;
			FirstResult.Mat3.m21 += SecondResult.Mat3.m21;
			FirstResult.Mat3.m22 += SecondResult.Mat3.m22;
		}
		else if (FirstResult.Type == GLSL_MAT4)
		{
			FirstResult.Mat4.m00 += SecondResult.Mat4.m00;
			FirstResult.Mat4.m01 += SecondResult.Mat4.m01;
			FirstResult.Mat4.m02 += SecondResult.Mat4.m02;
			FirstResult.Mat4.m03 += SecondResult.Mat4.m02;

			FirstResult.Mat4.m10 += SecondResult.Mat4.m10;
			FirstResult.Mat4.m11 += SecondResult.Mat4.m11;
			FirstResult.Mat4.m12 += SecondResult.Mat4.m12;
			FirstResult.Mat4.m13 += SecondResult.Mat4.m12;

			FirstResult.Mat4.m20 += SecondResult.Mat4.m20;
			FirstResult.Mat4.m21 += SecondResult.Mat4.m21;
			FirstResult.Mat4.m22 += SecondResult.Mat4.m22;
			FirstResult.Mat4.m23 += SecondResult.Mat4.m22;

			FirstResult.Mat4.m30 += SecondResult.Mat4.m30;
			FirstResult.Mat4.m31 += SecondResult.Mat4.m31;
			FirstResult.Mat4.m32 += SecondResult.Mat4.m32;
			FirstResult.Mat4.m33 += SecondResult.Mat4.m33;
		}

		return FirstResult;
	}
	else if (Token->Type == GLSL_TOK_SUB)
	{
		glslExValue FirstResult = ExecuteGLSLToken(Token->First);
		glslExValue SecondResult = ExecuteGLSLToken(Token->Second);

		if (FirstResult.Type != SecondResult.Type)
		{
			glslExValue ExOutput = { GLSL_UNKNOWN };
			return ExOutput;
		}

		FirstResult.x -= SecondResult.x;
		FirstResult.y -= SecondResult.y;
		FirstResult.z -= SecondResult.z;
		FirstResult.w -= SecondResult.w;
		FirstResult.i -= SecondResult.i;

		if (FirstResult.Type == GLSL_MAT2)
		{
			FirstResult.Mat2.m00 -= SecondResult.Mat2.m00;
			FirstResult.Mat2.m01 -= SecondResult.Mat2.m01;

			FirstResult.Mat2.m10 -= SecondResult.Mat2.m10;
			FirstResult.Mat2.m11 -= SecondResult.Mat2.m11;
		}
		else if (FirstResult.Type == GLSL_MAT3)
		{
			FirstResult.Mat3.m00 -= SecondResult.Mat3.m00;
			FirstResult.Mat3.m01 -= SecondResult.Mat3.m01;
			FirstResult.Mat3.m02 -= SecondResult.Mat3.m02;

			FirstResult.Mat3.m10 -= SecondResult.Mat3.m10;
			FirstResult.Mat3.m11 -= SecondResult.Mat3.m11;
			FirstResult.Mat3.m12 -= SecondResult.Mat3.m12;

			FirstResult.Mat3.m20 -= SecondResult.Mat3.m20;
			FirstResult.Mat3.m21 -= SecondResult.Mat3.m21;
			FirstResult.Mat3.m22 -= SecondResult.Mat3.m22;
		}
		else if (FirstResult.Type == GLSL_MAT4)
		{
			FirstResult.Mat4.m00 -= SecondResult.Mat4.m00;
			FirstResult.Mat4.m01 -= SecondResult.Mat4.m01;
			FirstResult.Mat4.m02 -= SecondResult.Mat4.m02;
			FirstResult.Mat4.m03 -= SecondResult.Mat4.m02;

			FirstResult.Mat4.m10 -= SecondResult.Mat4.m10;
			FirstResult.Mat4.m11 -= SecondResult.Mat4.m11;
			FirstResult.Mat4.m12 -= SecondResult.Mat4.m12;
			FirstResult.Mat4.m13 -= SecondResult.Mat4.m12;

			FirstResult.Mat4.m20 -= SecondResult.Mat4.m20;
			FirstResult.Mat4.m21 -= SecondResult.Mat4.m21;
			FirstResult.Mat4.m22 -= SecondResult.Mat4.m22;
			FirstResult.Mat4.m23 -= SecondResult.Mat4.m22;

			FirstResult.Mat4.m30 -= SecondResult.Mat4.m30;
			FirstResult.Mat4.m31 -= SecondResult.Mat4.m31;
			FirstResult.Mat4.m32 -= SecondResult.Mat4.m32;
			FirstResult.Mat4.m33 -= SecondResult.Mat4.m33;
		}

		return FirstResult;
	}
	else if (Token->Type == GLSL_TOK_MUL)
	{
		glslExValue FirstResult = ExecuteGLSLToken(Token->First);
		glslExValue SecondResult = ExecuteGLSLToken(Token->Second);

		if (FirstResult.Type != GLSL_MAT2 && FirstResult.Type != GLSL_MAT3 && FirstResult.Type != GLSL_MAT4)
		{
			if (FirstResult.Type != SecondResult.Type)
			{
				glslExValue ExOutput = { GLSL_UNKNOWN };
				return ExOutput;
			}
			FirstResult.x *= SecondResult.x;
			FirstResult.y *= SecondResult.y;
			FirstResult.z *= SecondResult.z;
			FirstResult.w *= SecondResult.w;
			FirstResult.i *= SecondResult.i;
		}
		else
		{
			if (FirstResult.Type == GLSL_MAT2 && SecondResult.Type == GLSL_MAT2)
			{
				FirstResult.Mat2 = MatMulMat2(&FirstResult.Mat2, &SecondResult.Mat2);
			}
			else if (FirstResult.Type == GLSL_MAT2 && SecondResult.Type == GLSL_VEC2)
			{
				glslVec2 InVec2 = { SecondResult.x, SecondResult.y };
				glslVec2 Result = MatMulMat2Vec(&FirstResult.Mat2, &InVec2);
				FirstResult.x = Result.x;
				FirstResult.y = Result.y;
				FirstResult.Type = GLSL_VEC2;
			}
			else if (FirstResult.Type == GLSL_MAT3 && SecondResult.Type == GLSL_MAT3)
			{
				FirstResult.Mat3 = MatMulMat3(&FirstResult.Mat3, &SecondResult.Mat3);
			}
			else if (FirstResult.Type == GLSL_MAT3 && SecondResult.Type == GLSL_VEC3)
			{
				glslVec3 InVec3 = { SecondResult.x, SecondResult.y, SecondResult.z };
				glslVec3 Result = MatMulMat3Vec(&FirstResult.Mat3, &InVec3);
				FirstResult.x = Result.x;
				FirstResult.y = Result.y;
				FirstResult.z = Result.z;
				FirstResult.Type = GLSL_VEC3;
			}
			else if (FirstResult.Type == GLSL_MAT4 && SecondResult.Type == GLSL_MAT4)
			{
				FirstResult.Mat4 = MatMulMat4(&FirstResult.Mat4, &SecondResult.Mat4);
			}
			else if (FirstResult.Type == GLSL_MAT4 && SecondResult.Type == GLSL_VEC4)
			{
				glslVec4 InVec4 = { SecondResult.x, SecondResult.y, SecondResult.z, SecondResult.w };
				glslVec4 Result = MatMulMat4Vec(&FirstResult.Mat4, &InVec4);
				FirstResult.x = Result.x;
				FirstResult.y = Result.y;
				FirstResult.z = Result.z;
				FirstResult.w = Result.w;
				FirstResult.Type = GLSL_VEC4;
			}
		}


		return FirstResult;
	}
	else if (Token->Type == GLSL_TOK_DIV)
	{
		glslExValue FirstResult = ExecuteGLSLToken(Token->First);
		glslExValue SecondResult = ExecuteGLSLToken(Token->Second);

		if (FirstResult.Type != SecondResult.Type)
		{
			glslExValue ExOutput = { GLSL_UNKNOWN };
			return ExOutput;
		}

		FirstResult.x /= SecondResult.x;
		FirstResult.y /= SecondResult.y;
		FirstResult.z /= SecondResult.z;
		FirstResult.w /= SecondResult.w;
		if (SecondResult.i != 0) FirstResult.i /= SecondResult.i;
		
		return FirstResult;
	}
	else if (Token->Type == GLSL_TOK_SWIZZLE)
	{
		glslExValue Input = ExecuteGLSLToken(Token->First);

		glslExValue Output;

		for (int i = 0; i < Token->Swizzle->Size; i++)
		{
			float CurVal = 0;
			int CurSwizzle;
			
			VectorRead(Token->Swizzle, &CurSwizzle, i);

			if (CurSwizzle == 0)
			{
				CurVal = Input.x;
			}
			else if (CurSwizzle == 1)
			{
				CurVal = Input.y;
			}
			else if (CurSwizzle == 2)
			{
				CurVal = Input.z;
			}
			else if (CurSwizzle == 3)
			{
				CurVal = Input.w;
			}

			if (i == 0)
			{
				Output.x = CurVal;
			}
			else if (i == 1)
			{
				Output.y = CurVal;
			}
			else if (i == 2)
			{
				Output.z = CurVal;
			}
			else if (i == 3)
			{
				Output.w = CurVal;
			}
		}

		if (Token->Swizzle->Size == 0)
		{
			glslExValue ExOutput = { GLSL_UNKNOWN };
			return ExOutput;
		}
		else if (Token->Swizzle->Size == 1)
		{
			Output.Type = GLSL_FLOAT;
		}
		else if (Token->Swizzle->Size == 2)
		{
			Output.Type = GLSL_VEC2;
		}
		else if (Token->Swizzle->Size == 3)
		{
			Output.Type = GLSL_VEC3;
		}
		else if (Token->Swizzle->Size == 4)
		{
			Output.Type = GLSL_VEC4;
		}
		return Output;
	}
	else if (Token->Type == GLSL_TOK_FLOAT_CONSTRUCT)
	{
		glslToken* TokArg;

		VectorRead(Token->Args, &TokArg, 0);
		glslExValue Arg0 = ExecuteGLSLToken(TokArg);

		glslExValue ExOutput = { GLSL_FLOAT, Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i };
		return ExOutput;
	}
	else if (Token->Type == GLSL_TOK_VEC2_CONSTRUCT)
	{
		glslToken* TokArg;

		VectorRead(Token->Args, &TokArg, 0);
		glslExValue Arg0 = ExecuteGLSLToken(TokArg);
		VectorRead(Token->Args, &TokArg, 1);
		glslExValue Arg1 = ExecuteGLSLToken(TokArg);

		glslExValue ExOutput = { GLSL_VEC2, 
			Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i,
			Arg1.Type != GLSL_INT ? Arg1.x : (float)Arg1.i
		};
		return ExOutput;
	}
	else if (Token->Type == GLSL_TOK_VEC3_CONSTRUCT)
	{
		glslToken* TokArg;

		VectorRead(Token->Args, &TokArg, 0);
		glslExValue Arg0 = ExecuteGLSLToken(TokArg);
		VectorRead(Token->Args, &TokArg, 1);
		glslExValue Arg1 = ExecuteGLSLToken(TokArg);
		VectorRead(Token->Args, &TokArg, 2);
		glslExValue Arg2 = ExecuteGLSLToken(TokArg);

		glslExValue ExOutput = { GLSL_VEC3,
			Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i,
			Arg1.Type != GLSL_INT ? Arg1.x : (float)Arg1.i,
			Arg2.Type != GLSL_INT ? Arg2.x : (float)Arg2.i
		};
		return ExOutput;
	}
	else if (Token->Type == GLSL_TOK_VEC4_CONSTRUCT)
	{
		glslToken* TokArg;

		VectorRead(Token->Args, &TokArg, 0);
		glslExValue Arg0 = ExecuteGLSLToken(TokArg);
		VectorRead(Token->Args, &TokArg, 1);
		glslExValue Arg1 = ExecuteGLSLToken(TokArg);
		VectorRead(Token->Args, &TokArg, 2);
		glslExValue Arg2 = ExecuteGLSLToken(TokArg);
		VectorRead(Token->Args, &TokArg, 3);
		glslExValue Arg3 = ExecuteGLSLToken(TokArg);

		glslExValue ExOutput = { GLSL_VEC4,
			Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i,
			Arg1.Type != GLSL_INT ? Arg1.x : (float)Arg1.i,
			Arg2.Type != GLSL_INT ? Arg2.x : (float)Arg2.i,
			Arg3.Type != GLSL_INT ? Arg3.x : (float)Arg3.i
		};
		return ExOutput;
	}
	else if (Token->Type == GLSL_TOK_INT_CONSTRUCT)
	{
		glslToken* TokArg;

		VectorRead(Token->Args, &TokArg, 0);
		glslExValue Arg0 = ExecuteGLSLToken(TokArg);

		glslExValue ExOutput = { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, Arg0.Type != GLSL_INT ? (int)Arg0.x : Arg0.i };
		return ExOutput;
	}
}

void ExecuteGLSLFunction(glslFunction* Func)
{
	for (int i = 0;i < Func->RootScope->Lines->Size;i++)
	{
		glslToken* LineTok;

		VectorRead(Func->RootScope->Lines, &LineTok, i);

		if (!LineTok) continue;
		ExecuteGLSLToken(LineTok);
	}
}

void ExecuteGLSL(glslTokenized Tokens)
{
	for (int i = 0; i < Tokens.Funcs->Size; i++)
	{
		glslFunction* Func;

		VectorRead(Tokens.Funcs, &Func, i);

		if (StringEquals(Func->Name, "main"))
		{
			ExecuteGLSLFunction(Func);
		}
	}
}

GLuint glCreateShader(GLenum type)
{
	RawShader* Shader = (RawShader*)malloc(sizeof(RawShader));
	Shader->Type = type;
	VectorPushBack(GlobalShaders, &Shader);
	return GlobalShaders->Size - 1;
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
	_String* ShaderCode = NewString();
	for (int i = 0;i < count;i++)
	{
		if (!length)
		{
			StringAppend(ShaderCode, CString2String((const char*)string[i]));
		}
		else
		{
			for (int j = 0; j < length[i]; j++)
			{
				StringPush(ShaderCode, string[i][j]);
			}
		}
	}
	
	RawShader* TargetShader;

	VectorRead(GlobalShaders, &TargetShader, shader);

	TargetShader->MyCode = ShaderCode;
}

void glCompileShader(GLuint shader)
{
	RawShader* TargetShader;

	VectorRead(GlobalShaders, &TargetShader, shader);

	TargetShader->CompiledData = GLSLTokenize(TargetShader->MyCode);
	TargetShader->Compiled = 1;
}

void glDeleteShader(GLuint shader)
{
	// Deletion is for bitches;
}

typedef struct
{
	uint8_t Linked;
	_Vector* VertexFragInOut;
	_Vector* Uniforms;
	_Vector* Layouts;

	uint8_t HasVertex;
	uint8_t HasFrag;
	glslTokenized VertexShader;
	glslTokenized FragmentShader;
} Program;

Program* ActiveProgram;
_Vector* GlobalPrograms;

GLuint glCreateProgram()
{
	Program* NewProgram = (Program*)malloc(sizeof(Program));
	NewProgram->VertexFragInOut = NewVector(sizeof(_Pair*));
	VectorPushBack(GlobalPrograms, &NewProgram);
	return GlobalPrograms->Size;
}

void glAttachShader(GLuint program, GLuint shader)
{
	Program* MyProgram;
	RawShader* MyShader;

	VectorRead(GlobalPrograms, &MyProgram, program - 1);
	VectorRead(GlobalShaders, &MyShader, shader);

	if (MyShader->Type == GL_VERTEX_SHADER)
	{
		MyProgram->HasVertex = 1;
		MyProgram->VertexShader = MyShader->CompiledData;
	}
	if (MyShader->Type == GL_FRAGMENT_SHADER)
	{
		MyProgram->HasFrag = 1;
		MyProgram->FragmentShader = MyShader->CompiledData;
	}
}

void glLinkProgram(GLuint program)
{
	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, program - 1);

	_Vector* VertOuts = NewVector(sizeof(glslVariable*));
	_Vector* FragIns = NewVector(sizeof(glslVariable*));

	_Vector* Uniforms = NewVector(sizeof(glslVariable*));
	_Vector* Layouts = NewVector(sizeof(glslVariable*));


	for (int i = 0; i < MyProgram->VertexShader.GlobalVars->Size;i++)
	{
		glslVariable* VertVar;

		VectorRead(MyProgram->VertexShader.GlobalVars, &VertVar, i);

		if (VertVar->isOut) VectorPushBack(VertOuts, &VertVar);
		if (VertVar->isUniform) VectorPushBack(Uniforms, &VertVar);
		if (VertVar->isLayout) VectorPushBack(Layouts, &VertVar);
	}

	for (int i = 0; i < MyProgram->FragmentShader.GlobalVars->Size; i++)
	{
		glslVariable* FragVar;

		VectorRead(MyProgram->FragmentShader.GlobalVars, &FragVar, i);

		if (FragVar->isIn) VectorPushBack(FragIns, &FragVar);
		if (FragVar->isUniform) VectorPushBack(Uniforms, &FragVar);
		if (FragVar->isLayout) VectorPushBack(Layouts, &FragVar);
	}
	
	MyProgram->Uniforms = Uniforms;
	MyProgram->Layouts = Layouts;

	for (int i = 0;i < FragIns->Size;i++)
	{
		glslVariable* FragIn;

		VectorRead(FragIns, &FragIn, i);

		uint8_t LinkedPair = 0;

		for (int j = 0; j < VertOuts->Size; j++)
		{
			glslVariable* VertOut;

			VectorRead(VertOuts, &VertOut, j);

			if (StringEquals(FragIn->Name, String2CString(VertOut->Name)))
			{
				_Pair* AddPair = MakePair(&FragIn, sizeof(glslVariable*), &VertOut, sizeof(glslVariable*));
				VectorPushBack(MyProgram->VertexFragInOut, &AddPair);
				LinkedPair = 1;
				break;
			}
		}
	}

	MyProgram->Linked = 1;
}

void glUseProgram(GLuint program)
{
	if (program == 0) ActiveProgram = 0;
	else VectorRead(GlobalPrograms, &ActiveProgram, program - 1);
}

typedef struct
{
	void* data;
	GLsizei size;
} Buffer;

typedef struct
{
	GLsizei stride;
	GLboolean normalized;
	GLenum type;
	GLint size;
	GLuint index;

	int offset;
} VertexArrayAttrib;

typedef struct
{
	_Vector* Attribs;
	Buffer* VertexBuffer;
	Buffer* ElementBuffer;
} VertexArray;

VertexArray* ActiveVertexArray;
_Vector* GlobalVertexArrays;

GLuint glGenVertexArrays(GLsizei n, GLuint* arrays)
{
	// Only supports one vertex array per call for now
	*arrays = GlobalVertexArrays->Size + 1;

	VertexArray* VertArray = (VertexArray*)malloc(sizeof(VertexArray));
	VertArray->Attribs = NewVector(sizeof(VertexArrayAttrib));
	VertArray->ElementBuffer = (Buffer*)malloc(sizeof(Buffer));
	VertArray->ElementBuffer->data = 0;
	VertArray->ElementBuffer->size = 0;
	VertArray->VertexBuffer = (Buffer*)malloc(sizeof(Buffer));
	VertArray->VertexBuffer->data = 0;
	VertArray->VertexBuffer->size = 0;

	VectorPushBack(GlobalVertexArrays, &VertArray);
	return 0;
}

void glBindVertexArray(GLuint array)
{
	if (array == 0) ActiveVertexArray = 0;
	else VectorRead(GlobalVertexArrays, &ActiveVertexArray, array - 1);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
	if (ActiveVertexArray)
	{
		VertexArrayAttrib Attrib;

		Attrib.index = index;
		Attrib.normalized = normalized;
		Attrib.offset = (int)pointer;
		Attrib.size = size;
		Attrib.stride = stride;
		Attrib.type = type;

		VectorPushBack(ActiveVertexArray->Attribs, &Attrib);
	}
}

_Vector* GlobalBuffers;

GLuint glGenBuffers(GLsizei n, GLuint* buffers)
{
	// Only supports 1 buffer per call for now
	*buffers = GlobalBuffers->Size + 1;

	Buffer* NewBuffer = (Buffer*)malloc(sizeof(Buffer));

	NewBuffer->data = 0;
	NewBuffer->size = 0;

	VectorPushBack(GlobalBuffers, &NewBuffer);
	return 0;
}

Buffer* GlobalArrayBuffer;

void glBindBuffer(GLenum type, GLuint buffer)
{
	if (type == GL_ARRAY_BUFFER)
	{
		if (buffer == 0)
		{
			GlobalArrayBuffer = 0;
			return;
		}

		Buffer* TargetBuffer;

		VectorRead(GlobalBuffers, &TargetBuffer, buffer - 1);

		if (ActiveVertexArray)
		{
			GlobalArrayBuffer = ActiveVertexArray->VertexBuffer;

			GlobalArrayBuffer->data = TargetBuffer->data;
			GlobalArrayBuffer->size = TargetBuffer->size;
		}
		else
		{
			GlobalArrayBuffer = TargetBuffer;
		}
	}
}
void glBufferData(GLenum target, GLsizei size, const void* data, GLenum usage)
{
	Buffer* MyBuffer = 0;

	if (target == GL_ARRAY_BUFFER)
	{
		MyBuffer = GlobalArrayBuffer;
	}

	if (MyBuffer)
	{
		if (MyBuffer->size == 0)
		{
			MyBuffer->data = malloc(size);
			MyBuffer->size = size;
			memcpy(MyBuffer->data, data, size);
		}
	}
}

GLint ViewportX;
GLint ViewportY;
GLsizei ViewportWidth;
GLsizei ViewportHeight;

typedef struct
{
	GLsizei Width;
	GLsizei Height;

	GLenum ColorFormat;
	uint32_t* ColorAttachment;

	GLenum DepthFormat;
	void* DepthAttachment;
} Framebuffer;

Framebuffer* GlobalFramebuffer;

GLfloat ClearColorRed;
GLfloat ClearColorGreen;
GLfloat ClearColorBlue;
GLfloat ClearColorAlpha;

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	ClearColorRed = MIN(MAX(red, 0.0f), 1.0f);
	ClearColorGreen = MIN(MAX(green, 0.0f), 1.0f);
	ClearColorBlue = MIN(MAX(blue, 0.0f), 1.0f);
	ClearColorAlpha = MIN(MAX(alpha, 0.0f), 1.0f);
}

void glClear(GLuint flags)
{
	if (flags & GL_COLOR_BUFFER_BIT)
	{
		uint32_t ClearColor = 0;
		ClearColor |= (uint32_t)(ClearColorRed * 255) << 24;
		ClearColor |= (uint32_t)(ClearColorGreen * 255) << 16;
		ClearColor |= (uint32_t)(ClearColorBlue * 255) << 8;
		ClearColor |= (uint32_t)(ClearColorAlpha * 255);

		for (int y = ViewportY; y < ViewportY + ViewportHeight; y++)
		{
			if (y < 0) continue;
			if (y >= GlobalFramebuffer->Height) break;
			for (int x = ViewportX; x < ViewportX + ViewportWidth; x++)
			{
				if (x < 0) continue;
				if (x >= GlobalFramebuffer->Width) break;
				GlobalFramebuffer->ColorAttachment[y * GlobalFramebuffer->Width + x] = ClearColor;
			}
		}
	}
	if (flags & GL_DEPTH_BUFFER_BIT)
	{
		if (GlobalFramebuffer->DepthFormat == GL_FLOAT)
		{
			for (int y = ViewportY; y < ViewportY + ViewportHeight; y++)
			{
				if (y < 0) continue;
				if (y >= GlobalFramebuffer->Height) break;
				for (int x = ViewportX; x < ViewportX + ViewportWidth; x++)
				{
					if (x < 0) continue;
					if (x >= GlobalFramebuffer->Width) break;
					((GLfloat*)GlobalFramebuffer->DepthAttachment)[y * GlobalFramebuffer->Width + x] = 0.0f;
				}
			}
		}
	}
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	ViewportX = x;
	ViewportY = y;
	ViewportWidth = width;
	ViewportHeight = height;
}

glslVec4 Sub(glslVec4 x, glslVec4 y)
{
	x.x -= y.x;
	x.y -= y.y;
	x.z -= y.z;
	x.w -= y.w;
	return x;
}

float Dot2(glslVec4 x, glslVec4 y)
{
	return x.x * y.x + x.y * y.y;
}

void Barycentric(glslVec4 a, glslVec4 b, glslVec4 c, glslVec4 p, float* u, float* v, float* w)
{
	glslVec4 v0 = Sub(b, a), v1 = Sub(c, a), v2 = Sub(p, a);
	float d00 = Dot2(v0, v0);
	float d01 = Dot2(v0, v1);
	float d11 = Dot2(v1, v1);
	float d20 = Dot2(v2, v0);
	float d21 = Dot2(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	*v = (d11 * d20 - d01 * d21) / denom;
	*w = (d00 * d21 - d01 * d20) / denom;
	*u = 1.0f - *v - *w;
}

glslExValue InterpolateLinearEx(glslExValue a, glslExValue b, glslExValue c, float aW, float bW, float cW)
{
	glslExValue Out;
	Out.Type = a.Type;
	if (a.Type == GLSL_FLOAT)
	{
		Out.x = a.x * aW + b.x * bW + c.x * cW;
	}
	else if (a.Type == GLSL_VEC2)
	{
		Out.x = a.x * aW + b.x * bW + c.x * cW;
		Out.y = a.y * aW + b.y * bW + c.y * cW;
	}
	else if (a.Type == GLSL_VEC3)
	{
		Out.x = a.x * aW + b.x * bW + c.x * cW;
		Out.y = a.y * aW + b.y * bW + c.y * cW;
		Out.z = a.z * aW + b.z * bW + c.z * cW;
	}
	else if (a.Type == GLSL_VEC4)
	{
		Out.x = a.x * aW + b.x * bW + c.x * cW;
		Out.y = a.y * aW + b.y * bW + c.y * cW;
		Out.z = a.z * aW + b.z * bW + c.z * cW;
		Out.w = a.w * aW + b.w * bW + c.w * cW;
	}
	return Out;
}

// COORD DATA ORIGINALLY WAS std::vector<std::pair<glslExValue, glslVariable*>>*

void DrawTriangle(glslVec4* Coords, _Vector** CoordData)
{

	float minX = MAX(MIN(MIN(Coords[0].x, Coords[1].x), Coords[2].x), (float)ViewportX);
	float maxX = MIN(MAX(MAX(Coords[0].x, Coords[1].x), Coords[2].x), (float)ViewportX + ViewportWidth - 1);

	float minY = MAX(MIN(MIN(Coords[0].y, Coords[1].y), Coords[2].y), (float)ViewportY);
	float maxY = MIN(MAX(MAX(Coords[0].y, Coords[1].y), Coords[2].y), (float)ViewportY + ViewportHeight - 1);
	

	for (float y = minY;y <= maxY;y++)
	{
		if (y < 0) continue;
		if (y >= GlobalFramebuffer->Height) break;
		for (float x = minX; x <= maxX; x++)
		{
			if (x < 0) continue;
			if (x >= GlobalFramebuffer->Width) break;
			
			glslVec4 MyPoint = { x, y, 0.0f, 0.0f };

			float u, v, w;
			Barycentric(Coords[0], Coords[1], Coords[2], MyPoint, &u, &v, &w);

			if (u >= -0.002f && v >= -0.002f && w >= -0.002f)
			{
				float uCorrected = u / Coords[0].w;
				float vCorrected = v / Coords[1].w;
				float wCorrected = w / Coords[2].w;

				float sum = uCorrected + vCorrected + wCorrected;

				uCorrected /= sum;
				vCorrected /= sum;
				wCorrected /= sum;

				u = uCorrected;
				v = vCorrected;
				w = wCorrected;

				float z = (Coords[0].z * u + Coords[1].z * v + Coords[2].z * w);

				if (GlobalFramebuffer->DepthFormat == GL_FLOAT)
				{
					float* CurZ = &(((float*)GlobalFramebuffer->DepthAttachment)[(int)x + (int)y * GlobalFramebuffer->Width]);
					if (*CurZ == 0.0f || *CurZ >= z)
					{
						*CurZ = z;

						uint32_t* CurCol = &(GlobalFramebuffer->ColorAttachment[(int)x + ((ViewportHeight - ((int)y - ViewportY + 1)) + ViewportY) * GlobalFramebuffer->Width]);


						for (int i = 0; i < CoordData[0]->Size; i++)
						{
							_Pair* FirstArg, *SecondArg, *ThirdArg;

							VectorRead(CoordData[0], &FirstArg, i);
							VectorRead(CoordData[1], &SecondArg, i);
							VectorRead(CoordData[2], &ThirdArg, i);

							glslExValue FirstEx, SecondEx, ThirdEx;

							PairGetFirst(FirstArg, &FirstEx);
							PairGetFirst(SecondArg, &SecondEx);
							PairGetFirst(ThirdArg, &ThirdEx);

							glslExValue InterpVal = InterpolateLinearEx(FirstEx, SecondEx, ThirdEx, u, v, w);

							glslVariable* OutVar;
							
							PairGetSecond(FirstArg, &OutVar);

							AssignToExVal(OutVar, InterpVal);
						}

						ExecuteGLSL(ActiveProgram->FragmentShader);

						float OutR, OutG, OutB, OutA;

						for (int _i = 0;_i < ActiveProgram->FragmentShader.GlobalVars->Size;_i++)
						{
							glslVariable* Var;

							VectorRead(ActiveProgram->FragmentShader.GlobalVars, &Var, _i);

							if (Var->isOut)
							{
								OutR = ((float*)Var->Value.Data)[0];
								OutG = ((float*)Var->Value.Data)[1];
								OutB = ((float*)Var->Value.Data)[2];
								OutA = ((float*)Var->Value.Data)[3];
							}
						}

						OutR = MIN(MAX(OutR, 0.0f), 1.0f);
						OutG = MIN(MAX(OutG, 0.0f), 1.0f);
						OutB = MIN(MAX(OutB, 0.0f), 1.0f);
						OutA = MIN(MAX(OutA, 0.0f), 1.0f);

						uint32_t Color;

						if (GlobalFramebuffer->ColorFormat == GL_RGB)
						{
							Color = 0xFF;
							Color |= (int)(OutR * 255) << 24;
							Color |= (int)(OutG * 255) << 16;
							Color |= (int)(OutB * 255) << 8;
						}
						else if (GlobalFramebuffer->ColorFormat == GL_RGBA)
						{
							Color = 0x0;
							Color |= (int)(OutR * 255) << 24;
							Color |= (int)(OutG * 255) << 16;
							Color |= (int)(OutB * 255) << 8;
							Color |= (int)(OutA * 255);
						}

						*CurCol = Color;
					}
				}
			}
		}
	}
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (!ActiveVertexArray) return;
	if (!ActiveProgram) return;

	glslVariable* glPositionVar = 0;

	for (int i = 0;i < ActiveProgram->VertexShader.GlobalVars->Size;i++)
	{
		glslVariable* Var;
		
		VectorRead(ActiveProgram->VertexShader.GlobalVars, &Var, i);

		if (StringEquals(Var->Name, "gl_Position"))
		{
			glPositionVar = Var;
		}
	}

	VerifyVar(glPositionVar);

	if (mode == GL_POINTS)
	{
		for (int i = first; i < first + count; i++)
		{
			for (int j = 0;j < ActiveVertexArray->Attribs->Size;j++)
			{
				VertexArrayAttrib Attrib;
				
				VectorRead(ActiveVertexArray->Attribs, &Attrib, j);

				if (Attrib.type == GL_FLOAT)
				{
					float* AttribData = (float*)((uint8_t*)ActiveVertexArray->VertexBuffer->data + i * Attrib.stride + Attrib.offset);

					for (int k = 0;k < ActiveProgram->Layouts->Size;k++)
					{
						glslVariable* Var;

						VectorRead(ActiveProgram->Layouts, &Var, k);

						if (Var->Layout->Location == Attrib.index)
						{
							if (!Var->Value.Alloc)
							{
								Var->Value.Alloc = 1;
								Var->Value.Data = malloc(Attrib.size * sizeof(float));
							}
							memcpy(Var->Value.Data, AttribData, Attrib.size * sizeof(float));
						}
					}
				}
			}

			ExecuteGLSL(ActiveProgram->VertexShader);

			int OutPosX = ((float*)glPositionVar->Value.Data)[0] / ((float*)glPositionVar->Value.Data)[3] * (ViewportHeight / 2) + (ViewportWidth / 2) + ViewportX;
			int OutPosY = ((float*)glPositionVar->Value.Data)[1] / ((float*)glPositionVar->Value.Data)[3] * (ViewportHeight / 2) + (ViewportHeight / 2) + ViewportY;

			if (OutPosX < 0 || OutPosX >= GlobalFramebuffer->Width) continue;
			if (OutPosY < 0 || OutPosY >= GlobalFramebuffer->Height) continue;

			for (int j = 0;j < ActiveProgram->VertexFragInOut->Size;j++)
			{
				_Pair* InOut;

				VectorRead(ActiveProgram->VertexFragInOut, &InOut, j);

				glslVariable* InOutFirst, *InOutSecond;

				PairGetFirst(InOut, &InOutFirst);
				PairGetSecond(InOut, &InOutSecond);

				if (InOutFirst->Type != InOutSecond->Type)
				{
					continue;
				}

				if (InOutFirst->Type == GLSL_FLOAT) memcpy(InOutFirst->Value.Data, InOutSecond->Value.Data, sizeof(float));
				if (InOutFirst->Type == GLSL_VEC2) memcpy(InOutFirst->Value.Data, InOutSecond->Value.Data, sizeof(float) * 2);
				if (InOutFirst->Type == GLSL_VEC3) memcpy(InOutFirst->Value.Data, InOutSecond->Value.Data, sizeof(float) * 3);
				if (InOutFirst->Type == GLSL_VEC4) memcpy(InOutFirst->Value.Data, InOutSecond->Value.Data, sizeof(float) * 4);
				if (InOutFirst->Type == GLSL_INT) memcpy(InOutFirst->Value.Data, InOutSecond->Value.Data, sizeof(int));
			}

			ExecuteGLSL(ActiveProgram->FragmentShader);

			float OutR, OutG, OutB, OutA;

			for (int j = 0;j < ActiveProgram->FragmentShader.GlobalVars->Size;j++)
			{
				glslVariable* Var;
				VectorRead(ActiveProgram->FragmentShader.GlobalVars, &Var, j);
				if (Var->isOut)
				{
					OutR = ((float*)Var->Value.Data)[0];
					OutG = ((float*)Var->Value.Data)[1];
					OutB = ((float*)Var->Value.Data)[2];
					OutA = ((float*)Var->Value.Data)[3];
					break;
				}
			}

			OutR = MIN(MAX(OutR, 0.0f), 1.0f);
			OutG = MIN(MAX(OutG, 0.0f), 1.0f);
			OutB = MIN(MAX(OutB, 0.0f), 1.0f);
			OutA = MIN(MAX(OutA, 0.0f), 1.0f);

			if (GlobalFramebuffer->DepthAttachment)
			{
				if (GlobalFramebuffer->DepthFormat == GL_FLOAT)
				{
					float OutPosZ = ((float*)glPositionVar->Value.Data)[2];
					((float*)GlobalFramebuffer->DepthAttachment)[OutPosX + OutPosY * GlobalFramebuffer->Width] = OutPosZ;
				}
			}

			if (GlobalFramebuffer->ColorAttachment)
			{
				if (GlobalFramebuffer->ColorFormat == GL_RGB)
				{
					uint32_t Color = 0xFF;
					Color |= (int)(OutR * 255) << 24;
					Color |= (int)(OutG * 255) << 16;
					Color |= (int)(OutB * 255) << 8;
					GlobalFramebuffer->ColorAttachment[OutPosX + OutPosY * GlobalFramebuffer->Width] = Color;
				}
				else if (GlobalFramebuffer->ColorFormat == GL_RGBA)
				{
					uint32_t Color = 0;
					Color |= (int)(OutR * 255) << 24;
					Color |= (int)(OutG * 255) << 16;
					Color |= (int)(OutB * 255) << 8;
					Color |= (int)(OutA * 255);
					GlobalFramebuffer->ColorAttachment[OutPosX + OutPosY * GlobalFramebuffer->Width] = Color;
				}
			}
		}
	}
	else if (mode == GL_TRIANGLES)
	{
		for (int i = first; i < first + count; i += 3)
		{
			glslVec4 TriangleCoords[3];

			// ORIGINALLY DEFINED AS std::vector<std::pair<glslExValue, glslVariable*>> TriangleVertexData[3];
			_Vector* TriangleVertexData[3];

			for (int j = 0; j < 3; j++)
			{
				for (int k = 0;k < ActiveVertexArray->Attribs->Size;k++)
				{
					VertexArrayAttrib Attrib;
					VectorRead(ActiveVertexArray->Attribs, &Attrib, k);
					if (Attrib.type == GL_FLOAT)
					{
						float* AttribData = (float*)((uint8_t*)ActiveVertexArray->VertexBuffer->data + (i + j) * Attrib.stride + Attrib.offset);

						for (int k = 0;k < ActiveProgram->Layouts->Size;k++)
						{
							glslVariable* Var;
							VectorRead(ActiveProgram->Layouts, &Var, k);
							if (Var->Layout->Location == Attrib.index)
							{
								if (!Var->Value.Alloc)
								{
									Var->Value.Alloc = 1;
									Var->Value.Data = malloc(Attrib.size * sizeof(float));
								}
								memcpy(Var->Value.Data, AttribData, Attrib.size * sizeof(float));
							}
						}
					}
				}

				ExecuteGLSL(ActiveProgram->VertexShader);

				TriangleCoords[j].x = ((float*)glPositionVar->Value.Data)[0];
				TriangleCoords[j].y = ((float*)glPositionVar->Value.Data)[1];
				TriangleCoords[j].z = ((float*)glPositionVar->Value.Data)[2];
				TriangleCoords[j].w = ((float*)glPositionVar->Value.Data)[3];

				TriangleVertexData[j] = NewVector(sizeof(_Pair*));

				for (int k = 0;k < ActiveProgram->VertexFragInOut->Size;k++)
				{
					_Pair* InOut = 0;

					VectorRead(ActiveProgram->VertexFragInOut, &InOut, k);

					glslVariable* InOutFirst, *InOutSecond;

					PairGetFirst(InOut, &InOutFirst);
					PairGetSecond(InOut, &InOutSecond);

					if (InOutFirst->Type != InOutSecond->Type)
					{
						continue;
					}
					
					glslExValue OutExValue = VarToExVal(InOutSecond);

					_Pair* OutPair = MakePair(&OutExValue, sizeof(glslExValue), &InOutFirst, sizeof(glslVariable*));

					VectorPushBack(TriangleVertexData[j], &OutPair);
				}
			}

			Triangle MyTri;
			MyTri.Verts[0] = TriangleCoords[0];
			MyTri.Verts[1] = TriangleCoords[1];
			MyTri.Verts[2] = TriangleCoords[2];
			MyTri.TriangleVertexData[0] = TriangleVertexData[0];
			MyTri.TriangleVertexData[1] = TriangleVertexData[1];
			MyTri.TriangleVertexData[2] = TriangleVertexData[2];
			Triangle Triangles[2];
			int nTri = ClipTriangleAgainstNearPlane(&MyTri, Triangles);

			for (int k = 0;k < nTri;k++)
			{
				Triangle Tri = Triangles[k];
				for (int j = 0; j < 3; j++)
				{
					int OutPosX = Tri.Verts[j].x / Tri.Verts[j].w * (ViewportWidth / 2) + (ViewportWidth / 2) + ViewportX;
					int OutPosY = Tri.Verts[j].y / Tri.Verts[j].w * (ViewportHeight / 2) + (ViewportHeight / 2) + ViewportY;

					TriangleCoords[j].x = OutPosX;
					TriangleCoords[j].y = OutPosY;
					TriangleCoords[j].z = Tri.Verts[j].z;
					TriangleCoords[j].w = Tri.Verts[j].w;
				}

				DrawTriangle(TriangleCoords, Tri.TriangleVertexData);
			}
		}
	}
}

void glInit(GLsizei width, GLsizei height)
{
	GlobalFramebuffer = (Framebuffer*)malloc(sizeof(Framebuffer));
	GlobalFramebuffer->Width = width;
	GlobalFramebuffer->Height = height;
	
	GlobalFramebuffer->DepthFormat = GL_FLOAT;
	GlobalFramebuffer->DepthAttachment = malloc(sizeof(float) * width * height);

	GlobalFramebuffer->ColorFormat = GL_RGBA;
	GlobalFramebuffer->ColorAttachment = (uint32_t*)malloc(4 * width * height);

	GlobalArrayBuffer = 0;
	GlobalBuffers = NewVector(sizeof(Buffer*));
	GlobalPrograms = NewVector(sizeof(Program*));
	GlobalVertexArrays = NewVector(sizeof(VertexArray*));
	GlobalShaders = NewVector(sizeof(RawShader*));

	ActiveProgram = 0;
	ActiveVertexArray = 0;
}

uint32_t* glGetFramePtr()
{
	return GlobalFramebuffer->ColorAttachment;
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
	Program* MyProgram;
	
	VectorRead(GlobalPrograms, &MyProgram, program - 1);

	for (int i = 0;i < MyProgram->Uniforms->Size;i++)
	{
		glslVariable* Uniform;

		VectorRead(MyProgram->Uniforms, &Uniform, i);

		if (StringEquals(Uniform->Name, name))
		{
			return ((program - 1) << 16) | i;
		}
	}

	return -1;
}

void glUniform1f(GLint location, GLfloat v0)
{
	Program* MyProgram;
	
	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslExValue SetVal = { GLSL_FLOAT, v0 };

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslExValue SetVal = { GLSL_VEC2, v0, v1 };

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslExValue SetVal = { GLSL_VEC3, v0, v1, v2 };

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslExValue SetVal = { GLSL_VEC4, v0, v1, v2, v3 };

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform1i(GLint location, GLint v0)
{
	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslExValue SetVal = { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, v0 };

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	transpose = !transpose;

	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	if (!transpose) memcpy(MyUniform->Value.Data, value, sizeof(float) * 4);
	else
	{
		((float*)MyUniform->Value.Data)[0] = value[0];
		((float*)MyUniform->Value.Data)[1] = value[2];
		((float*)MyUniform->Value.Data)[2] = value[1];
		((float*)MyUniform->Value.Data)[3] = value[3];
	}
}

void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	transpose = !transpose;

	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	if (!transpose) memcpy(MyUniform->Value.Data, value, sizeof(float) * 9);
	else
	{
		((float*)MyUniform->Value.Data)[0] = value[0];
		((float*)MyUniform->Value.Data)[1] = value[3];
		((float*)MyUniform->Value.Data)[2] = value[6];
		((float*)MyUniform->Value.Data)[3] = value[1];
		((float*)MyUniform->Value.Data)[4] = value[4];
		((float*)MyUniform->Value.Data)[5] = value[7];
		((float*)MyUniform->Value.Data)[6] = value[2];
		((float*)MyUniform->Value.Data)[7] = value[5];
		((float*)MyUniform->Value.Data)[8] = value[8];
	}
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	transpose = !transpose;

	Program* MyProgram;

	VectorRead(GlobalPrograms, &MyProgram, location >> 16);

	glslVariable* MyUniform;
	VectorRead(MyProgram->Uniforms, &MyUniform, location & 0xFFFF);

	VerifyVar(MyUniform);

	if (!transpose) memcpy(MyUniform->Value.Data, value, sizeof(float) * 16);
	else
	{
		((float*)MyUniform->Value.Data)[0] = value[0];
		((float*)MyUniform->Value.Data)[1] = value[4];
		((float*)MyUniform->Value.Data)[2] = value[8];
		((float*)MyUniform->Value.Data)[3] = value[12];
		((float*)MyUniform->Value.Data)[4] = value[1];
		((float*)MyUniform->Value.Data)[5] = value[5];
		((float*)MyUniform->Value.Data)[6] = value[9];
		((float*)MyUniform->Value.Data)[7] = value[13];
		((float*)MyUniform->Value.Data)[8] = value[2];
		((float*)MyUniform->Value.Data)[9] = value[6];
		((float*)MyUniform->Value.Data)[10] = value[10];
		((float*)MyUniform->Value.Data)[11] = value[14];
		((float*)MyUniform->Value.Data)[12] = value[3];
		((float*)MyUniform->Value.Data)[13] = value[7];
		((float*)MyUniform->Value.Data)[14] = value[11];
		((float*)MyUniform->Value.Data)[15] = value[15];
	}
}

/*
* Every function below does not need to be implemented, 
* but is here for backwards compatibility
*/

void glEnableVertexAttribArray(GLuint index)
{

}
