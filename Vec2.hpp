#include <cmath>
#include <iostream>
#include <string>

struct Vec2 {
  float x, y;

  Vec2() {
    x = 0.0f;
    y = 0.0f;
  }

  Vec2(float x, float y) {
    this->x = x;
    this->y = y;
  }

#pragma region Operators

  Vec2 operator-() { return Vec2(-x, -y); }

  Vec2 operator+(Vec2 otherVec) { return Vec2(x + otherVec.x, y + otherVec.y); }

  Vec2 operator-(Vec2 otherVec) { return Vec2(x - otherVec.x, y - otherVec.y); }

  Vec2 operator*(Vec2 otherVec) { return Vec2(x * otherVec.x, y * otherVec.y); }

  Vec2 operator/(Vec2 otherVec) { return Vec2(x / otherVec.x, y / otherVec.y); }

  Vec2 operator+(float otherFlo) { return Vec2(x + otherFlo, y + otherFlo); }

  Vec2 operator-(float otherFlo) { return Vec2(x - otherFlo, y - otherFlo); }

  Vec2 operator*(float otherFlo) { return Vec2(x * otherFlo, y * otherFlo); }

  Vec2 operator/(float otherFlo) { return Vec2(x / otherFlo, y / otherFlo); }

  void operator+=(Vec2 otherVec) {
    x += otherVec.x;
    y += otherVec.y;
  }

  void operator-=(Vec2 otherVec) {
    x -= otherVec.x;
    y -= otherVec.y;
  }

  void operator*=(Vec2 otherVec) {
    x *= otherVec.x;
    y *= otherVec.y;
  }

  void operator/=(Vec2 otherVec) {
    x /= otherVec.x;
    y /= otherVec.y;
  }

  void operator+=(float otherFlo) {
    x += otherFlo;
    y += otherFlo;
  }

  void operator-=(float otherFlo) {
    x -= otherFlo;
    y -= otherFlo;
  }

  void operator*=(float otherFlo) {
    x *= otherFlo;
    y *= otherFlo;
  }

  void operator/=(float otherFlo) {
    x /= otherFlo;
    y /= otherFlo;
  }

  friend std::ostream &operator<<(std::ostream &stream, Vec2 &v) {
    stream << "(X: " << v.x << ", Y: " << v.y << ")";
    return stream;
  }

#pragma endregion Operators

  float mySqrt(float x) {
    unsigned int i = *(unsigned int *)&x;
    i += 127 << 23;
    i >>= 1;
    return *(float *)&i;
  }

  float euclideanDistance() { return mySqrt(x * x + y * y); }

  void print(std::string name = "") {
    if (name != "") {
      std::cout << name << ": "
                << "(X: " << x << ", Y: " << y << ")" << std::endl;
    } else {
      std::cout << "(X: " << x << ", Y: " << y << ")" << std::endl;
    }
  }
};
