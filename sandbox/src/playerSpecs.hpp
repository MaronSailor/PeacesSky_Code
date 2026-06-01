#pragma once

#include "inventory.hpp"

struct PlayerSpecs
{
	float movementSpeed   = 1.0f;
	float mouseSensitivity = 0.5f;
	float jetStrength     = 0.005f;   // thrust per dT unit while jet-flying (airborne + space)
	float maxJetSpeed     = 0.3f;     // terminal upward velocity for jet
	float jetDecay        = 0.02f;    // deceleration per dT unit when jet is off
	float jumpStrength    = 0.8f;     // initial upward impulse on jump (was 2.0f → ~1-block height)
	float staminaRegen       = 0.05f; // regen per dT unit while airborne
	float staminaRegenGround = 0.25f;  // regen per dT unit while standing on ground

	float stamina    = 100;
	float maxStamina = 100;

	Inventory inventory;

	int points = 0;

	// Returns thrust when jet is active, 0 otherwise. Does NOT handle regen.
	float jetFly(const float key, float dT) {
		if (stamina > 1.0f && key) {
			this->stamina -= 0.25f * dT;
			return this->jetStrength * dT;
		}
		return 0;
	}

	// Call once per frame regardless of ground state
	void tickStamina(bool usingJet, bool onGround, float dT) {
		if (!usingJet && onGround && stamina < maxStamina)
			stamina += staminaRegenGround * dT;
	}
};