#ifndef HERO_NARUTO_H_INCLUDED
#define HERO_NARUTO_H_INCLUDED

#include "Hero.h"

// 漩渦鳴人英雄
// 三招：
// 1) 普攻：同 Goju（由共用 Hero::update + interact 的近身傷害處理）
// 2) 遠程：螺旋丸（SHOOT）生成 projectile 水平射出
// 3) 大招：影分身（ULTIMATE）生成多個鳴人外觀 projectile 衝向對手形成聯招
class HeroNaruto : public Hero
{
public:
    explicit HeroNaruto(PlayerID pid);
    ~HeroNaruto() override = default;
    void init() override;
    void update() override;
    void interact(Object* other) override;

protected:
    std::string get_image_path(HeroState state, int frame_index) const override;

private:
    // 普攻音效
    bool punch_sfx_played = false;

    // 遠程（螺旋丸）
    double shoot_end_time = 0.0;

    // 大招（影分身）
    bool   ultimate_clone_started = false;   // 到最後 pose 才開始生分身
    int    clone_spawned = 0;                // 已生幾個
    double next_clone_spawn_time = 0.0;      // 下一次生分身的時間點
    double ultimate_end_time = 0.0;          // 大招結束時間點（到點回 IDLE）
};

#endif // HERO_NARUTO_H_INCLUDED
