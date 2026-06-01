#include <customEngine.hpp>
#include <cmath>

class CameraController
{
    struct MouseState {
        float prevX = 0.0f;
        float prevY = 0.0f;
        bool initialized = false;
    };

    struct RotationDelta {
        float deltaPitchDeg;
        float deltaYawDeg;
    };

    RotationDelta MouseToRotationDeg(float mouseX, float mouseY,
        MouseState& state,
        float sensDegPerPixel = 0.1f,
        bool centerLock = false,
        bool invertY = false,
        float winCenterX = 0.0f,
        float winCenterY = 0.0f)
    {
        float dx, dy;

        if (!state.initialized) {
            if (centerLock) {
                state.prevX = winCenterX;
                state.prevY = winCenterY;
            }
            else {
                state.prevX = mouseX;
                state.prevY = mouseY;
            }
            state.initialized = true;
        }

        if (centerLock) {
            dx = mouseX - winCenterX;
            dy = mouseY - winCenterY;
        }
        else {
            dx = mouseX - state.prevX;
            dy = mouseY - state.prevY;
            state.prevX = mouseX;
            state.prevY = mouseY;
        }

        float yawDeg = dx * sensDegPerPixel;
        float pitchDeg = (invertY ? dy : -dy) * sensDegPerPixel;

        return { pitchDeg, yawDeg };
    }

    const float PI = 3.14159265358979323846f;
    float Deg2Rad(float d) { return d * (PI / 180.0f); }

    Quaternion quat_from_axis_angle(const Vec3& axis, float angleRad) {
        float half = angleRad * 0.5f;
        float s = sinf(half);
        return { cosf(half), axis.x * s, axis.y * s, axis.z * s };
    }

    void quat_normalize(Quaternion& q) {
        float mag = sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
        if (mag > 0.0f) { float inv = 1.0f / mag; q.w *= inv; q.x *= inv; q.y *= inv; q.z *= inv; }
    }

    Quaternion quat_mul(const Quaternion& a, const Quaternion& b) {
        return {
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w
        };
    }

    Vec3 quat_rotate_vec(const Quaternion& q, const Vec3& v) {
        float ix = q.w * v.x + q.y * v.z - q.z * v.y;
        float iy = q.w * v.y + q.z * v.x - q.x * v.z;
        float iz = q.w * v.z + q.x * v.y - q.y * v.x;
        float iw = -q.x * v.x - q.y * v.y - q.z * v.z;
        return {
            ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y,
            iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z,
            iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x
        };
    }

    Quaternion currentOrientation = { 1.0f, 0.0f, 0.0f, 0.0f };
    Vec3 moveVelocity = { 0.0f, 0.0f, 0.0f };
    float currentPitchRad = 0.0f;
    float currentYawRad = 0.0f;
    const float maxPitchRad = 89.0f * (PI / 180.0f);

    MouseState state;

public:
    void resetMouseState() { state.initialized = false; }

    void update(float mousePitch, float mouseYaw, float fwd, float right, float up, bool horizontalOnly,
        Quaternion& cameraRotationOut, Vec3& cameraTranslation, Aspect windowAspect, float sensitivity,
        float deltaTime, float frictionPerSecond)
    {
        RotationDelta mouseDegrees = MouseToRotationDeg(mousePitch, mouseYaw, state, sensitivity*deltaTime, false, false, windowAspect.x, windowAspect.y);

        float dYawRad = Deg2Rad(mouseDegrees.deltaYawDeg);
        float dPitchRad = Deg2Rad(mouseDegrees.deltaPitchDeg);

        currentYawRad += dYawRad;
        currentPitchRad += dPitchRad;
        if (currentPitchRad > maxPitchRad) currentPitchRad = maxPitchRad;
        if (currentPitchRad < -maxPitchRad) currentPitchRad = -maxPitchRad;

        Quaternion qYaw   = quat_from_axis_angle({ 0.0f, 1.0f, 0.0f }, currentYawRad);
        Quaternion qPitch = quat_from_axis_angle({ 1.0f, 0.0f, 0.0f }, currentPitchRad);
        currentOrientation = quat_mul(qYaw, qPitch);

        quat_normalize(currentOrientation);
        cameraRotationOut = currentOrientation;

        Vec3 localMove = { right, up, -fwd };
        Vec3 worldMove;

        if (horizontalOnly) {
            Vec3 forward = quat_rotate_vec(currentOrientation, { 0.0f, 0.0f, 1.0f });
            Vec3 rightDir = quat_rotate_vec(currentOrientation, { 1.0f, 0.0f, 0.0f });

            forward.y = 0.0f;
            rightDir.y = 0.0f;

            float fLen = sqrtf(forward.x * forward.x + forward.z * forward.z);
            float rLen = sqrtf(rightDir.x * rightDir.x + rightDir.z * rightDir.z);

            if (fLen > 0.0f) { float inv = 1.0f / fLen; forward.x *= inv; forward.z *= inv; }
            if (rLen > 0.0f) { float inv = 1.0f / rLen; rightDir.x *= inv; rightDir.z *= inv; }

            worldMove.x = rightDir.x * right + forward.x * (-fwd);
            worldMove.y = up;
            worldMove.z = rightDir.z * right + forward.z * (-fwd);
        }
        else {
            worldMove = quat_rotate_vec(currentOrientation, localMove);
        }

        moveVelocity.x += worldMove.x;
        moveVelocity.y += worldMove.y;
        moveVelocity.z += worldMove.z;

        float damping = expf(-(frictionPerSecond / 5.0f) * deltaTime);
        moveVelocity.x *= damping;
        moveVelocity.y *= damping;
        moveVelocity.z *= damping;

        const float stopEps = 0.0001f;
        if (fabsf(moveVelocity.x) < stopEps) moveVelocity.x = 0.0f;
        if (fabsf(moveVelocity.y) < stopEps) moveVelocity.y = 0.0f;
        if (fabsf(moveVelocity.z) < stopEps) moveVelocity.z = 0.0f;

        cameraTranslation.x += moveVelocity.x * deltaTime;
        cameraTranslation.y += moveVelocity.y * deltaTime;
        cameraTranslation.z += moveVelocity.z * deltaTime;
    }

    void lerpRotation(Quaternion& currentRot, Quaternion targetRot, float lerpSpeed, float deltaTime)
    {
        float dot = currentRot.w * targetRot.w + currentRot.x * targetRot.x + currentRot.y * targetRot.y + currentRot.z * targetRot.z;
        if (dot < 0.0f) {
            targetRot.w = -targetRot.w;
            targetRot.x = -targetRot.x;
            targetRot.y = -targetRot.y;
            targetRot.z = -targetRot.z;
        }

        float t = lerpSpeed * deltaTime;
        currentRot.w = currentRot.w + (targetRot.w - currentRot.w) * t;
        currentRot.x = currentRot.x + (targetRot.x - currentRot.x) * t;
        currentRot.y = currentRot.y + (targetRot.y - currentRot.y) * t;
        currentRot.z = currentRot.z + (targetRot.z - currentRot.z) * t;

        float mag = sqrtf(currentRot.w * currentRot.w + currentRot.x * currentRot.x + currentRot.y * currentRot.y + currentRot.z * currentRot.z);
        if (mag > 0.0f) {
            float inv = 1.0f / mag;
            currentRot.w *= inv;
            currentRot.x *= inv;
            currentRot.y *= inv;
            currentRot.z *= inv;
        }
    }

};