#include "swgl.hpp"
#include <string>

#include <vector>

#include <fstream>
#include <sstream>

enum glslType
{
	GLSL_VEC3,
	GLSL_VEC2,
	GLSL_VEC4,
	GLSL_FLOAT,
	GLSL_INT,
	GLSL_MAT4,
	GLSL_MAT3,
	GLSL_MAT2,
	GLSL_UNKNOWN
};

enum glslTokenType
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
};

struct glslMat4
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
};

struct glslMat3
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
};

struct glslMat2
{
	float m00;
	float m01;

	float m10;
	float m11;
};

struct glslVec4
{
	float x;
	float y;
	float z;
	float w;
};

struct glslVec3
{
	float x;
	float y;
	float z;
};

struct glslVec2
{
	float x;
	float y;
};

struct glslExValue
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
};

struct glslVariable;

struct Triangle
{
	glslVec4 Verts[3];
	std::vector<std::pair<glslExValue, glslVariable*>> TriangleVertexData[3];
};

glslVec4 IntersectNearPlane(glslVec4 a, glslVec4 b, float &t)
{
	t = (a.w + a.z) / ((a.w + a.z) - (b.w + b.z));

	return { a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z), a.w + t * (b.w - a.w) };
}

glslExValue InterpolateExValue(glslExValue Val0, glslExValue Val1, float t)
{
	if (Val0.Type == GLSL_FLOAT)
	{
		return { GLSL_FLOAT, Val0.x + t * (Val1.x - Val0.x) };
	}
	if (Val0.Type == GLSL_VEC2)
	{
		return { GLSL_VEC2, Val0.x + t * (Val1.x - Val0.x), Val0.y + t * (Val1.y - Val0.y) };
	}
	if (Val0.Type == GLSL_VEC3)
	{
		return { GLSL_VEC3, Val0.x + t * (Val1.x - Val0.x), Val0.y + t * (Val1.y - Val0.y), Val0.z + t * (Val1.z - Val0.z) };
	}
	if (Val0.Type == GLSL_VEC4)
	{
		return { GLSL_VEC4, Val0.x + t * (Val1.x - Val0.x), Val0.y + t * (Val1.y - Val0.y), Val0.z + t * (Val1.z - Val0.z), Val0.w + t * (Val1.w - Val0.w) };
	}
	if (Val0.Type == GLSL_INT)
	{
		return { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, (int)(Val0.i + t * (Val1.i - Val0.i)) };
	}
}

int ClipTriangleAgainstNearPlane(Triangle& tri, Triangle *outTri) 
{
	outTri[0] = tri;
	outTri[1] = tri;

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	glslVec4* InsidePoints[3];  int nInsidePointCount = 0;
	std::vector<std::pair<glslExValue, glslVariable*>> InExValues[3];
	glslVec4* OutsidePoints[3]; int nOutsidePointCount = 0;
	std::vector<std::pair<glslExValue, glslVariable*>> OutExValues[3];

	if (tri.Verts[0].z >= -tri.Verts[0].w)
	{
		InExValues[nInsidePointCount] = tri.TriangleVertexData[0];
		InsidePoints[nInsidePointCount++] = &tri.Verts[0];
	}
	else 
	{
		OutExValues[nOutsidePointCount] = tri.TriangleVertexData[0];
		OutsidePoints[nOutsidePointCount++] = &tri.Verts[0];
	}
	if (tri.Verts[1].z >= -tri.Verts[1].w)
	{
		InExValues[nInsidePointCount] = tri.TriangleVertexData[1];
		InsidePoints[nInsidePointCount++] = &tri.Verts[1];
	}
	else 
	{
		OutExValues[nOutsidePointCount] = tri.TriangleVertexData[1];
		OutsidePoints[nOutsidePointCount++] = &tri.Verts[1];
	}
	if (tri.Verts[2].z >= -tri.Verts[2].w)
	{
		InExValues[nInsidePointCount] = tri.TriangleVertexData[2];
		InsidePoints[nInsidePointCount++] = &tri.Verts[2];
	}
	else 
	{
		OutExValues[nOutsidePointCount] = tri.TriangleVertexData[2];
		OutsidePoints[nOutsidePointCount++] = &tri.Verts[2];
	}

	// Now classify triangle points, and break the input triangle into 
	// smaller output triangles if required. There are four possible
	// outcomes...

	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole triangle
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		outTri[0] = tri;

		return 1; // Just the one returned original triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the triangle simply becomes a smaller triangle

		// The inside point is valid, so keep that...
		outTri[0].Verts[0] = *InsidePoints[0];
		outTri[0].TriangleVertexData[0] = InExValues[0];

		// but the two new points are at the locations where the 
		// original sides of the triangle (lines) intersect with the plane
		float t;
		outTri[0].Verts[1] = IntersectNearPlane(*InsidePoints[0], *OutsidePoints[0], t);
		for (int i = 0; i < tri.TriangleVertexData[1].size(); i++)
		{
			outTri[0].TriangleVertexData[1][i].first = InterpolateExValue(InExValues[0][i].first, OutExValues[0][i].first, t);
		}

		outTri[0].Verts[2] = IntersectNearPlane(*InsidePoints[0], *OutsidePoints[1], t);
		for (int i = 0; i < tri.TriangleVertexData[2].size(); i++)
		{
			outTri[0].TriangleVertexData[2][i].first = InterpolateExValue(InExValues[0][i].first, OutExValues[1][i].first, t);
		}

		return 1; // Return the newly formed single triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		outTri[0].Verts[0] = *InsidePoints[0];
		outTri[0].TriangleVertexData[0] = InExValues[0];
		outTri[0].Verts[1] = *InsidePoints[1];
		outTri[0].TriangleVertexData[1] = InExValues[1];

		float t;
		outTri[0].Verts[2] = IntersectNearPlane(*InsidePoints[0], *OutsidePoints[0], t);
		for (int i = 0; i < tri.TriangleVertexData[2].size(); i++)
		{
			outTri[0].TriangleVertexData[2][i].first = InterpolateExValue(InExValues[0][i].first, OutExValues[0][i].first, t);
		}

		// The second triangle is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// triangle and the plane, and the newly created point above
		outTri[1].Verts[0] = *InsidePoints[1];
		outTri[1].TriangleVertexData[0] = InExValues[1];
		outTri[1].Verts[1] = outTri[0].Verts[2];
		outTri[1].TriangleVertexData[1] = outTri[0].TriangleVertexData[2];
		outTri[1].Verts[2] = IntersectNearPlane(*InsidePoints[1], *OutsidePoints[0], t);
		for (int i = 0; i < tri.TriangleVertexData[2].size(); i++)
		{
			outTri[1].TriangleVertexData[2][i].first = InterpolateExValue(InExValues[1][i].first, OutExValues[0][i].first, t);
		}

		return 2; // Return two newly formed triangles which form a quad
	}
}

glslMat4 MatMul(const glslMat4& a, const glslMat4& b) 
{
	glslMat4 result;

	result.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30;
	result.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31;
	result.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32;
	result.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33;

	result.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30;
	result.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
	result.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
	result.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;

	result.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30;
	result.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
	result.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
	result.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;

	result.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30;
	result.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;
	result.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;
	result.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;

	return result;
}

glslMat3 MatMul(const glslMat3& a, const glslMat3& b) 
{
	glslMat3 result;

	result.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20;
	result.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21;
	result.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22;

	result.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20;
	result.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21;
	result.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22;

	result.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20;
	result.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21;
	result.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22;

	return result;
}

