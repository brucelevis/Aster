#pragma once

class Renderer;

class World
{
 public:
    World();
    void run();
 
 private:
    void step(const float dt);
 
 private:
    Renderer* renderer;
};