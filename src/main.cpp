#include <cmath>
#include <cstdlib>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>

#include "Props.hpp"
#include "ROV.hpp"

enum class GameState { MENU, SIMULATION };

// Particle system for marine snow
struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  float life;
};

// Helpers for glm <-> raylib conversion
Vector3 toRaylib(glm::vec3 v) { return {v.x, v.y, v.z}; }
glm::vec3 toGLM(Vector3 v) { return {v.x, v.y, v.z}; }

int main(void) {
  const int screenWidth = 1280;
  const int screenHeight = 720;

  SetConfigFlags(FLAG_MSAA_4X_HINT |
                 FLAG_VSYNC_HINT); // Enable VSYNC to prevent 100% GPU
  InitWindow(screenWidth, screenHeight,
             "MATE ROV Simulator - 6 DOF + 3 DOF Arm");
  SetTargetFPS(240); // Allow unlocked/high framerate

  GameState state = GameState::MENU;

  // Initialize ROV
  ROV rov;

  // Initialize Camera
  Camera3D camera = {0};
  camera.position = (Vector3){0.0f, 0.0f, 0.0f};
  camera.target = (Vector3){0.0f, 0.0f, 1.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  // Initialize particles
  std::vector<Particle> particles(150);
  for (auto &p : particles) {
    p.position = glm::vec3(GetRandomValue(-20, 20), GetRandomValue(-20, 0),
                           GetRandomValue(-20, 20));
    p.velocity = glm::vec3(GetRandomValue(-5, 5) * 0.02f,
                           GetRandomValue(-10, -1) * 0.02f,
                           GetRandomValue(-5, 5) * 0.02f);
    p.life = GetRandomValue(50, 200) / 100.0f;
  }

  double accumulator = 0.0;
  double lastTime = GetTime();

  while (!WindowShouldClose()) {
    double currentTime = GetTime();
    double frameTime = currentTime - lastTime;
    lastTime = currentTime;

    if (state == GameState::MENU) {
      // Main Menu Logic
      if (IsKeyPressed(KEY_ENTER)) {
        state = GameState::SIMULATION;
        lastTime = GetTime(); // Reset timer so we don't jump on load
      }

      BeginDrawing();
      ClearBackground(Color{5, 15, 40, 255}); // Dark blue water color

      // MATE UI
      DrawText("MATE ROV COMPETITION 2026",
               screenWidth / 2 -
                   MeasureText("MATE ROV COMPETITION 2026", 40) / 2,
               200, 40, LIGHTGRAY);
      DrawText("RANGER DIVISION SIMULATOR",
               screenWidth / 2 -
                   MeasureText("RANGER DIVISION SIMULATOR", 60) / 2,
               250, 60, RAYWHITE);

      // Instructions
      DrawText("Press ENTER to Start Dive",
               screenWidth / 2 -
                   MeasureText("Press ENTER to Start Dive", 30) / 2,
               450, 30, YELLOW);

      DrawText("Features 6 DOF Physics, Fixed Timestep, and 2026 Props.", 10,
               screenHeight - 30, 20, GRAY);

      EndDrawing();

    } else if (state == GameState::SIMULATION) {
      // Simulation Logic
      accumulator += frameTime;

      if (accumulator > 0.2)
        accumulator = 0.2; // Cap accumulator

      // 1. Control input handling
      glm::vec3 forward = rov.orientation * glm::vec3(0, 0, 1);
      glm::vec3 right = rov.orientation * glm::vec3(1, 0, 0);
      glm::vec3 up = rov.orientation * glm::vec3(0, 1, 0);

      float thrustForce = 1500.0f;
      float torqueForce = 300.0f;

      glm::vec3 frameForce{0.0f};
      glm::vec3 frameTorque{0.0f};

      // Translation
      if (IsKeyDown(KEY_W))
        frameForce += forward * thrustForce;
      if (IsKeyDown(KEY_S))
        frameForce -= forward * thrustForce;
      if (IsKeyDown(KEY_D))
        frameForce += right * thrustForce;
      if (IsKeyDown(KEY_A))
        frameForce -= right * thrustForce;
      if (IsKeyDown(KEY_LEFT_SHIFT))
        frameForce += up * thrustForce;
      if (IsKeyDown(KEY_LEFT_CONTROL))
        frameForce -= up * thrustForce;

      // Rotation
      if (IsKeyDown(KEY_Q))
        frameTorque += up * torqueForce;
      if (IsKeyDown(KEY_E))
        frameTorque -= up * torqueForce;
      if (IsKeyDown(KEY_UP))
        frameTorque += right * torqueForce;
      if (IsKeyDown(KEY_DOWN))
        frameTorque -= right * torqueForce;
      if (IsKeyDown(KEY_LEFT))
        frameTorque += forward * torqueForce;
      if (IsKeyDown(KEY_RIGHT))
        frameTorque -= forward * torqueForce;

      // Arm Controls
      if (IsKeyDown(KEY_I))
        rov.armShoulderPitch += 1.0f * frameTime;
      if (IsKeyDown(KEY_K))
        rov.armShoulderPitch -= 1.0f * frameTime;
      if (IsKeyDown(KEY_J))
        rov.armBaseYaw += 1.0f * frameTime;
      if (IsKeyDown(KEY_L))
        rov.armBaseYaw -= 1.0f * frameTime;
      if (IsKeyDown(KEY_U))
        rov.armElbowPitch += 1.0f * frameTime;
      if (IsKeyDown(KEY_O))
        rov.armElbowPitch -= 1.0f * frameTime;

      rov.armShoulderPitch = glm::clamp(rov.armShoulderPitch, -1.0f, 1.0f);
      rov.armElbowPitch = glm::clamp(rov.armElbowPitch, -2.0f, 0.0f);
      rov.armBaseYaw = glm::clamp(rov.armBaseYaw, -1.5f, 1.5f);

      // 2. Fixed Timestep Physics Integration
      while (accumulator >= FIXED_PHYSICS_STEP) {
        rov.AddForce(frameForce);
        rov.AddTorque(frameTorque);

        UpdatePhysics(rov, FIXED_PHYSICS_STEP);
        accumulator -= FIXED_PHYSICS_STEP;
      }

      // 3. Environment updates (Particles)
      for (auto &p : particles) {
        p.position += p.velocity * (float)frameTime;
        p.position.x += sin(GetTime() + p.life) * 0.01f;
        if (p.position.y > 0 ||
            glm::distance(p.position, rov.position) > 20.0f) {
          p.position = rov.position + glm::vec3(GetRandomValue(-15, 15),
                                                GetRandomValue(-15, 15),
                                                GetRandomValue(5, 20));
          p.position.y = glm::min(p.position.y, -0.1f);
        }
      }

      // 4. Camera Update
      glm::vec3 camLocalOffset(0.0f, 0.2f, 0.4f);
      glm::vec3 camWorldPos = rov.position + (rov.orientation * camLocalOffset);
      glm::vec3 camWorldTarget =
          camWorldPos + (rov.orientation * glm::vec3(0, 0, 1));

      camera.position = toRaylib(camWorldPos);
      camera.target = toRaylib(camWorldTarget);
      camera.up = toRaylib(rov.orientation * glm::vec3(0, 1, 0));

      // 5. Render
      BeginDrawing();
      ClearBackground(Color{10, 30, 60, 255});

      BeginMode3D(camera);

      // Draw Environment
      rlPushMatrix();
      rlTranslatef(0, -10.0f, 0);
      DrawGrid(100, 1.0f);
      rlPopMatrix();

      DrawPlane(Vector3{0, 0, 0}, Vector2{100.0f, 100.0f},
                Color{100, 150, 255, (unsigned char)(WATER_OPACITY * 255)});

      // DRAW 2026 MATE RANGER PROPS
      RenderCoralGarden(glm::vec3(-5.0f, -10.0f, 15.0f));
      RenderProfilingFloat(glm::vec3(10.0f, 0.0f, 10.0f),
                           -4.0f + sin(GetTime()) * 0.5f);
      RenderCrabMap(glm::vec3(0.0f, 0.0f, 12.0f));
      RenderCrabMap(glm::vec3(2.0f, 0.0f, 14.0f));
      RenderCrabMap(glm::vec3(-3.0f, 0.0f, 18.0f));

      // DRAW ROV & ARM
      rlPushMatrix();
      rlTranslatef(rov.position.x, rov.position.y, rov.position.z);

      // Apply ROV orientation
      glm::vec3 axis = glm::axis(rov.orientation);
      float angle = glm::angle(rov.orientation);
      rlRotatef(glm::degrees(angle), axis.x, axis.y, axis.z);

      DrawCube(Vector3{0, 0, 0}, 0.5f, 0.4f, 0.8f, YELLOW);
      DrawCubeWires(Vector3{0, 0, 0}, 0.5f, 0.4f, 0.8f, BLACK);

      // 3 DOF Arm
      rlPushMatrix();
      rlTranslatef(0.0f, -0.1f, 0.4f);
      rlRotatef(glm::degrees(rov.armBaseYaw), 0, 1, 0);
      DrawCylinder(Vector3{0, 0, 0}, 0.05f, 0.05f, 0.1f, 12, DARKGRAY);

      rlPushMatrix();
      rlTranslatef(0.0f, 0.0f, 0.05f);
      rlRotatef(glm::degrees(rov.armShoulderPitch), 1, 0, 0);
      DrawCube(Vector3{0, 0, 0.2f}, 0.04f, 0.04f, 0.4f, GRAY);

      rlPushMatrix();
      rlTranslatef(0.0f, 0.0f, 0.4f);
      rlRotatef(glm::degrees(rov.armElbowPitch), 1, 0, 0);
      DrawCube(Vector3{0, 0, 0.15f}, 0.03f, 0.03f, 0.3f, LIGHTGRAY);
      DrawSphere(Vector3{0, 0, 0.3f}, 0.03f, RED);

      rlPopMatrix();
      rlPopMatrix();
      rlPopMatrix();

      rlPopMatrix();

      // Particle System
      for (const auto &p : particles) {
        DrawPoint3D(toRaylib(p.position), Color{200, 255, 255, 180});
      }

      EndMode3D();

      // Fake Post-Processing
      DrawRectangle(0, 0, screenWidth, screenHeight, Color{10, 50, 120, 40});

      // HUD Overlay
      DrawFPS(10, 10);

      DrawText(TextFormat("Depth: %.2f m", -rov.position.y), 10, 40, 20, WHITE);
      DrawText(TextFormat("Speed: %.2f m/s", glm::length(rov.velocity)), 10, 60,
               20, WHITE);

      // Controls Overlay
      DrawText("Controls:", 10, screenHeight - 110, 20, LIGHTGRAY);
      DrawText("W/S: Surge | A/D: Sway | LShift/LCtrl: Heave", 10,
               screenHeight - 80, 20, WHITE);
      DrawText("Q/E: Yaw | Up/Down: Pitch | L/R Arr: Roll", 10,
               screenHeight - 50, 20, WHITE);
      DrawText("I/K, J/L, U/O: Robotic Arm Joints", 10, screenHeight - 20, 20,
               YELLOW);

      // Reticle
      DrawCircle(screenWidth / 2, screenHeight / 2, 2.0f, GREEN);
      DrawLine(screenWidth / 2 - 20, screenHeight / 2, screenWidth / 2 - 10,
               screenHeight / 2, GREEN);
      DrawLine(screenWidth / 2 + 20, screenHeight / 2, screenWidth / 2 + 10,
               screenHeight / 2, GREEN);
      DrawLine(screenWidth / 2, screenHeight / 2 - 20, screenWidth / 2,
               screenHeight / 2 - 10, GREEN);
      DrawLine(screenWidth / 2, screenHeight / 2 + 20, screenWidth / 2,
               screenHeight / 2 + 10, GREEN);

      EndDrawing();
    }
  }

  CloseWindow();
  return 0;
}
