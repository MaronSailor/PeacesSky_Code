#pragma once

struct Vec3
{
	float x = 0, y = 0, z = 0;

	Vec3 operator+(const Vec3& other) const
	{
		return Vec3{ this->x + other.x, this->y + other.y, this->z + other.z };
	}

	Vec3 operator*(const float number) const {
		return Vec3{ this->x * number, this->y * number, this->z * number };
	}

};

struct Vec2
{
	float x = 0, y = 0;
};

struct Vec4Color
{
	float r = 1, g = 1, b = 1, a = 1;
};

struct Quaternion
{
	float w = 0, x = 0, y = 0, z = 0;

	Quaternion operator*(const Quaternion& other) const
	{
		return Quaternion{
			this->w * other.w - this->x * other.x - this->y * other.y - this->z * other.z,
			this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y,
			this->w * other.y - this->x * other.z + this->y * other.w + this->z * other.x,
			this->w * other.z + this->x * other.y - this->y * other.x + this->z * other.w
		};
	}

	Quaternion operator*(const float other) const
	{
		return Quaternion{
			this->w * other,
			this->x * other,
			this->y * other,
			this->z * other
		};
	}
};

struct Aspect
{
	unsigned int x = 0, y = 0;
};