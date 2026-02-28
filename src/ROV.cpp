#include "ROV.hpp"
#include <glm/glm.hpp>

void ROV::AddForce(glm::vec3 force) { forceAccumulator += force; }
void ROV::AddTorque(glm::vec3 torque) { torqueAccumulator += torque; }

void UpdatePhysics(ROV &rov, float dt) {
  // 1. Environment Forces
  // Gravity
  rov.AddForce(glm::vec3(0, -GRAVITY * rov.mass, 0));

  // Buoyancy (only if submerged)
  if (rov.position.y < WATER_LEVEL) {
    float depthPercent =
        glm::clamp((WATER_LEVEL - rov.position.y) / 0.5f, 0.0f, 1.0f);
    float buoyancyScalar = WATER_DENSITY * rov.volume * GRAVITY * depthPercent;
    rov.AddForce(glm::vec3(0, buoyancyScalar, 0));

    // Buoyancy puts center of buoyancy slightly above COM
    glm::vec3 cobLocal(0.0f, 0.1f, 0.0f);
    glm::vec3 globalCobOffset = rov.orientation * cobLocal;
    rov.AddTorque(glm::cross(globalCobOffset, glm::vec3(0, buoyancyScalar, 0)));
  }

  // Fluid Drag (Linear)
  glm::vec3 localVel = glm::conjugate(rov.orientation) * rov.velocity;
  glm::vec3 localDragForce = -rov.linearDrag * localVel * glm::abs(localVel);
  rov.AddForce(rov.orientation * localDragForce);

  // Fluid Drag (Angular)
  glm::vec3 localAngVel = glm::conjugate(rov.orientation) * rov.angularVelocity;
  glm::vec3 localDragTorque =
      -rov.angularDrag * localAngVel * glm::abs(localAngVel);
  rov.AddTorque(rov.orientation * localDragTorque);

  // 2. Integration Output (Semi-Implicit Euler)
  rov.acceleration = rov.forceAccumulator / rov.mass;
  rov.velocity += rov.acceleration * dt;
  rov.position += rov.velocity * dt;

  // Angular integration
  glm::vec3 localTorque =
      glm::conjugate(rov.orientation) * rov.torqueAccumulator;
  glm::vec3 localAngAcc = localTorque / rov.inertiaTensor;
  rov.angularAcceleration = rov.orientation * localAngAcc;

  rov.angularVelocity += rov.angularAcceleration * dt;

  // Update orientation quaternion
  if (glm::length(rov.angularVelocity) > 0.0001f) {
    glm::quat spin(0.0f, rov.angularVelocity.x * dt * 0.5f,
                   rov.angularVelocity.y * dt * 0.5f,
                   rov.angularVelocity.z * dt * 0.5f);
    rov.orientation = glm::normalize(rov.orientation + spin * rov.orientation);
  }

  // Reset accumulators for next frame
  rov.forceAccumulator = glm::vec3(0.0f);
  rov.torqueAccumulator = glm::vec3(0.0f);
}