glslMat2 MatMul(const glslMat2& a, const glslMat2& b) 
{
	glslMat2 result;

	result.m00 = a.m00 * b.m00 + a.m01 * b.m10;
	result.m01 = a.m00 * b.m01 + a.m01 * b.m11;

	result.m10 = a.m10 * b.m00 + a.m11 * b.m10;
	result.m11 = a.m10 * b.m01 + a.m11 * b.m11;

	return result;
}

glslVec4 MatMul(const glslMat4& mat, const glslVec4& vec) 
{
	glslVec4 result;

	result.x = mat.m00 * vec.x + mat.m01 * vec.y + mat.m02 * vec.z + mat.m03 * vec.w;
	result.y = mat.m10 * vec.x + mat.m11 * vec.y + mat.m12 * vec.z + mat.m13 * vec.w;
	result.z = mat.m20 * vec.x + mat.m21 * vec.y + mat.m22 * vec.z + mat.m23 * vec.w;
	result.w = mat.m30 * vec.x + mat.m31 * vec.y + mat.m32 * vec.z + mat.m33 * vec.w;

	return result;
}

glslVec3 MatMul(const glslMat3& mat, const glslVec3& vec) 
{
	glslVec3 result;

	result.x = mat.m00 * vec.x + mat.m01 * vec.y + mat.m02 * vec.z;
	result.y = mat.m10 * vec.x + mat.m11 * vec.y + mat.m12 * vec.z;
	result.z = mat.m20 * vec.x + mat.m21 * vec.y + mat.m22 * vec.z;

	return result;
}

glslVec2 MatMul(const glslMat2& mat, const glslVec2& vec) 
{
	glslVec2 result;

	result.x = mat.m00 * vec.x + mat.m01 * vec.y;
	result.y = mat.m10 * vec.x + mat.m11 * vec.y;

	return result;
}

glslType GetTypeFromStr(std::string Type)
{
	if (Type == "vec2") return GLSL_VEC2;
	if (Type == "vec3") return GLSL_VEC3;
	if (Type == "vec4") return GLSL_VEC4;
	if (Type == "float") return GLSL_FLOAT;
	if (Type == "int") return GLSL_INT;
	if (Type == "mat2") return GLSL_MAT2;
	if (Type == "mat3") return GLSL_MAT3;
	if (Type == "mat4") return GLSL_MAT4;
	return GLSL_UNKNOWN;
}

struct glslConst
{
	bool IsFloat;

	float Fval;
	int Ival;
};

struct glslLayout
{
	int Location;
};

struct glslValue
{
	void* Data;
	bool Alloc = false;
};

struct glslVariable
{
	std::string Name;
	glslType Type;

	bool isUniform = false;
	bool isLayout = false;

	bool isIn = false;
	bool isOut = false;

	glslLayout* Layout;

	glslValue Value;
};

struct glslToken
{
	glslTokenType Type;

	glslToken* First;
	glslToken* Second;
	glslToken* Third;

	glslConst Const;
	glslVariable* Var;
	
	std::vector<int> Swizzle;

	std::vector<glslToken*> Args;
};

struct glslScope
{
	std::vector<glslVariable*> Variables;
	glslScope* ParentScope = 0;
	std::vector<glslToken*> Lines;
};

struct glslFunction
{
	glslType ReturnType;

	std::string Name;
	glslScope* RootScope;
	int ParamCount;
};

struct glslTokenized
{
	std::vector<glslFunction*> Funcs;
	std::vector<glslVariable*> GlobalVars;
};

std::vector<char> glslOpChars = { '+', '-', '*', '/', '>', '<', '=' };

class glslTokenizer
{
private:
	int At = 0;
	std::string Code;
	std::vector<glslVariable*> GlobalVars;
public:
	int TellNext(char c)
	{
		for (int i = At; i < Code.size(); i++)
		{
			if (Code[i] == c)
			{
				return i;
			}
		}
		return 0x7FFFFFFF;
	}
	int TellNextWithEnd(char c, int end)
	{
		for (int i = At; i < end; i++)
		{
			if (Code[i] == c)
			{
				return i;
			}
		}
		return 0x7FFFFFFF;
	}
	int TellNextMatching(char Inc, char Dec)
	{
		int counter = 0;
		for (int i = At; i < Code.size(); i++)
		{
			if (Code[i] == Inc) counter++;
			if (Code[i] == Dec)
			{
				counter--;
				if (counter == 0) return i;
			}
		}
		return 0x7FFFFFFF;
	}
	int TellNextArgStart()
	{
		int counter = 0;
		for (int i = At; i < Code.size(); i++)
		{
			if (Code[i] == '(') counter++;
			if (Code[i] == ')') counter--;
			if (counter == 0 && Code[i] == ',') return i;
		}
		return 0x7FFFFFFF;
	}
	bool IsOpChar(char c)
	{
		for (char op : glslOpChars)
		{
			if (op == c) return true;
		}
		return false;
	}
	std::pair<int, int> TellNextOperator(glslTokenType *OutOp)
	{
		int counter = 0;
		for (int i = At; i < Code.size(); i++)
		{
			if (counter == 0)
			{
				std::string Operator;
				if (IsOpChar(Code[i]))
				{
					Operator.push_back(Code[i]);
					if (IsOpChar(Code[i + 1]))
					{
						Operator.push_back(Code[i + 1]);
					}

					if (Operator == "+")
					{
						*OutOp = GLSL_TOK_ADD;
						return { i, i + 1 };
					}

					if (Operator == "-")
					{
						*OutOp = GLSL_TOK_SUB;
						return { i, i + 1 };
					}

					if (Operator == "*")
					{
						*OutOp = GLSL_TOK_MUL;
						return { i, i + 1 };
					}

					if (Operator == "/")
					{
						*OutOp = GLSL_TOK_DIV;
						return { i, i + 1 };
					}

					if (Operator == "=")
					{
						*OutOp = GLSL_TOK_ASSIGN;
						return { i, i + 1 };
					}

					if (Operator == "<")
					{
						*OutOp = GLSL_TOK_LT;
						return { i, i + 1 };
					}

					if (Operator == ">")
					{
						*OutOp = GLSL_TOK_GT;
						return { i, i + 1 };
					}

					if (Operator == "==")
					{
						*OutOp = GLSL_TOK_EQ;
						return { i, i + 2 };
					}
				}
			}
			if (Code[i] == '(') counter++;
			if (Code[i] == ')') counter--;
		}
		return { 0x7FFFFFFF, 0x7FFFFFFF };
	}
	bool FindVariableInScope(glslScope* Scope, std::string Name, glslVariable** Out)
	{
		for (glslVariable* Var : Scope->Variables)
		{
			if (Var->Name == Name)
			{
				*Out = Var;
				return true;
			}
		}
		if (Scope->ParentScope) return FindVariableInScope(Scope->ParentScope, Name, Out);
		return false;
	}
	bool FindVariable(glslScope* Scope, std::string Name, glslVariable** Out)
	{
		for (glslVariable* Var : GlobalVars)
		{
			if (Var->Name == Name)
			{
				*Out = Var;
				return true;
			}
		}
		return FindVariableInScope(Scope, Name, Out);
	}
	bool IsDigit(char c)
	{
		return c >= '0' && c <= '9';
	}
	std::string TellStringUntil(int idx)
	{
		if (idx == 0x7FFFFFFF) return "ERROR";

		std::string Out;
		for (int i = At; i < idx; i++)
		{
			Out.push_back(Code[i]);
		}
		return Out;
	}
	std::string TellStringUntilNWS(int idx)
	{
		if (idx == 0x7FFFFFFF) return "ERROR";

		std::string Out;
		for (int i = At; i < idx; i++)
		{
			if (Code[i] == ' ') return Out;
			Out.push_back(Code[i]);
		}
		return Out;
	}
	glslToken* TokenizeArgs(glslScope* Scope, int EndAt)
	{
		glslToken* Tok = new glslToken();

		while (At < EndAt)
		{
			while (Code[At] == ' ') At++;
			int NextArgStart = std::min(EndAt, TellNextArgStart());
			Tok->Args.push_back(TokenizeExpr(Scope, NextArgStart));

			if (NextArgStart == EndAt)
			{
				break;
			}

			At = NextArgStart + 1;

			while (Code[At] == ' ') At++;
		}

		At = EndAt + 1;

		return Tok;
	}

