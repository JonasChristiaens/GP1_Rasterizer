#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};
		float m_NearPlane{ 1.0f };
		float m_FarPlane{ 1000.0f };

		Matrix worldMatrix{ {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} };
		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};
		Matrix worldViewProjectionMatrix{};

		bool rotateModel{ false };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float _nearPlane = 0.1f, float _farPlane = 100.0f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			m_NearPlane = _nearPlane;
			m_FarPlane = _farPlane;
		}

		void CalculateViewMatrix()
		{
			// done in week 1
			
			//ONB => invViewMatrix
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			Matrix invViewMatrix{ {right, 0}, {up, 0}, {forward, 0}, {origin, 1} };

			//Inverse(ONB) => ViewMatrix
			viewMatrix = invViewMatrix.Inverse();

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix(float aspectRatio)
		{
			// done in week 3
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, m_NearPlane, m_FarPlane);

			// combine all space transformation matrix into one matrix
			worldViewProjectionMatrix = worldMatrix * viewMatrix * projectionMatrix;
		}

		void Update(Timer* pTimer, float aspectRatio)
		{
			//Camera Update Logic
			const float deltaTime = pTimer->GetElapsed();
			const float movementSpeed{ 20.f };
			const float rotationSpeed{ 20.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//Transforming camera's origin ( Movement )
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * movementSpeed * deltaTime;
			}

			//Transforming camera's forward vector ( Rotation )
			if (mouseState == 4)
			{
				totalPitch -= mouseY * rotationSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if (mouseState == 1)
			{
				origin -= forward * mouseY * movementSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if (mouseState == 5)
			{
				origin.y -= mouseY * movementSpeed * deltaTime;
			}
			Matrix rotationMatrix{ Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS) };

			if (rotateModel)
			{
				const auto yawAngle{ pTimer->GetTotal() };
				worldMatrix = Matrix::CreateRotationY(yawAngle);
			}

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(aspectRatio); 
		}

		void RotateModel()
		{
			rotateModel = !rotateModel;
		}
	};
}
