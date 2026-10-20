#include "Vec2.hpp"

#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace std;

int maxBallRadius = 50;
int ballAmount = 0;
int substeps = 20;
Vec2 gravity = {0, 0.1};

int interactionRadius = 20;
int cellSize = 20;
float targetDensity = 3;
float correctionStrength = 0.1;

int ballsToBeSpawned = 4000;

sf::RenderWindow window(sf::VideoMode(0, 0), "MY Physics Engine!",
                        sf::Style::Fullscreen);
sf::Vector2u winsize = window.getSize();
Vec2 WINDOW_SIZE = Vec2(winsize.x, winsize.y);

Vec2 constraintSize = Vec2(1300, 900);
Vec2 constraintPosition = WINDOW_SIZE / 2 + constraintSize / 2;
float cleft = constraintPosition.x - constraintSize.x;
float cright = constraintPosition.x;
float ctop = constraintPosition.y - constraintSize.y;
float cbottom = constraintPosition.y;

struct Slider {
  sf::RectangleShape middleRect;
  sf::RectangleShape frontRect;
  sf::RectangleShape backRect;
  float sidesOffset;
  float maxWidth;
  float maxValue;
  float minValue;
  float value;
  Vec2 size;

  Slider(int index, float _maxValue, float startingValue, float _minValue = 0) {
    maxValue = _maxValue;
    minValue = _minValue;
    value = startingValue;

    size = Vec2(170, 50);
    Vec2 position = Vec2(10, 10 + ((size.y + 10) * index));
    sidesOffset = 10;

    backRect.setSize(sf::Vector2f(size.x, size.y));
    backRect.setPosition(position.x, position.y);
    backRect.setFillColor(sf::Color(50, 50, 50));

    maxWidth = size.x - sidesOffset * 2;

    middleRect.setSize(sf::Vector2f(maxWidth, size.y - sidesOffset * 2));
    middleRect.setPosition(position.x + sidesOffset, position.y + sidesOffset);
    middleRect.setFillColor(sf::Color(100, 100, 100));

    frontRect = middleRect;
    frontRect.setFillColor(sf::Color(255, 240, 31));
    updateSprite();
  }

  void updateSprite() {
    frontRect.setSize(sf::Vector2f(
        maxWidth * ((value - minValue + 1) / (maxValue - minValue + 1)),
        size.y - sidesOffset * 2));
  }

  void changeValue(float val) {
    value = val;
    updateSprite();
  }

  void add(int n) {
    if (value < maxValue) {
      value += n;
      updateSprite();
    }
  }

  void subtract(int n) {
    if (value > minValue) {
      value -= n;
      updateSprite();
    }
  }

  void draw(sf::RenderWindow &window) {
    window.draw(backRect);
    window.draw(middleRect);
    window.draw(frontRect);
  }
};

struct Ball {
  Vec2 position;
  Vec2 oldPosition;
  Vec2 velocity;
  Vec2 acceleration;
  sf::CircleShape sprite;

  bool isPivot;
  float radius;
  float density = 0;

  Ball() {}

  Ball(Vec2 _position, float _radius, sf::Color _color, bool _isPivot = false) {
    radius = _radius;
    position = _position;
    oldPosition = _position;
    isPivot = _isPivot;

    sprite = sf::CircleShape();
    sprite.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
    sprite.setFillColor(_color);
    sprite.setRadius(radius);
  }

  void update() {
    if (isPivot)
      position = oldPosition;

    velocity = position - oldPosition;

    oldPosition = position;

    position = position + velocity + acceleration;

    acceleration = gravity;

    sprite.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
  }
};

struct Link {
  float targetDist;
  int obj1_i;
  int obj2_i;
  bool pivot;

  Link(int obj1_i_, int obj2_i_, float _targetDist) {
    targetDist = _targetDist;
    obj1_i = obj1_i_;
    obj2_i = obj2_i_;
  }

  void update(vector<Ball> *balls) {
    Vec2 dist = (*balls)[obj1_i].position - (*balls)[obj2_i].position;
    float absDist = dist.euclideanDistance();
    Vec2 direction = dist / absDist;

    Vec2 nudge = direction * ((targetDist - absDist) / 2);
    (*balls)[obj1_i].position += nudge;
    (*balls)[obj2_i].position -= nudge;
  }
};

Slider radiusSlider = Slider(0, maxBallRadius, 7, 1);
Slider densitySlider = Slider(1, 10, targetDensity, 0);
Slider strengthSlider = Slider(2, 10, correctionStrength, 0);
vector<Ball> buffered_balls;
vector<Ball> balls;

