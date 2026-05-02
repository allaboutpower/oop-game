# 🕹️ C++ OOP Game Project

### 📌 Project Overview
This project is a C++ Object-Oriented Programming (OOP) game system that implements a hero-based combat framework.  
Each hero is designed with three core abilities: a basic attack, a ranged attack, and an ultimate skill.  

The system is built with scalability in mind, allowing new characters to be easily extended through inheritance.

---

### 🧠 Core Concepts
- Object-Oriented Programming (OOP)
- Inheritance
- Polymorphism
- Encapsulation
- Modular Design

---

### 🧩 Base Class: Hero
The `Hero` class defines the fundamental structure of all characters.

It includes:
- Basic attributes (e.g., stats, state)
- Three core skills:
  - Basic Attack
  - Ranged Attack
  - Ultimate Skill

This class serves as the foundation for all derived hero characters.

---

### 🔹 Derived Classes
The project includes multiple hero characters that inherit from the `Hero` base class:

- `HeroGoju`
- `HeroNaruto`

Each derived class:
- Overrides skill implementations
- Customizes attack behavior
- Demonstrates polymorphism in gameplay logic

---

### 🔹 Projectile System
The `HeroProjectile` class handles ranged attack mechanics.

It is responsible for:
- Projectile creation
- Movement logic
- Interaction handling

This separates attack behavior from the hero class, improving modularity.

---

### 📂 File Structure


---

### ⚙️ Features
- Extensible hero system using inheritance
- Independent skill implementation per character
- Reusable projectile module
- Clean separation between core logic and extensions

---

### 🚀 What I Learned
- Designing class hierarchies in C++
- Applying polymorphism to real use cases
- Structuring multi-file C++ projects
- Building reusable and maintainable systems

---

### 📈 Future Improvements
- Add more hero characters
- Implement skill cooldown system
- Improve collision detection
- Integrate with a full game loop or engine

---

### 👨‍💻 Summary
This project demonstrates how OOP principles can be applied to build a flexible and scalable game system.  
New heroes and features can be added with minimal changes to the existing architecture.