	glslToken* TokenizeSubExpr(glslScope* Scope, int EndAt)
	{
		while (Code[At] == ' ') At++;

		if (Code[At] == '(')
		{
			int MatchingParam = TellNextMatching('(', ')');
			At++;
			glslToken* EnclosedTok = TokenizeExpr(Scope, MatchingParam);
			
			int Swizzle = -1;
			for (int i = At; i < EndAt; i++)
			{
				if (Code[i] == ' ') break;
				if (Code[i] == '.')
				{
					Swizzle = i + 1;
					break;
				}
			}

			if (Swizzle != -1)
			{
				glslToken* SwizzleTok = new glslToken;
				SwizzleTok->Type = GLSL_TOK_SWIZZLE;
				SwizzleTok->First = EnclosedTok;
				for (int i = Swizzle; i < EndAt; i++)
				{
					char SwizzleChar = Code[i];

					if (SwizzleChar == 'x' || SwizzleChar == 's')
					{
						SwizzleTok->Swizzle.push_back(0);
					}
					else if (SwizzleChar == 'y' || SwizzleChar == 't')
					{
						SwizzleTok->Swizzle.push_back(1);
					}
					else if (SwizzleChar == 'z')
					{
						SwizzleTok->Swizzle.push_back(2);
					}
					else if (SwizzleChar == 'w')
					{
						SwizzleTok->Swizzle.push_back(3);
					}
					else if (SwizzleChar == ' ')
					{
						break;
					}
					else
					{
						printf("SWGL: FATAL! GLSL compiler detected invalid swizzle component '%c'\n", SwizzleChar);
						return 0;
					}
				}
				EnclosedTok = SwizzleTok;
			}

			At = EndAt + 1;
			return EnclosedTok;
		}

		int NextBeginParen = TellNext('(');
		std::string ParenStr = TellStringUntilNWS(NextBeginParen);

		glslType TypeFromStr = GetTypeFromStr(ParenStr);

		if (TypeFromStr != GLSL_UNKNOWN)
		{
			At = NextBeginParen;

			int NextCloseParen = TellNextMatching('(', ')');

			At++;
			
			glslToken* OutTok = TokenizeArgs(Scope, NextCloseParen);
			
			if (TypeFromStr == GLSL_FLOAT)
			{
				if (OutTok->Args.size() != 1)
				{
					printf("SWGL: FATAL! GLSL compiler expects float constructors to have only 1 argument!\n");
					return 0;
				}
				OutTok->Type = GLSL_TOK_FLOAT_CONSTRUCT;
			}
			else if (TypeFromStr == GLSL_VEC2)
			{
				if (OutTok->Args.size() != 2)
				{
					printf("SWGL: FATAL! GLSL compiler expects vec2 constructors to have only 2 arguments!\n");
					return 0;
				}
				OutTok->Type = GLSL_TOK_VEC2_CONSTRUCT;
			}
			else if (TypeFromStr == GLSL_VEC3)
			{
				if (OutTok->Args.size() != 3)
				{
					printf("SWGL: FATAL! GLSL compiler expects float constructors to have only 3 arguments!\n");
					return 0;
				}
				OutTok->Type = GLSL_TOK_VEC3_CONSTRUCT;
			}
			else if (TypeFromStr == GLSL_VEC4)
			{
				if (OutTok->Args.size() != 4)
				{
					printf("SWGL: FATAL! GLSL compiler expects vec4 constructors to have only 4 arguments!\n");
					return 0;
				}
				OutTok->Type = GLSL_TOK_VEC4_CONSTRUCT;
			}
			else if (TypeFromStr == GLSL_INT)
			{
				if (OutTok->Args.size() != 1)
				{
					printf("SWGL: FATAL! GLSL compiler expects int constructors to have only 1 argument!\n");
					return 0;
				}
				OutTok->Type = GLSL_TOK_INT_CONSTRUCT;
			}
			return OutTok;
		}

		if (IsDigit(Code[At]))
		{
			glslToken* ConstTok = new glslToken;
			ConstTok->Type = GLSL_TOK_CONST;

			bool IsFloat = false;
			for (int i = At; i < EndAt; i++)
			{
				if (Code[i] == '.')
				{
					IsFloat = true;
					break;
				}
			}

			std::string NumStr = TellStringUntilNWS(EndAt);

			if (IsFloat)
			{
				ConstTok->Const.IsFloat = true;
				ConstTok->Const.Fval = atof(NumStr.c_str());
			}
			else
			{
				ConstTok->Const.IsFloat = false;
				ConstTok->Const.Ival = atoi(NumStr.c_str());
			}
			At = EndAt + 1;
			return ConstTok;
		}
		else
		{
			std::string ProbeVarName;
			int Swizzle = -1;
			for (int i = At; i < EndAt; i++)
			{
				if (Code[i] == ' ') break;
				if (Code[i] == '.')
				{
					Swizzle = i + 1;
					break;
				}
				ProbeVarName.push_back(Code[i]);
			}

			glslVariable* ProbeVar;
			if (FindVariable(Scope, ProbeVarName, &ProbeVar))
			{
				glslToken* VarTok = new glslToken;
				
				VarTok->Type = GLSL_TOK_VAR;
				VarTok->Var = ProbeVar;

				if (Swizzle != -1)
				{
					glslToken* SwizzleTok = new glslToken;
					SwizzleTok->Type = GLSL_TOK_SWIZZLE;
					SwizzleTok->First = VarTok;
					for (int i = Swizzle; i < EndAt; i++)
					{
						char SwizzleChar = Code[i];

						if (SwizzleChar == 'x' || SwizzleChar == 's')
						{
							SwizzleTok->Swizzle.push_back(0);
						}
						else if (SwizzleChar == 'y' || SwizzleChar == 't')
						{
							SwizzleTok->Swizzle.push_back(1);
						}
						else if (SwizzleChar == 'z')
						{
							SwizzleTok->Swizzle.push_back(2);
						}
						else if (SwizzleChar == 'w')
						{
							SwizzleTok->Swizzle.push_back(3);
						}
						else if (SwizzleChar == ' ')
						{
							break;
						}
						else
						{
							printf("SWGL: FATAL! GLSL compiler detected invalid swizzle component '%c'\n", SwizzleChar);
							return 0;
						}
					}
					VarTok = SwizzleTok;
				}

				At = EndAt + 1;
				return VarTok;
			}
			else
			{
				printf("SWGL: FATAL! GLSL compiler couldn't find the symbol '%s'!\n", ProbeVarName.c_str());
				return 0;
			}
		}
	}

	glslToken* TokenizeExpr(glslScope* Scope, int EndAt)
	{
		glslToken* Tok = new glslToken;

		while (Code[At] == ' ') At++;

		if (At == EndAt)
		{
			printf("SWGL: FATAL! GLSL compiler doesn't support empty expressions!\n");
			return 0;
		}

		glslTokenType OpType;
		std::pair<int, int> NextOp = TellNextOperator(&OpType);

		Tok = TokenizeSubExpr(Scope, std::min(NextOp.first, EndAt));

		while (At < EndAt && NextOp.first != 0x7FFFFFFF)
		{
			glslToken* SurroundToken = new glslToken;
			SurroundToken->Type = OpType;
			SurroundToken->First = Tok;

			At = NextOp.second;

			NextOp = TellNextOperator(&OpType);

			SurroundToken->Second = TokenizeSubExpr(Scope, std::min(NextOp.first, EndAt));

			Tok = SurroundToken;

		}
		
		At = EndAt + 1;

		return Tok;
	}

