#ifndef HERO_GOJU_H_INCLUDED
#define HERO_GOJU_H_INCLUDED
#include "Hero.h"

// 五條悟英雄（名字可以自己改）
// 具體行為與貼圖路徑在這裡定義
class HeroGoju : public Hero
{
public:
    explicit HeroGoju(PlayerID pid);
    ~HeroGoju() override = default;

    void init() override;
    void update() override;

    // 英雄之間互打的行為在這裡定義
    void interact(Object* other) override;

protected:
    // 根據狀態＋frame index 組成正確的圖檔路徑
    std::string get_image_path(HeroState state,int frame_index) const override;
private:
    bool  blue_spawned_this_punch = false; // 這次 PUNCH 是否已生成藍洞
    int  normal_attack_counter = 0; // 普攻累積次數（第 5 次產生藍洞）
    bool pending_red_shot = false;  // 紅彈是否還沒射出

    double shoot_end_time = 0.0;
    bool ultimate_beam_spawned = false;
    // ⭐ 新增：大招維持到什麼時間點
    double ultimate_end_time = 0.0;

    bool punch_sfx_played = false;// 普攻音效是否已播放
    
    
};

#endif // HERO_GOJU_H_INCLUDED
