#ifndef HERO_PROJECTILE_H_INCLUDED
#define HERO_PROJECTILE_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include <memory>

class Hero;   // 前向宣告

// 投射物種類
enum class ProjectileKind {
    RED_SHOT,         // Goju_red1：往前飛
    BLUE_FIELD,       // Goju_blue1/2：停在地上 3 秒
    ULTIMATE_PURPLE,  // Goju 大招：紫色光束

    RASENGAN,         // Naruto 螺旋丸：水平射出
    NARUTO_CLONE      // Naruto 影分身：衝刺連段用的分身投射物
};

// 給英雄用的遠程能量球 Projectile
class HeroProjectile : public Object
{
public:
    // dir = -1 (向左) 或 1 (向右)
    HeroProjectile(Hero* owner,
                   double x, double y,
                   double dir,
                   double speed,
                   int damage,
                   ProjectileKind kind,
                   double life_seconds);

    ~HeroProjectile() override = default;

    void update();
    void draw();

    bool is_alive() const { return alive; }
    void kill()          { alive = false; }

    int   get_damage() const { return damage; }
    Hero* get_owner()  const { return owner;  }

    ProjectileKind get_kind() const { return kind; }

    // 給光束用：記錄對各英雄上次造成傷害的時間
    double& last_hit_time_for_hero1() { return last_hit_time_hero1; }
    double& last_hit_time_for_hero2() { return last_hit_time_hero2; }

private:
    Hero*  owner;   // 發射者（1P 或 2P）
    double vx;
    double vy;
    int    damage;
    bool   alive;

    ProjectileKind kind;
    double spawn_time;    // 產生時間
    double life_time;     // 可以活多久（秒）

    // 專給 ULTIMATE_PURPLE 用：對每個英雄的上次傷害時間
    double last_hit_time_hero1 = 0.0;
    double last_hit_time_hero2 = 0.0;
};

#endif // HERO_PROJECTILE_H_INCLUDED
