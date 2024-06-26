#pragma once
#include <cstdint>
#include "../networking/net.h"

/// <summary>
/// Contains common definitions and includes used across the whole framework.
/// More importantly, it defines the available message types for the server, the client, and the game,
/// as well as the elements that describe each player.
/// Also contains helper classes/structs.
/// </summary>

struct sVector2
{
	float x;
	float y;

	sVector2(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y) {}

	// Scalar multiplication
	sVector2 operator*(float scalar) const
	{
		return { x * scalar, y * scalar };
	}

	// Vector addition
	sVector2 operator+(const sVector2& other) const
	{
		return { x + other.x, y + other.y };
	}

	// Vector subtraction
	sVector2 operator-(const sVector2& other) const
	{
		return { x - other.x, y - other.y };
	}

	// Vector multiplication by a scalar (reverse order)
	friend sVector2 operator*(float scalar, const sVector2& vec)
	{
		return vec * scalar;
	}

	// Overloaded operators for vector arithmetic
	sVector2& operator+=(const sVector2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	sVector2& operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	sVector2& operator-=(const sVector2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	// Length (magnitude)
	float mag() const
	{
		return std::sqrt(mag2());
	}

	// Length squared
	float mag2() const
	{
		return x * x + y * y;
	}

	// Normalize
	sVector2 norm() const
	{
		float length = mag();
		if (length != 0.0f)
		{
			return { x / length, y / length };
		}
		else
		{
			return { 0.0f, 0.0f };
		}
	}

	// Floor
	sVector2 floor() const
	{
		return { std::floor(x), std::floor(y) };
	}

	// Min
	sVector2 min(const sVector2& other) const
	{
		return { std::min(x, other.x), std::min(y, other.y) };
	}

	// Max
	sVector2 max(const sVector2& other) const
	{
		return { std::max(x, other.x), std::max(y, other.y) };
	}
};

struct Color
{
	uint8_t r, g, b;

	Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {}
};

enum class GameMsg : uint32_t
{
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignID,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_AddPlayer,
	Game_RemovePlayer,
	Game_UpdatePlayer,
};

struct sPlayerDescription
{
	uint32_t nUniqueID = 0;
	uint32_t nSize = 20;
	Color nColor;

	uint32_t nOreCount = 0;
	float fMiningSpeed = 1.0f;

	sVector2 vPos;
	sVector2 vVel;
};