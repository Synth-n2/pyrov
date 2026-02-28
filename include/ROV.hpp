#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Constants
constexpr float WATER_OPACITY = 0.4f;
constexpr float WATER_LEVEL = 0.0f;
constexpr float GRAVITY = 9.81f;
constexpr float WATER_DENSITY = 1000.0f;
constexpr float FIXED_PHYSICS_STEP = 1.0f / 120.0f; // 120Hz physics

// Basic 3D struct for physics
struct ROV {
  // 6 DOF Translational
  glm::vec3 position{0.0f, -2.0f, -5.0f};
  glm::vec3 velocity{0.0f};
  glm::vec3 acceleration{0.0f};

  // 6 DOF Rotational
  glm::quat orientation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 angularVelocity{0.0f};
  glm::vec3 angularAcceleration{0.0f};

  // Physical properties
  float mass = 30.0f;    // kg
  float volume = 0.035f; // m^3
  glm::vec3 inertiaTensor{5.0f, 5.0f, 5.0f};

  // Drag coefficients for different axes
  glm::vec3 linearDrag{300.0f, 400.0f, 300.0f};
  glm::vec3 angularDrag{150.0f, 150.0f, 150.0f};

  // Arm state (3 DOF)
  float armBaseYaw = 0.0f;
  float armShoulderPitch = 0.0f;
  float armElbowPitch = -0.5f;

  // Control forces for this frame
  glm::vec3 forceAccumulator{0.0f};
  glm::vec3 torqueAccumulator{0.0f};

  void AddForce(glm::vec3 force);
  void AddTorque(glm::vec3 torque);
};

void UpdatePhysics(ROV &rov, float dt);