	glslToken* TokenizeLine(glslScope* Scope)
	{
		while (Code[At] == ' ') At++;
		int NextBlank = TellNext(' ');
		int NextSemi = TellNext(';');
		glslTokenType OpType;
		std::pair<int, int> NextOperator = TellNextOperator(&OpType);

		bool Uninitialized = false;
		if (NextOperator.first >= NextSemi)
		{
			Uninitialized = true;
		}

		glslType DeclType = GetTypeFromStr(TellStringUntil(NextBlank));

		if (DeclType != GLSL_UNKNOWN)
		{
			if (OpType != GLSL_TOK_ASSIGN && !Uninitialized)
			{
				printf("SWGL: FATAL! Variable declaration must have = be the first operator!\n");
				return 0;
			}
		}
		else
		{
			return TokenizeExpr(Scope, NextSemi);
		}

		glslToken* OutToken = new glslToken;
		OutToken->Type = Uninitialized ? GLSL_TOK_VAR : GLSL_TOK_VAR_DECL;


		if (DeclType == GLSL_UNKNOWN)
		{
			printf("SWGL: FATAL! GLSL compiler encountered unknown type!\n");
			return 0;
		}

		At = NextBlank + 1;
		NextBlank = TellNext(' ');

		std::string DeclName = TellStringUntilNWS(std::min(NextOperator.first, NextSemi));

		glslVariable* DeclVar = new glslVariable;

		DeclVar->Name = DeclName;
		DeclVar->Type = DeclType;
		Scope->Variables.push_back(DeclVar);

		if (NextOperator.first > NextSemi)
		{
			At = NextSemi + 1;
			return 0;
		}

		OutToken->Var = DeclVar;

		At = NextOperator.second;
		OutToken->Second = TokenizeExpr(Scope, NextSemi);

		return OutToken;
	}

	glslFunction* TokenizeFunction()
	{
		while (Code[At] == ' ') At++;

		glslFunction* MyFunc = new glslFunction;

		int NextBlank = TellNext(' ');
		std::string ReturnTypeName = TellStringUntil(NextBlank);
		MyFunc->ReturnType = GetTypeFromStr(ReturnTypeName);

		At = NextBlank + 1;

		while (Code[At] == ' ') At++;

		int NextParen = TellNext('(');
		int NextClosingParen = TellNext(')');
		MyFunc->Name = TellStringUntil(NextParen);

		MyFunc->RootScope = new glslScope();
		MyFunc->ParamCount = 0;

		At = NextParen + 1;

		while (At < NextClosingParen)
		{
			while (Code[At] == ' ') At++;

			int NextBlank = TellNext(' ');

			std::string ParamTypeName = TellStringUntil(NextBlank);
			glslType ParamType = GetTypeFromStr(ParamTypeName);

			if (ParamType == GLSL_UNKNOWN)
			{
				printf("SWGL: FATAL! GLSL compiler found unrecognized type in function parameters!");
				return 0;
			}

			At = NextBlank + 1;

			while (Code[At] == ' ') At++;

			int NextParamEnd = std::min(TellNext(','), NextClosingParen);
			std::string ParamName = TellStringUntilNWS(NextParamEnd);

			glslVariable* Param = new glslVariable;
			Param->Type = ParamType;
			Param->Name = ParamName;
			MyFunc->RootScope->Variables.push_back(Param);
			MyFunc->ParamCount++;

			At = NextParamEnd + 1;
		}

		At = NextClosingParen + 1;

		while (Code[At] == ' ') At++;

		if (Code[At] != '{')
		{
			printf("SWGL: FATAL! GLSL compiler expected '{' to start code block\n");
			return 0;
		}

		int NextCodeBlockEnd = TellNextMatching('{', '}');

		At++;

		while (At < NextCodeBlockEnd)
		{
			MyFunc->RootScope->Lines.push_back(TokenizeLine(MyFunc->RootScope));
		}

		At = NextCodeBlockEnd + 1;

		while (Code[At] == ' ') At++;

		return MyFunc;
	}

	void TokenizeUniform()
	{
		while (Code[At] == ' ') At++;

		int NextSemi = TellNext(';');
		int NextBlank = TellNext(' ');
	
		if (NextBlank > NextSemi || NextBlank == 0x7FFFFFFF)
		{
			printf("SWGL: FATAL! GLSL compiler expected uniform declaration to have a whitespace seperating type and uniform name!\n");
			return;
		}

		std::string UniformTypeName = TellStringUntil(NextBlank);
		glslType UniformType = GetTypeFromStr(UniformTypeName);

		if (UniformType == GLSL_UNKNOWN)
		{
			printf("SWGL: FATAL! GLSL compiler encountered unrecognized type '%s' during uniform declaration!\n", UniformTypeName.c_str());
			return;
		}

		At = NextBlank + 1;

		while (Code[At] == ' ') At++;

		std::string UniformName = TellStringUntilNWS(NextSemi);

		glslVariable* Uniform = new glslVariable;

		Uniform->Type = UniformType;
		Uniform->Name = UniformName;

		Uniform->isUniform = true;

		At = NextSemi + 1;

		GlobalVars.push_back(Uniform);
	}

	void TokenizeOut()
	{
		while (Code[At] == ' ') At++;

		int NextSemi = TellNext(';');
		int NextBlank = TellNext(' ');

		if (NextBlank > NextSemi || NextBlank == 0x7FFFFFFF)
		{
			printf("SWGL: FATAL! GLSL compiler expected OUT declaration to have a whitespace seperating type and uniform name!\n");
			return;
		}

		std::string OutTypeName = TellStringUntil(NextBlank);
		glslType OutType = GetTypeFromStr(OutTypeName);

		if (OutType == GLSL_UNKNOWN)
		{
			printf("SWGL: FATAL! GLSL compiler encountered unrecognized type '%s' during OUT declaration!\n", OutTypeName.c_str());
			return;
		}

		At = NextBlank + 1;

		while (Code[At] == ' ') At++;

		std::string OutName = TellStringUntilNWS(NextSemi);

		glslVariable* Out = new glslVariable;

		Out->Type = OutType;
		Out->Name = OutName;

		Out->isOut = true;

		At = NextSemi + 1;

		GlobalVars.push_back(Out);
	}

	void TokenizeIn()
	{
		while (Code[At] == ' ') At++;

		int NextSemi = TellNext(';');
		int NextBlank = TellNext(' ');

		if (NextBlank > NextSemi || NextBlank == 0x7FFFFFFF)
		{
			printf("SWGL: FATAL! GLSL compiler expected IN declaration to have a whitespace seperating type and uniform name!\n");
			return;
		}

		std::string InTypeName = TellStringUntil(NextBlank);
		glslType InType = GetTypeFromStr(InTypeName);

		if (InType == GLSL_UNKNOWN)
		{
			printf("SWGL: FATAL! GLSL compiler encountered unrecognized type '%s' during IN declaration!\n", InTypeName.c_str());
			return;
		}

		At = NextBlank + 1;

		while (Code[At] == ' ') At++;

		std::string InName = TellStringUntilNWS(NextSemi);

		glslVariable* In = new glslVariable;

		In->Type = InType;
		In->Name = InName;

		In->isIn = true;

		At = NextSemi + 1;

		GlobalVars.push_back(In);
	}

