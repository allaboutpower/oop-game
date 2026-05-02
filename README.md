# 🕹️ C++ OOP 遊戲系統  
Hero-Based Combat Engine



## 📌 專案簡介
本專案為一個基於 **C++ 物件導向（OOP）設計的 2D 戰鬥遊戲系統**，實作角色控制、技能系統與戰鬥機制。  

核心架構透過：
- 繼承（Inheritance）
- 多型（Polymorphism）
- 模組化設計（Modular Design）

建立一個**高擴充性（Extensible）且低耦合（Low Coupling）**的遊戲系統。

系統支援：
- 多角色（Hero）擴展  
- 技能客製化  
- 投射物（Projectile）系統  
- 狀態機控制（State Machine）  

---

## 🧠 系統架構

本系統採用分層與模組化設計，主要包含以下核心組件：

---

### 🔹 1. 核心角色系統（Hero System）

- `Hero.h / Hero.cpp`

功能：
- 定義所有角色共用邏輯
- 包含移動、跳躍、受擊、攻擊等基礎行為
- 管理角色狀態（State Machine）
- 控制動畫與 hitbox

👉 為整個遊戲的「核心引擎」

---

### 🔹 2. 角色擴展層（Derived Heroes）

- `HeroGoju.cpp / HeroGoju.h`
- `HeroNaruto.cpp / HeroNaruto.h`

功能：
- 繼承 `Hero` 類別
- 覆寫技能邏輯（update / interact）
- 定義角色專屬攻擊與大招

👉 透過 OOP 實現角色差異化

---

### 🔹 3. 投射物系統（Projectile System）

- `HeroProjectile.cpp / HeroProjectile.h`

功能：
- 管理所有遠程攻擊與技能效果
- 支援多種類型攻擊（紅球、光束、螺旋丸等）
- 處理移動、碰撞與生命週期

👉 將技能邏輯從角色中解耦（Decoupling）

---

### 🔹 4. 遊戲控制流程（Game Loop）

- 主流程負責：
  - 輸入處理
  - 更新角色狀態
  - 更新投射物
  - 繪製畫面

👉 所有物件共享統一 update/draw pipeline

---

## ⚙️ 系統實體狀態（Entity States）

系統透過以下狀態維護角色與戰鬥邏輯：

---

### 🔹 角色狀態（Hero State）

```cpp
enum class HeroState {
    IDLE,
    MOVING,
    PUNCH1,
    JUMPING,
    DEFENDING,
    SHOOT,
    ULTIMATE,
    DEAD
};