unordered_map<long long, vector<int>> grid;
long myhash(int x, int y) { return ((long long)x << 32) ^ (long long)y; };

sf::Color randomColor() {
  return sf::Color(rand() % 155 + 100, rand() % 155 + 100, rand() % 155 + 100);
}

float spikyKernel(float dist, float interactionRadius) {
  return (1 - pow(dist / interactionRadius, 3.0f));
}

void loop() {
  vector<Ball> animation_balls;
  /* float angle = 0; */
  /* ballsToBeSpawned /= 2; */
  /* for (int i = 0; i < ballsToBeSpawned; i++) */
  /* { */
  /*     animation_balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 - cos(angle) *
   * radius + 20, WINDOW_SIZE.y / 2 - sin(angle) * radius + 20), 15,
   * sf::Color(randomColor()))); */
  /*     animation_balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 + cos(angle) *
   * radius - 20, WINDOW_SIZE.y / 2 + sin(angle) * radius - 20), 15,
   * sf::Color(randomColor()))); */
  /*     angle += 0.1; */
  /*     radius -= 0.5; */
  /* } */

  float radius = 1.5;
  float y = constraintPosition.y - 10;
  float x = constraintPosition.x - constraintSize.x + 10;
  for (int i = 0; i < ballsToBeSpawned; i++) {
    x += interactionRadius;
    if (x > constraintPosition.x - 10) {
      x = constraintPosition.x - constraintSize.x + 10;
      y -= interactionRadius;
      if (y < constraintPosition.y - constraintSize.y + 10) {
        y = constraintPosition.y - 10;
      }
    }

    balls.push_back(Ball(Vec2(x + rand() % 2, y), radius + rand() % 2 * 0.1,
                         sf::Color::Cyan));
  }

  int ropeBalls = 0;
  vector<Link> links;
  for (int i = 0; i < ropeBalls; i++) {
    balls.push_back(Ball(Vec2(550 + i * 20, WINDOW_SIZE.y / 2), 10,
                         sf::Color(randomColor())));
    if (i != 0)
      links.push_back(Link(balls.size() - 2, balls.size() - 1, 25));
  }

  if (links.size() > 0) {
    balls[links[0].obj1_i].isPivot = true;
    balls[links.back().obj2_i].isPivot = true;
  }

  ballAmount = balls.size();
  int linkAmount = links.size();

  const int targetFPS = 60;
  const sf::Time targetFrameTime = sf::seconds(1.f / targetFPS);
  sf::Clock clock;
  sf::Time dt;

  int animationStep = 0;
  while (window.isOpen()) {
    dt = clock.restart();

    if (animation_balls.size() > 0) {
      if (animationStep > 10) {
        animationStep = 0;
        balls.push_back(animation_balls.back());
        animation_balls.pop_back();
        ballAmount++;
      }
      animationStep++;
    }
    int num_buffered_balls = buffered_balls.size();
    for (int i = 0; i < num_buffered_balls; i++) {
      balls.push_back(buffered_balls.back());
      buffered_balls.pop_back();
      ballAmount++;
    }

    for (int _ = 0; _ < substeps; _++) {
      bool push = true;
      if ((push = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) ||
          (push != sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) {
        int controlDistance = 100;
        float restDist = 20.0f; // stable "orbit" radius
        float strength = 20;

        sf::Vector2i sfMouse = sf::Mouse::getPosition();
        Vec2 mouse(sfMouse.x, sfMouse.y);

        int cx = floor(mouse.x / cellSize);
        int cy = floor(mouse.y / cellSize);
        int radius = ceil(controlDistance / cellSize);
        for (int dx = -radius; dx <= radius; dx++) {
          for (int dy = -radius; dy <= radius; dy++) {
            long long ckey =
                ((long long)(cx + dx) << 32) ^ (long long)(cy + dy);
            if (!grid.count(ckey))
              continue;

            for (int i : grid[ckey]) {
              Vec2 distVec = balls[i].position - mouse;
              float dist = distVec.euclideanDistance();
              if (dist < controlDistance && dist > 1e-3f) {
                Vec2 dir = distVec / dist;
                if (!push)
                  dir *= -1;
                balls[i].position += dir;
              }
            }
          }
        }
      }
      bool increase = true;
      if ((increase = sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) ||
          sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        if (increase)
          targetDensity += 0.1f;
        else
          targetDensity -= 0.1f;
        targetDensity = std::clamp(targetDensity, 0.1f, 10.0f);
        densitySlider.changeValue(targetDensity);
      }
      if ((increase = sf::Keyboard::isKeyPressed(sf::Keyboard::A)) ||
          sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        if (increase)
          correctionStrength += 0.1f;
        else
          correctionStrength -= 0.1f;
        correctionStrength = std::clamp(correctionStrength, 0.1f, 10.0f);
        strengthSlider.changeValue(correctionStrength);
      }

      for (int i = 0; i < linkAmount; i++) {
        links[i].update(&balls);
      }

      grid.clear();
      for (int i = 0; i < ballAmount; i++) {
        int cx = floor(balls[i].position.x / cellSize);
        int cy = floor(balls[i].position.y / cellSize);
        long long key = ((long long)cx << 32) ^ (long long)cy;
        grid[key].push_back(i);
      }

      // density calcuation
      for (int i = 0; i < ballAmount; i++) {
        balls[i].density = 0;
        int cx = floor(balls[i].position.x / cellSize);
        int cy = floor(balls[i].position.y / cellSize);
        for (int dx = -1; dx <= 1; dx++) {
          for (int dy = -1; dy <= 1; dy++) {
            long long ckey =
                ((long long)(cx + dx) << 32) ^ (long long)(cy + dy);
            if (!grid.count(ckey))
              continue;

            for (int j : grid[ckey]) {
              if (j == i)
                continue;

              Vec2 distVec = balls[i].position - balls[j].position;
              float dist = distVec.euclideanDistance();
              if (dist < interactionRadius) {
                balls[i].density += spikyKernel(dist, interactionRadius);
              }
            }
          }
        }

        // if close to wall increase density
        float dLeft = (balls[i].position.x - interactionRadius) - cleft;
        float dRight = cright - (balls[i].position.x + interactionRadius);
        float dTop = (balls[i].position.y - interactionRadius) - ctop;
        float dBottom = cbottom - (balls[i].position.y + interactionRadius);
        auto wallDensity = [&](float dist) {
          return (dist < interactionRadius) ? 1.0f - dist / interactionRadius
                                            : 0.0f;
        };
        balls[i].density += wallDensity(dLeft);
        balls[i].density += wallDensity(dRight);
        balls[i].density += wallDensity(dTop);
        balls[i].density += wallDensity(dBottom);

        /* auto lerp = [](sf::Uint8 a, sf::Uint8 b, float t) { */
        /*     return static_cast<sf::Uint8>(a + (b - a) * t); */
        /* }; */

        /* sf::Color lowColor(200, 0, 0);   // dim red */
        /* sf::Color highColor(0, 170, 255); // bright cyan */

        /* float t = std::clamp(balls[i].density / (targetDensity*2),
         * 0.0f, 1.0f); */

        /* sf::Uint8 r = lerp(lowColor.r, highColor.r, t); */
        /* sf::Uint8 g = lerp(lowColor.g, highColor.g, t); */
        /* sf::Uint8 b = lerp(lowColor.b, highColor.b, t); */

        /* balls[i].sprite.setFillColor(sf::Color(r, g, b)); */

        float t =
            std::clamp(balls[i].density / (targetDensity * 2), 0.0f, 1.0f);
        sf::Uint8 b = static_cast<sf::Uint8>(255 * t);
        sf::Uint8 r = static_cast<sf::Uint8>(255 * (0.5f - t));
        balls[i].sprite.setFillColor(sf::Color(r, 0, b));
      }

      // density correction
      for (int i = 0; i < ballAmount; i++) {
        // constrain inside rectangle
        if (balls[i].position.x < cleft + interactionRadius) {
          balls[i].oldPosition.x = balls[i].position.x;
          balls[i].position.x = cleft + interactionRadius;
        } else if (balls[i].position.x > cright - interactionRadius) {
          balls[i].oldPosition.x = balls[i].position.x;
          balls[i].position.x = cright - interactionRadius;
        }
        if (balls[i].position.y < ctop + interactionRadius) {
          balls[i].oldPosition.y = balls[i].position.y;
          balls[i].position.y = ctop + interactionRadius;
        } else if (balls[i].position.y > cbottom - interactionRadius) {
          balls[i].oldPosition.y = balls[i].position.y;
          balls[i].position.y = cbottom - interactionRadius;
        }

        int cx = floor(balls[i].position.x / cellSize);
        int cy = floor(balls[i].position.y / cellSize);
        for (int dx = -1; dx <= 1; dx++) {
          for (int dy = -1; dy <= 1; dy++) {
            long long ckey =
                ((long long)(cx + dx) << 32) ^ (long long)(cy + dy);
            if (!grid.count(ckey))
              continue;

            for (int j : grid[ckey]) {
              if (j == i)
                continue;

              Vec2 distVec = balls[i].position - balls[j].position;
              float dist = distVec.euclideanDistance();

              if (dist < interactionRadius) {
                if (dist == 0) {
                  dist = 0.01;
                  distVec = Vec2(0.01, 0.01);
                }

                Vec2 dir = distVec / dist;
                float kernel = spikyKernel(dist, interactionRadius);
                float error = (balls[i].density / targetDensity - 1.0f) +
                              (balls[j].density / targetDensity - 1.0f);

                float correction = 0.5f * error * kernel * correctionStrength;
                balls[i].position += dir * correction;
                balls[j].position -= dir * correction;
              }
            }
          }
        }

        balls[i].update();
      }
    }

    sf::Time frameEnd = clock.getElapsedTime();
    sf::Time sleepTime = targetFrameTime - frameEnd;
    if (sleepTime > sf::Time::Zero) {
      sf::sleep(sleepTime);
    }
  }
}

int main() {
  window.setFramerateLimit(60);

  sf::RectangleShape constraintSprite = sf::RectangleShape();
  constraintSprite.setPosition(
      sf::Vector2f(constraintPosition.x - constraintSize.x,
                   constraintPosition.y - constraintSize.y));
  constraintSprite.setFillColor(sf::Color::Black);
  constraintSprite.setSize(sf::Vector2f(constraintSize.x, constraintSize.y));

  Ball spawningBall;
  sf::RectangleShape spawningLine;
  spawningLine.setPosition(sf::Vector2f(-100, -100));

  sf::Thread loop1(&loop);
  loop1.launch();

  sf::Event ev;
  while (window.isOpen()) {
    while (window.pollEvent(ev)) {
      if (ev.type == sf::Event::Closed)
        window.close();
      if (ev.type == sf::Event::KeyPressed)
        if (ev.key.code == sf::Keyboard::Escape)
          window.close();
      if (ev.type == sf::Event::MouseButtonPressed)
        if (ev.mouseButton.button == sf::Mouse::Right) {
          sf::Vector2i mouse = sf::Mouse::getPosition();
          spawningBall = Ball(Vec2(mouse.x, mouse.y), radiusSlider.value,
                              randomColor(), true);
          spawningLine.setPosition(sf::Vector2f(mouse.x, mouse.y));
        }
      if (ev.type == sf::Event::MouseButtonReleased)
        if (ev.mouseButton.button == sf::Mouse::Right) {
          spawningBall.isPivot = false;
          sf::Vector2i sfMouse = sf::Mouse::getPosition();
          Vec2 mouse = Vec2(sfMouse.x, sfMouse.y);
          Vec2 distance = (mouse - spawningBall.position) / 5;
          spawningBall.oldPosition = spawningBall.position + distance;
          buffered_balls.push_back(spawningBall);
        }
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
      sf::Vector2i mouse = sf::Mouse::getPosition();
      buffered_balls.push_back(
          Ball(Vec2(mouse.x, mouse.y), radiusSlider.value, randomColor()));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
      radiusSlider.add(1);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
      radiusSlider.subtract(1);
    }

    window.clear(sf::Color(30, 30, 30));

    window.draw(constraintSprite);

    for (int i = 0; i < ballAmount; i++) {
      window.draw(balls[i].sprite);
    }
    if (spawningBall.isPivot) {
      sf::Vector2i sfMouse = sf::Mouse::getPosition();
      Vec2 mouse = Vec2(sfMouse.x, sfMouse.y);
      Vec2 distance = mouse - spawningBall.position;
      float absDistance = distance.euclideanDistance();
      spawningLine.setSize(sf::Vector2f(absDistance, 3));

      float angle = atan2(distance.y, distance.x);
      spawningLine.setRotation(angle * 360 / (2 * 3.14159265358979323846));

      if (spawningLine.getPosition().x != -100)
        window.draw(spawningLine);
      window.draw(spawningBall.sprite);
    }

    radiusSlider.draw(window);
    densitySlider.draw(window);
    strengthSlider.draw(window);

    window.display();

    sf::sleep(sf::milliseconds(10));
  }

  return 0;
}
