#include <glm.hpp>

class Camera {
private:
	glm::vec3 cameraPosition;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float fieldOfView;
public:
	void setFieldOfView(float fieldOfView);

public:
	float getFieldOfView() const;

public:
	Camera(const glm::vec3& cameraPosition, const glm::vec3& cameraFront, const glm::vec3& cameraUp);

	const glm::vec3& getCameraPosition() const;

	void setCameraPosition(const glm::vec3& cameraPosition);

	const glm::vec3& getCameraFront() const;

	void setCameraFront(const glm::vec3& cameraFront);

	const glm::vec3& getCameraUp() const;

	void setCameraUp(const glm::vec3& cameraUp);
};
