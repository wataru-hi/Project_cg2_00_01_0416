#pragma once
#include <assert.h>
#include <cmath>
#include <vector>
#include <string>

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
//vec, matrix, transform
namespace mymath
{

	

	
	Vector3 changeVec3(Vector4 a);

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	struct DirectrionaLight {
		Vector4 color; //!< ライトの色
		Vector3 direction; //!< ライトの向き
		float intensity; //!< 輝度
	};

	struct MaterialData {
		std::string textureFilepPath;
	};

	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	Matrix4x4 add(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 Inverse(const Matrix4x4& m);

	Matrix4x4 Transpose(const Matrix4x4& m);

	Matrix4x4 MakeIdentity4x4();

	Matrix4x4 MakeTranslateMatrix(const Vector3& vector);

	Matrix4x4 MakeScaleMatrix(const Vector3& vector);

	Vector3 Transford(const Vector3& vector, const Matrix4x4& matrix);

	Matrix4x4 MakeRoatateXMatix(float radian);

	Matrix4x4 MakeRoatateYMatix(float radian);

	Matrix4x4 MakeRoatateZMatix(float radian);

	Matrix4x4 MakeRotateMatrix(const Vector3& radian);

	Matrix4x4 MakeAfineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	Matrix4x4 makePerspectiveMatrix(const float fovY, const float aspectRatio, const float nearClip, const float farClip);

	Matrix4x4 makeOrthogphicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);

	Matrix4x4 makeViewportMatrix(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth);
};