	void TokenizeLayout()
	{
		// For now, only accepts location.

		int NextSemi = TellNext(';');

		int NextEq = TellNext('=');
		int NextParen = TellNext(')');

		if (NextParen > NextSemi)
		{
			printf("SWGL: FATAL! GLSL compiler expected enclosing ')' in layout!\n");
			return;
		}

		if (NextEq > NextParen)
		{
			printf("SWGL: FATAL! GLSL compiler doesn't support empty layout!\n");
			return;
		}

		std::string ParamName = TellStringUntilNWS(NextEq);

		if (ParamName != "location")
		{
			printf("SWGL: FATAL! GLSL compiler found unknown parameter '%s' to layout!\n", ParamName.c_str());
			return;
		}

		At = NextEq + 1;

		while (Code[At] == ' ') At++;

		if (!IsDigit(Code[At]))
		{
			printf("SWGL: FATAL! GLSL compiler expects a number in location in layout!\n");
			return;
		}

		std::string ParamValStr = TellStringUntilNWS(NextParen);
		
		int ParamVal = std::atoi(ParamValStr.c_str());

		At = NextParen + 1;

		while (Code[At] == ' ') At++;

		int NextBlank = TellNext(' ');

		if (NextBlank > NextSemi)
		{
			printf("SWGL: FATAL! GLSL compiler expected at least 1 whitespace after layout!");
			return;
		}

		std::string TypeName = TellStringUntilNWS(NextBlank);
		glslType Type = GetTypeFromStr(TypeName);

		if (Type == GLSL_UNKNOWN)
		{
			printf("SWGL: FATAL! GLSL compiler found unknown type '%s' after layout!\n", TypeName.c_str());
			return;
		}

		At = NextBlank + 1;

		while (Code[At] == ' ') At++;

		std::string Name = TellStringUntilNWS(NextSemi);

		glslVariable* Variable = new glslVariable;
		Variable->Type = Type;
		Variable->Name = Name;
		Variable->isLayout = true;
		Variable->Layout = new glslLayout;
		Variable->Layout->Location = ParamVal;
		GlobalVars.push_back(Variable);

		At = NextSemi + 1;
	}

	glslFunction* DispatchTokenize()
	{
		while (Code[At] == ' ') At++;
		
		int NextBlank = TellNext(' ');
		int NextParen = TellNext('(');

		std::string BlankStr = TellStringUntilNWS(NextBlank);
		std::string ParenStr = TellStringUntilNWS(NextParen);

		if (BlankStr == "uniform")
		{
			At = NextBlank + 1;
			TokenizeUniform();
			return 0;
		}

		if (BlankStr == "in")
		{
			At = NextBlank + 1;
			TokenizeIn();
			return 0;
		}

		if (BlankStr == "out")
		{
			At = NextBlank + 1;
			TokenizeOut();
			return 0;
		}

		if (ParenStr == "layout")
		{
			At = NextParen + 1;
			TokenizeLayout();
			return 0;
		}

		return TokenizeFunction();
	}

	glslTokenized Tokenize(std::string ToTokenize)
	{
		Code = ToTokenize;
		std::vector<glslFunction*> OutFuncs;

		Code.erase(std::remove(Code.begin(), Code.end(), '\n'), Code.end());
		Code.erase(std::remove(Code.begin(), Code.end(), '\t'), Code.end());

		glslVariable* PositionVariable = new glslVariable;
		PositionVariable->Name = "gl_Position";
		PositionVariable->Type = GLSL_VEC4;
		GlobalVars.push_back(PositionVariable);

		while (At < Code.size() - 2)
		{
			glslFunction* DispatchResult = DispatchTokenize();
			if (DispatchResult) OutFuncs.push_back(DispatchResult);
		}

		return { OutFuncs, GlobalVars };
	}
};

struct RawShader
{
	GLenum Type;

	std::string MyCode;
	bool Compiled;
	glslTokenized CompiledData;
};

std::vector<RawShader*> GlobalShaders;

void VerifyVar(glslVariable* Var)
{
	if (Var->Value.Alloc) return;

	if (Var->Type == GLSL_FLOAT) Var->Value.Data = malloc(sizeof(float));
	else if (Var->Type == GLSL_VEC2) Var->Value.Data = malloc(sizeof(float) * 2);
	else if (Var->Type == GLSL_VEC3) Var->Value.Data = malloc(sizeof(float) * 3);
	else if (Var->Type == GLSL_VEC4) Var->Value.Data = malloc(sizeof(float) * 4);
	else if (Var->Type == GLSL_INT) Var->Value.Data = malloc(sizeof(int));
	else if (Var->Type == GLSL_MAT2) Var->Value.Data = malloc(sizeof(float) * 4);
	else if (Var->Type == GLSL_MAT3) Var->Value.Data = malloc(sizeof(float) * 9);
	else if (Var->Type == GLSL_MAT4) Var->Value.Data = malloc(sizeof(float) * 16);
	Var->Value.Alloc = true;
}

