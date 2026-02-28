#include "Props.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

// Procedural MATE 2026 Props
void RenderCoralGarden(glm::vec3 center) {
  rlPushMatrix();
  rlTranslatef(center.x, center.y, center.z);
  // A simple "coral reef" made of intersecting white PVC lines
  for (int i = 0; i < 15; i++) {
    float xOffset = sin(i * 1.5f) * 2.0f;
    float zOffset = cos(i * 2.1f) * 2.0f;
    float height = 1.0f + (i % 3) * 0.5f;
    DrawCylinder(Vector3{xOffset, height / 2.0f, zOffset}, 0.02f, 0.02f, height,
                 4, WHITE);

    // Connect branches
    if (i > 0) {
      float pxOffset = sin((i - 1) * 1.5f) * 2.0f;
      float pzOffset = cos((i - 1) * 2.1f) * 2.0f;
      DrawLine3D(Vector3{xOffset, height, zOffset},
                 Vector3{pxOffset, height, pzOffset}, RAYWHITE);
    }
  }
  rlPopMatrix();
}

void RenderProfilingFloat(glm::vec3 position, float depth) {
  rlPushMatrix();
  rlTranslatef(position.x, depth, position.z);
  // Float main body
  DrawCylinder(Vector3{0, 0, 0}, 0.3f, 0.3f, 1.2f, 16, YELLOW);
  DrawCylinderWires(Vector3{0, 0, 0}, 0.3f, 0.3f, 1.2f, 16, BLACK);

  // U-bolt on top for ROV recovery (Simulated with a cylinder since DrawTorus
  // isn't in base raylib)
  DrawCylinder(Vector3{0, 0.65f, 0}, 0.1f, 0.1f, 0.05f, 8, DARKGRAY);
  rlPopMatrix();

  // Line connecting float to bottom
  DrawLine3D(Vector3{position.x, depth - 0.6f, position.z},
             Vector3{position.x, -10.0f, position.z}, LIGHTGRAY);
}

void RenderCrabMap(glm::vec3 position) {
  rlPushMatrix();
  rlTranslatef(position.x, -9.9f, position.z); // Slightly above floor
  // Main Crab body
  DrawCube(Vector3{0, 0, 0}, 0.4f, 0.2f, 0.3f, RED);
  // Legs
  DrawCube(Vector3{0.3f, -0.1f, 0.2f}, 0.1f, 0.3f, 0.1f, MAROON);
  DrawCube(Vector3{-0.3f, -0.1f, 0.2f}, 0.1f, 0.3f, 0.1f, MAROON);
  DrawCube(Vector3{0.3f, -0.1f, -0.2f}, 0.1f, 0.3f, 0.1f, MAROON);
  DrawCube(Vector3{-0.3f, -0.1f, -0.2f}, 0.1f, 0.3f, 0.1f, MAROON);
  rlPopMatrix();
}
