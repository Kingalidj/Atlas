#include "atlpch.h"
#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Atlas
{
	void PerspectiveCamera::RecalculateViewMatrix()
	{
		ATL_PROFILE_FUNCTION();

		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::RecalculateProjectionMatrix()
	{
		ATL_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::perspective(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	PerspectiveCamera::PerspectiveCamera(float nearPlane, float farPlane, float fov, float aspectRatio)
		: m_NearPlane(nearPlane), m_FarPlane(farPlane), m_Fov(fov), m_AspectRatio(aspectRatio),
		m_ProjectionMatrix(glm::perspective(fov, aspectRatio, nearPlane, farPlane))
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::SetFrontVec(const glm::vec3& direction)
	{
		m_Front = direction;
		m_Right = glm::normalize(glm::cross(m_Front, m_Up));
		RecalculateViewMatrix();
	}

	void PerspectiveCamera::SetProjection(float nearPlane, float farPlane, float fov, float aspectRatio)
	{
		ATL_PROFILE_FUNCTION();

		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		m_Fov = fov;
		m_AspectRatio = aspectRatio;
		m_ProjectionMatrix = glm::perspective(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
	}
}