void AssignToExVal(glslVariable* AssignTo, glslExValue Val)
{
	VerifyVar(AssignTo);

	if (AssignTo->Type != Val.Type)
	{
		printf("SWGL: ERROR! Type mismatch while executing shader! Type 0: %i Type 1: %i\n", (int)AssignTo->Type, (int)Val.Type);
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

		if (Token->Var->Type == GLSL_FLOAT) return { GLSL_FLOAT, ((float*)Token->Var->Value.Data)[0] };
		else if (Token->Var->Type == GLSL_VEC2) return { GLSL_VEC2, ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1] };
		else if (Token->Var->Type == GLSL_VEC3) return { GLSL_VEC3, ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2] };
		else if (Token->Var->Type == GLSL_VEC4) return { GLSL_VEC4, ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2], ((float*)Token->Var->Value.Data)[3] };
		else if (Token->Var->Type == GLSL_INT) return { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, ((int*)Token->Var->Value.Data)[0] };
		else
		{
			if (Token->Var->Type == GLSL_MAT2) return { GLSL_MAT2, 0.0f, 0.0f, 0.0f, 0.0f, 0, 
				{ ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1],
				  ((float*)Token->Var->Value.Data)[2], ((float*)Token->Var->Value.Data)[3] }
			};
			if (Token->Var->Type == GLSL_MAT3) return { GLSL_MAT3, 0.0f, 0.0f, 0.0f, 0.0f, 0, {}, 
				{ ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2],
				  ((float*)Token->Var->Value.Data)[3], ((float*)Token->Var->Value.Data)[4], ((float*)Token->Var->Value.Data)[5], 
				  ((float*)Token->Var->Value.Data)[6], ((float*)Token->Var->Value.Data)[7], ((float*)Token->Var->Value.Data)[8] }
			};
			if (Token->Var->Type == GLSL_MAT4) return { GLSL_MAT4, 0.0f, 0.0f, 0.0f, 0.0f, 0, {}, {},
				{ ((float*)Token->Var->Value.Data)[0], ((float*)Token->Var->Value.Data)[1], ((float*)Token->Var->Value.Data)[2], ((float*)Token->Var->Value.Data)[3],
				  ((float*)Token->Var->Value.Data)[4], ((float*)Token->Var->Value.Data)[5], ((float*)Token->Var->Value.Data)[6], ((float*)Token->Var->Value.Data)[7],
				  ((float*)Token->Var->Value.Data)[8], ((float*)Token->Var->Value.Data)[9], ((float*)Token->Var->Value.Data)[10], ((float*)Token->Var->Value.Data)[11],
				  ((float*)Token->Var->Value.Data)[12], ((float*)Token->Var->Value.Data)[13], ((float*)Token->Var->Value.Data)[14], ((float*)Token->Var->Value.Data)[15] }
			};
		}
	}
	else if (Token->Type == GLSL_TOK_CONST)
	{
		if (Token->Const.IsFloat) return { GLSL_FLOAT, Token->Const.Fval };
		else return { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, Token->Const.Ival };
	}
	else if (Token->Type == GLSL_TOK_VAR_DECL)
	{
		VerifyVar(Token->Var);

		glslExValue Result = ExecuteGLSLToken(Token->Second);

		AssignToExVal(Token->Var, Result);
		return {};
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
			printf("SWGL: ERROR! Type mismatch while executing shader (ADDITION)!\n");
			return {};
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
			printf("SWGL: ERROR! Type mismatch while executing shader (SUBTRACTION)!\n");
			return {};
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
				printf("SWGL: ERROR! Type mismatch while executing shader (MULTIPLY)!\n");
				return {};
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
				FirstResult.Mat2 = MatMul(FirstResult.Mat2, SecondResult.Mat2);
			}
			else if (FirstResult.Type == GLSL_MAT2 && SecondResult.Type == GLSL_VEC2)
			{
				glslVec2 Result = MatMul(FirstResult.Mat2, glslVec2{ SecondResult.x, SecondResult.y });
				FirstResult.x = Result.x;
				FirstResult.y = Result.y;
				FirstResult.Type = GLSL_VEC2;
			}
			else if (FirstResult.Type == GLSL_MAT3 && SecondResult.Type == GLSL_MAT3)
			{
				FirstResult.Mat2 = MatMul(FirstResult.Mat2, SecondResult.Mat2);
			}
			else if (FirstResult.Type == GLSL_MAT3 && SecondResult.Type == GLSL_VEC3)
			{
				glslVec3 Result = MatMul(FirstResult.Mat3, glslVec3{ SecondResult.x, SecondResult.y, SecondResult.z });
				FirstResult.x = Result.x;
				FirstResult.y = Result.y;
				FirstResult.z = Result.z;
				FirstResult.Type = GLSL_VEC3;
			}
			else if (FirstResult.Type == GLSL_MAT4 && SecondResult.Type == GLSL_MAT4)
			{
				FirstResult.Mat4 = MatMul(FirstResult.Mat4, SecondResult.Mat4);
			}
			else if (FirstResult.Type == GLSL_MAT4 && SecondResult.Type == GLSL_VEC4)
			{
				glslVec4 Result = MatMul(FirstResult.Mat4, glslVec4{ SecondResult.x, SecondResult.y, SecondResult.z, SecondResult.w });
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
			printf("SWGL: ERROR! Type mismatch while executing shader (DIVISION)!\n");
			return {};
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

		glslExValue Output = glslExValue();

		for (int i = 0; i < Token->Swizzle.size(); i++)
		{
			float CurVal = 0;
			int CurSwizzle = Token->Swizzle[i];

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

		if (Token->Swizzle.size() == 0)
		{
			printf("SWGL: ERROR! Swizzle size can't be 0!\n");
			return {};
		}
		else if (Token->Swizzle.size() == 1)
		{
			Output.Type = GLSL_FLOAT;
		}
		else if (Token->Swizzle.size() == 2)
		{
			Output.Type = GLSL_VEC2;
		}
		else if (Token->Swizzle.size() == 3)
		{
			Output.Type = GLSL_VEC3;
		}
		else if (Token->Swizzle.size() == 4)
		{
			Output.Type = GLSL_VEC4;
		}
		return Output;
	}
	else if (Token->Type == GLSL_TOK_FLOAT_CONSTRUCT)
	{
		glslExValue Arg0 = ExecuteGLSLToken(Token->Args[0]);

		return { GLSL_FLOAT, Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i };
	}
	else if (Token->Type == GLSL_TOK_VEC2_CONSTRUCT)
	{
		glslExValue Arg0 = ExecuteGLSLToken(Token->Args[0]);
		glslExValue Arg1 = ExecuteGLSLToken(Token->Args[1]);

		return { GLSL_VEC2, 
			Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i,
			Arg1.Type != GLSL_INT ? Arg1.x : (float)Arg1.i
		};
	}
	else if (Token->Type == GLSL_TOK_VEC3_CONSTRUCT)
	{
		glslExValue Arg0 = ExecuteGLSLToken(Token->Args[0]);
		glslExValue Arg1 = ExecuteGLSLToken(Token->Args[1]);
		glslExValue Arg2 = ExecuteGLSLToken(Token->Args[2]);

		return { GLSL_VEC3,
			Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i,
			Arg1.Type != GLSL_INT ? Arg1.x : (float)Arg1.i,
			Arg2.Type != GLSL_INT ? Arg2.x : (float)Arg2.i
		};
	}
	else if (Token->Type == GLSL_TOK_VEC4_CONSTRUCT)
	{
		glslExValue Arg0 = ExecuteGLSLToken(Token->Args[0]);
		glslExValue Arg1 = ExecuteGLSLToken(Token->Args[1]);
		glslExValue Arg2 = ExecuteGLSLToken(Token->Args[2]);
		glslExValue Arg3 = ExecuteGLSLToken(Token->Args[3]);

		return { GLSL_VEC4,
			Arg0.Type != GLSL_INT ? Arg0.x : (float)Arg0.i,
			Arg1.Type != GLSL_INT ? Arg1.x : (float)Arg1.i,
			Arg2.Type != GLSL_INT ? Arg2.x : (float)Arg2.i,
			Arg3.Type != GLSL_INT ? Arg3.x : (float)Arg3.i
		};
	}
	else if (Token->Type == GLSL_TOK_INT_CONSTRUCT)
	{
		glslExValue Arg0 = ExecuteGLSLToken(Token->Args[0]);

		return { GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, Arg0.Type != GLSL_INT ? (int)Arg0.x : Arg0.i };
	}
}

void ExecuteGLSLFunction(glslFunction* Func)
{
	for (glslToken* LineTok : Func->RootScope->Lines)
	{
		if (!LineTok) continue;
		ExecuteGLSLToken(LineTok);
	}
}

void ExecuteGLSL(glslTokenized Tokens)
{
	for (glslFunction* Func : Tokens.Funcs)
	{
		if (Func->Name == "main")
		{
			ExecuteGLSLFunction(Func);
		}
	}
}

GLuint glCreateShader(GLenum type)
{
	GLuint CurSize = GlobalShaders.size();
	GlobalShaders.push_back(new RawShader{ type });
	return CurSize;
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
	std::string ShaderCode;
	for (int i = 0;i < count;i++)
	{
		if (!length)
		{
			ShaderCode += std::string(string[i]);
		}
		else
		{
			for (int j = 0; j < length[i]; j++)
			{
				ShaderCode.push_back(string[i][j]);
			}
		}
	}
	GlobalShaders[shader]->MyCode = ShaderCode;
}

void glCompileShader(GLuint shader)
{
	glslTokenizer Tokenizer;
	GlobalShaders[shader]->CompiledData = Tokenizer.Tokenize(GlobalShaders[shader]->MyCode);
	GlobalShaders[shader]->Compiled = true;
}

void glDeleteShader(GLuint shader)
{
	delete GlobalShaders[shader];
}

struct Program
{
	bool Linked;
	std::vector<std::pair<glslVariable*, glslVariable*>> VertexFragInOut;
	std::vector<glslVariable*> Uniforms;
	std::vector<glslVariable*> Layouts;

	bool HasVertex;
	bool HasFrag;
	glslTokenized VertexShader;
	glslTokenized FragmentShader;
};

Program* ActiveProgram;
std::vector<Program*> GlobalPrograms;

GLuint glCreateProgram()
{
	GLuint CurSize = GlobalPrograms.size() + 1;
	GlobalPrograms.push_back(new Program);
	return CurSize;
}

