# 🕹️ C++ OOP Game System (Hero-Based Combat Engine)

---

## 📌 Project Overview
This project implements a **hero-based combat system in C++** using Object-Oriented Programming (OOP).  

The system separates:
- Core character logic
- Skill execution
- Combat interaction (hitbox / projectile)

Each hero shares a unified base structure while supporting **customized behaviors via inheritance**, resulting in a highly extensible and maintainable architecture.

---

## 🧠 Architecture Design (OOP Core)

The system is built around a **base class abstraction + polymorphism**:

- `Hero` → Core engine (movement, state machine, combat)
- `HeroGoju`, `HeroNaruto` → Derived classes (custom skills)
- `HeroProjectile` → Decoupled skill system

```cpp
class Hero : public Object {
public:
    virtual void update();
    virtual void draw();
    virtual void take_damage(int dmg, double dir);
    virtual void interact(Object* other) = 0;
};