void glAttachShader(GLuint program, GLuint shader)
{
	Program* MyProgram = GlobalPrograms[program - 1];
	RawShader* MyShader = GlobalShaders[shader];

	if (MyShader->Type == GL_VERTEX_SHADER)
	{
		MyProgram->HasVertex = true;
		MyProgram->VertexShader = MyShader->CompiledData;
	}
	if (MyShader->Type == GL_FRAGMENT_SHADER)
	{
		MyProgram->HasFrag = true;
		MyProgram->FragmentShader = MyShader->CompiledData;
	}
}

void glLinkProgram(GLuint program)
{
	Program* MyProgram = GlobalPrograms[program - 1];

	if (!MyProgram->HasFrag || !MyProgram->HasVertex)
	{
		printf("SWGL: Link program failed! SWGL currently needs Shader Programs to have both a vertex shader and fragment shader attached\n");
		return;
	}

	std::vector<glslVariable*> VertOuts;
	std::vector<glslVariable*> FragIns;

	std::vector<glslVariable*> Uniforms;
	std::vector<glslVariable*> Layouts;


	for (glslVariable* VertVar : MyProgram->VertexShader.GlobalVars)
	{
		if (VertVar->isOut) VertOuts.push_back(VertVar);
		if (VertVar->isUniform) Uniforms.push_back(VertVar);
		if (VertVar->isLayout) Layouts.push_back(VertVar);
	}

	for (glslVariable* FragVar : MyProgram->FragmentShader.GlobalVars)
	{
		if (FragVar->isIn) FragIns.push_back(FragVar);
		if (FragVar->isUniform) Uniforms.push_back(FragVar);
		if (FragVar->isLayout) Layouts.push_back(FragVar);
	}
	
	MyProgram->Uniforms = Uniforms;
	MyProgram->Layouts = Layouts;

	for (glslVariable* FragIn : FragIns)
	{
		bool LinkedPair = false;

		for (glslVariable* VertOut : VertOuts)
		{

			if (FragIn->Name == VertOut->Name)
			{
				MyProgram->VertexFragInOut.push_back({ FragIn, VertOut });
				LinkedPair = true;
				break;
			}
		}

		if (!LinkedPair)
		{
			printf("SWGL: Warning! No matching vertex shader OUT statement to fragment shader IN statement 'in <type> %s;'\n", FragIn->Name.c_str());
		}
	}

	MyProgram->Linked = true;
}

void glUseProgram(GLuint program)
{
	if (program == 0) ActiveProgram = 0;
	else ActiveProgram = GlobalPrograms[program - 1];
}

struct Buffer
{
	void* data;
	GLsizei size;
};

struct VertexArrayAttrib
{
	GLsizei stride;
	GLboolean normalized;
	GLenum type;
	GLint size;
	GLuint index;

	int offset;
};

struct VertexArray
{
	std::vector<VertexArrayAttrib> Attribs;
	Buffer* VertexBuffer = 0;
	Buffer* ElementBuffer = 0;
};

VertexArray* ActiveVertexArray;
std::vector<VertexArray*> GlobalVertexArrays;

GLuint glGenVertexArrays(GLsizei n, GLuint* arrays)
{
	// Only supports one vertex array per call for now
	*arrays = GlobalVertexArrays.size() + 1;
	GlobalVertexArrays.push_back(new VertexArray{ std::vector<VertexArrayAttrib>(), new Buffer(), new Buffer() });
	return 0;
}

void glBindVertexArray(GLuint array)
{
	if (array == 0) ActiveVertexArray = 0;
	else ActiveVertexArray = GlobalVertexArrays[array - 1];
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

		ActiveVertexArray->Attribs.push_back(Attrib);
	}
}

std::vector<Buffer*> GlobalBuffers;

GLuint glGenBuffers(GLsizei n, GLuint* buffers)
{
	// Only supports 1 buffer per call for now
	*buffers = GlobalBuffers.size() + 1;
	GlobalBuffers.push_back(new Buffer{ 0, 0 });
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

		if (ActiveVertexArray != 0)
		{
			GlobalArrayBuffer = ActiveVertexArray->VertexBuffer;
			GlobalArrayBuffer->data = GlobalBuffers[buffer - 1]->data;
			GlobalArrayBuffer->size = GlobalBuffers[buffer - 1]->size;
		}
		else
		{
			GlobalArrayBuffer = GlobalBuffers[buffer - 1];
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

struct Framebuffer
{
	GLsizei Width;
	GLsizei Height;

	GLenum ColorFormat;
	uint32_t* ColorAttachment;

	GLenum DepthFormat;
	void* DepthAttachment;
};

Framebuffer* GlobalFramebuffer;

GLfloat ClearColorRed;
GLfloat ClearColorGreen;
GLfloat ClearColorBlue;
GLfloat ClearColorAlpha;

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	ClearColorRed = std::min(std::max(red, 0.0f), 1.0f);
	ClearColorGreen = std::min(std::max(green, 0.0f), 1.0f);
	ClearColorBlue = std::min(std::max(blue, 0.0f), 1.0f);
	ClearColorAlpha = std::min(std::max(alpha, 0.0f), 1.0f);
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
	return glslVec4{ x.x - y.x, x.y - y.y, x.z - y.z, x.w - y.w };
}

float Dot2(glslVec4 x, glslVec4 y)
{
	return x.x * y.x + x.y * y.y;
}

void Barycentric(glslVec4 a, glslVec4 b, glslVec4 c, glslVec4 p, float& u, float& v, float& w)
{
	glslVec4 v0 = Sub(b, a), v1 = Sub(c, a), v2 = Sub(p, a);
	float d00 = Dot2(v0, v0);
	float d01 = Dot2(v0, v1);
	float d11 = Dot2(v1, v1);
	float d20 = Dot2(v2, v0);
	float d21 = Dot2(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;
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

void DrawTriangle(glslVec4* Coords, std::vector<std::pair<glslExValue, glslVariable*>>* CoordData)
{
	float minX = std::max(std::min(std::min(Coords[0].x, Coords[1].x), Coords[2].x), (float)ViewportX);
	float maxX = std::min(std::max(std::max(Coords[0].x, Coords[1].x), Coords[2].x), (float)ViewportX + ViewportWidth - 1);

	float minY = std::max(std::min(std::min(Coords[0].y, Coords[1].y), Coords[2].y), (float)ViewportY);
	float maxY = std::min(std::max(std::max(Coords[0].y, Coords[1].y), Coords[2].y), (float)ViewportY + ViewportHeight - 1);

	for (float y = minY;y <= maxY;y++)
	{
		if (y < 0) continue;
		if (y >= GlobalFramebuffer->Height) break;
		for (float x = minX; x <= maxX; x++)
		{
			if (x < 0) continue;
			if (x >= GlobalFramebuffer->Width) break;


			float u, v, w;
			Barycentric(Coords[0], Coords[1], Coords[2], glslVec4{ x, y, 0.0f, 0.0f }, u, v, w);

			if (u >= 0.0f && v >= 0.0f && w >= 0.0f)
			{

				float z = Coords[0].z * u + Coords[1].z * v + Coords[2].z * w;

				if (GlobalFramebuffer->DepthFormat == GL_FLOAT)
				{
					float* CurZ = &(((float*)GlobalFramebuffer->DepthAttachment)[(int)x + (int)y * GlobalFramebuffer->Width]);
					if (*CurZ == 0.0f || *CurZ >= z)
					{
						*CurZ = z;

						uint32_t* CurCol = &(GlobalFramebuffer->ColorAttachment[(int)x + (GlobalFramebuffer->Height - 1 -(int)y) * GlobalFramebuffer->Width]);

						for (int i = 0; i < CoordData[0].size(); i++)
						{
							glslExValue InterpVal = InterpolateLinearEx(CoordData[0][i].first, CoordData[1][i].first, CoordData[2][i].first, u, v, w);

							AssignToExVal(CoordData[0][i].second, InterpVal);
						}

						ExecuteGLSL(ActiveProgram->FragmentShader);

						float OutR, OutG, OutB, OutA;

						for (glslVariable* Var : ActiveProgram->FragmentShader.GlobalVars)
						{
							if (Var->isOut)
							{
								OutR = ((float*)Var->Value.Data)[0];
								OutG = ((float*)Var->Value.Data)[1];
								OutB = ((float*)Var->Value.Data)[2];
								OutA = ((float*)Var->Value.Data)[3];
							}
						}

						OutR = std::min(std::max(OutR, 0.0f), 1.0f);
						OutG = std::min(std::max(OutG, 0.0f), 1.0f);
						OutB = std::min(std::max(OutB, 0.0f), 1.0f);
						OutA = std::min(std::max(OutA, 0.0f), 1.0f);

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

	for (glslVariable* Var : ActiveProgram->VertexShader.GlobalVars)
	{
		if (Var->Name == "gl_Position")
		{
			glPositionVar = Var;
		}
	}

	VerifyVar(glPositionVar);

	if (mode == GL_POINTS)
	{
		for (int i = first; i < first + count; i++)
		{
			for (VertexArrayAttrib& Attrib : ActiveVertexArray->Attribs)
			{
				if (Attrib.type == GL_FLOAT)
				{
					float* AttribData = (float*)((uint8_t*)ActiveVertexArray->VertexBuffer->data + i * Attrib.stride + Attrib.offset);

					for (glslVariable* Var : ActiveProgram->Layouts)
					{
						if (Var->Layout->Location == Attrib.index)
						{
							if (!Var->Value.Alloc)
							{
								Var->Value.Alloc = true;
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

			for (std::pair<glslVariable*, glslVariable*> InOut : ActiveProgram->VertexFragInOut)
			{
				if (InOut.first->Type != InOut.second->Type)
				{
					printf("SWGL: ERROR! Type mismatch between out/in pair in shaders\n");
					continue;
				}

				if (InOut.first->Type == GLSL_FLOAT) memcpy(InOut.first->Value.Data, InOut.second->Value.Data, sizeof(float));
				if (InOut.first->Type == GLSL_VEC2) memcpy(InOut.first->Value.Data, InOut.second->Value.Data, sizeof(float) * 2);
				if (InOut.first->Type == GLSL_VEC3) memcpy(InOut.first->Value.Data, InOut.second->Value.Data, sizeof(float) * 3);
				if (InOut.first->Type == GLSL_VEC4) memcpy(InOut.first->Value.Data, InOut.second->Value.Data, sizeof(float) * 4);
				if (InOut.first->Type == GLSL_INT) memcpy(InOut.first->Value.Data, InOut.second->Value.Data, sizeof(int));
			}

			ExecuteGLSL(ActiveProgram->FragmentShader);

			float OutR, OutG, OutB, OutA;

			for (glslVariable* Var : ActiveProgram->FragmentShader.GlobalVars)
			{
				if (Var->isOut)
				{
					OutR = ((float*)Var->Value.Data)[0];
					OutG = ((float*)Var->Value.Data)[1];
					OutB = ((float*)Var->Value.Data)[2];
					OutA = ((float*)Var->Value.Data)[3];
				}
			}

			OutR = std::min(std::max(OutR, 0.0f), 1.0f);
			OutG = std::min(std::max(OutG, 0.0f), 1.0f);
			OutB = std::min(std::max(OutB, 0.0f), 1.0f);
			OutA = std::min(std::max(OutA, 0.0f), 1.0f);

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

			std::vector<std::pair<glslExValue, glslVariable*>> TriangleVertexData[3];

			for (int j = 0; j < 3; j++)
			{
				for (VertexArrayAttrib& Attrib : ActiveVertexArray->Attribs)
				{
					if (Attrib.type == GL_FLOAT)
					{
						float* AttribData = (float*)((uint8_t*)ActiveVertexArray->VertexBuffer->data + (i + j) * Attrib.stride + Attrib.offset);

						for (glslVariable* Var : ActiveProgram->Layouts)
						{
							if (Var->Layout->Location == Attrib.index)
							{
								if (!Var->Value.Alloc)
								{
									Var->Value.Alloc = true;
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

				TriangleVertexData[j] = std::vector<std::pair<glslExValue, glslVariable*>>();

				for (std::pair<glslVariable*, glslVariable*> InOut : ActiveProgram->VertexFragInOut)
				{
					if (InOut.first->Type != InOut.second->Type)
					{
						printf("SWGL: ERROR! Type mismatch between out/in pair in shaders\n");
						continue;
					}
					TriangleVertexData[j].push_back({ VarToExVal(InOut.second), InOut.first });
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
			int nTri = ClipTriangleAgainstNearPlane(MyTri, Triangles);

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
				}

				DrawTriangle(TriangleCoords, Tri.TriangleVertexData);
			}
		}
	}
}

void glInit(GLsizei width, GLsizei height)
{
	GlobalFramebuffer = new Framebuffer;
	GlobalFramebuffer->Width = width;
	GlobalFramebuffer->Height = height;
	
	GlobalFramebuffer->DepthFormat = GL_FLOAT;
	GlobalFramebuffer->DepthAttachment = malloc(sizeof(float) * width * height);

	GlobalFramebuffer->ColorFormat = GL_RGBA;
	GlobalFramebuffer->ColorAttachment = (uint32_t*)malloc(4 * width * height);

	glslOpChars = std::vector<char>{ '+', '-', '*', '/', '>', '<', '=' };

	GlobalArrayBuffer = 0;
	GlobalBuffers = std::vector<Buffer*>();
	GlobalPrograms = std::vector<Program*>();
	GlobalVertexArrays = std::vector<VertexArray*>();
	GlobalShaders = std::vector<RawShader*>();

	ActiveProgram = 0;
	ActiveVertexArray = 0;
}

uint32_t* glGetFramePtr()
{
	return GlobalFramebuffer->ColorAttachment;
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
	Program* MyProgram = GlobalPrograms[program - 1];

	GLint i = 0;

	for (glslVariable* Uniform : MyProgram->Uniforms)
	{
		if (Uniform->Name == name)
		{
			return ((program - 1) << 16) | i;
		}
		i++;
	}

	return -1;
}

void glUniform1f(GLint location, GLfloat v0)
{
	Program* MyProgram = GlobalPrograms[location >> 16];

	glslExValue SetVal{ GLSL_FLOAT, v0 };

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	Program* MyProgram = GlobalPrograms[location >> 16];

	glslExValue SetVal{ GLSL_VEC2, v0, v1 };

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	Program* MyProgram = GlobalPrograms[location >> 16];

	glslExValue SetVal{ GLSL_VEC3, v0, v1, v2 };

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	Program* MyProgram = GlobalPrograms[location >> 16];

	glslExValue SetVal{ GLSL_VEC4, v0, v1, v2, v3 };

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniform1i(GLint location, GLint v0)
{
	Program* MyProgram = GlobalPrograms[location >> 16];

	glslExValue SetVal{ GLSL_INT, 0.0f, 0.0f, 0.0f, 0.0f, v0 };

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
	VerifyVar(MyUniform);

	AssignToExVal(MyUniform, SetVal);
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	transpose = !transpose;

	Program* MyProgram = GlobalPrograms[location >> 16];

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
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

	Program* MyProgram = GlobalPrograms[location >> 16];

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
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

	Program* MyProgram = GlobalPrograms[location >> 16];

	glslVariable* MyUniform = MyProgram->Uniforms[location & 0xFFFF];
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
