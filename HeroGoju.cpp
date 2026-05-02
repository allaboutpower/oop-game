#include "HeroGoju.h"
#include "HeroProjectile.h"
#include "../data/DataCenter.h"
#include "../data/SoundCenter.h"
#include "../shapes/Rectangle.h"
#include <allegro5/allegro.h>
#include <string>

using std::string;
using std::to_string;

HeroGoju::HeroGoju(PlayerID pid)
    : Hero(pid)
{
    // 根據你的素材設定對應的 frame index
    for (auto &vec : bitmap_img_ids) vec.clear();

    bitmap_img_ids[(int)HeroState::IDLE]      = {1,2,3,4};
    bitmap_img_ids[(int)HeroState::MOVING]    = {1,2,3,4};
    bitmap_img_ids[(int)HeroState::PUNCH1]    = {1,2};
    bitmap_img_ids[(int)HeroState::JUMPING]   = {1};
    bitmap_img_ids[(int)HeroState::HITSTUN]   = {1}; // 受擊動作
    bitmap_img_ids[(int)HeroState::DEFENDING] = {1};
    bitmap_img_ids[(int)HeroState::SHOOT]     = {1}; // 紅彈施放動作
    bitmap_img_ids[(int)HeroState::ULTIMATE]  = {1,2,3,4,5,6,7,8,9};  // 大招施放動作
    bitmap_img_ids[(int)HeroState::DEAD]      = {1};

    maxHP = 100;
    HP    = maxHP;
    maxMP = 100;
    MP    = 0;
    speed = 8.0;

    pending_red_shot        = false;
    blue_spawned_this_punch = false;
    ultimate_beam_spawned = false;
}

// 根據狀態回傳正確的圖片路徑
std::string HeroGoju::get_image_path(HeroState st, int frame_index) const {
    if (frame_index <= 0) frame_index = 1;

    switch (st) {
        case HeroState::IDLE: {
            int idx = frame_index;
            if (idx < 2 || idx > 4) idx = 2;
            return "assets/image/Goju/Goju_idle" + to_string(idx) + ".png";
        }
        case HeroState::MOVING:
            return "assets/image/Goju/Goju_move.png";

        // ⭐ 所有 PUNCH 動作都用同一張圖
        case HeroState::PUNCH1:
            return "assets/image/Goju/Goju_punch.png";
        
        case HeroState::HITSTUN:
            // ✅ 被打到往後仰僵直的那張圖
            return "assets/image/Goju/Goju_hitstun.png";

        case HeroState::JUMPING:
            return "assets/image/Goju/Goju_jump.png";
        case HeroState::DEFENDING:
            return "assets/image/Goju/Goju_defend.png";

        case HeroState::SHOOT:
            // 施放紅彈的預備動作
            return "assets/image/Goju/Goju_red.png";

        case HeroState::ULTIMATE: {
            int idx = frame_index;
            if (idx < 1) idx = 1;
            if (idx > 9) idx = 9;
            return "assets/image/Goju/Goju_ult_pose" + to_string(idx) + ".png";
        }

        case HeroState::DEAD:
            return "assets/image/Goju/Goju_dead.png";

        default:
            return "assets/image/Goju/Goju_idle1.png";
    }
}

void HeroGoju::init() {
    Hero::init();
}

void HeroGoju::update() {
    DataCenter* DC = DataCenter::get_instance();
    // ========= 0. 普攻音效 =========
    if(state==HeroState::PUNCH1&&!punch_sfx_played){
        SoundCenter::get_instance()->play("./assets/sound/Goju_blue.mp3",ALLEGRO_PLAYMODE_ONCE);
        punch_sfx_played = true;
    }else if(state!=HeroState::PUNCH1){
        punch_sfx_played = false;
    }

    // ========= 1. 大招（紫色光束） =========
    int key_ult;//
        if(player_id==PlayerID::PLAYER1){
            key_ult = ALLEGRO_KEY_U;
        }else{
            key_ult = ALLEGRO_KEY_M;
        }

    //前一偵是否按過大招
    bool ult_pressed = DC->key_state[key_ult] && !DC->prev_key_state[key_ult];

    if (ult_pressed && MP >= 100 && state != HeroState::DEAD && shape)
    {
        MP = 0;  // 清空 MP
        SoundCenter::get_instance()->play("./assets/sound/Goju_ult.mp3",ALLEGRO_PLAYMODE_ONCE);
        state = HeroState::ULTIMATE;
        frame_id = 0;
        frame_switch_counter = frame_switch_freq;  // 這裡如果想更快可以之後改成 6
        attack_has_hit = false;

        // 還沒生成光束
        ultimate_beam_spawned = false;

        // ★ 不要在這裡 new HeroProjectile，也不要 return
        //    讓後面的 Hero::update() 幫你跑動畫（frame_id 會從 0 播到 8）
    }

    // ========= 2. 遠程紅彈 =========
    int key_proj = (player_id == PlayerID::PLAYER1)
                   ? ALLEGRO_KEY_I
                   : ALLEGRO_KEY_N;

    bool red_pressed =
        DC->key_state[key_proj] && !DC->prev_key_state[key_proj];

    if (red_pressed && MP >= 10 && state != HeroState::DEAD) {

        MP -= 10;  // 扣 MP
        SoundCenter::get_instance()->play("./assets/sound/Goju_red.mp3",ALLEGRO_PLAYMODE_ONCE);
        state = HeroState::SHOOT;
        frame_id = 0;
        frame_switch_counter = frame_switch_freq;
        pending_red_shot = false;

        shoot_end_time = al_get_time() + 0.5;//// ★★ 初始化 SHOOT 的結束時間（最重要的部分）

        auto* r = dynamic_cast<Rectangle*>(shape.get());
        double cx = r->center_x();
        double cy = r->center_y();
        double dir = facing_right ? 1.0 : -1.0;

        // 紅球從角色前方一點的位置射出
        HeroProjectile* proj = new HeroProjectile(
            this,
            cx + dir * 350,
            cy-250,
            dir,
            10.0,             // 飛行速度
            1,                // 傷害
            ProjectileKind::RED_SHOT,
            2.0               // 存活時間（秒）
        );

        DC->hero_projectiles.push_back(proj);

        update_attack_box();    
        update_bitmap_from_state();
        return;
    }
    if (state == HeroState::SHOOT) {
            if (al_get_time() >= shoot_end_time) {
                state = HeroState::IDLE;
            }
     }


    // ========= 3. 其餘基本行為交給共用 Hero =========
    Hero::update();
     // ========= 3-1. 如果正在放大招，播到第 9 張 pose 時才生成光束 =========
    if (state == HeroState::ULTIMATE && !ultimate_beam_spawned && shape) {
        // bitmap_img_ids[ULTIMATE] = {1..9}，frame_id 從 0~8
        // 播到最後一張（frame_id >= 8）時生成 projectile
        if (frame_id >= 8) {
            auto* r = dynamic_cast<Rectangle*>(shape.get());
            double cx = center_x();
            double cy = center_y();
            double dir = facing_right ? 1.0 : -1.0;

            // 光束從角色前方邊緣開始
            double spawn_x = facing_right ? r->x2 : r->x1;
            double spawn_y = cy - 150.0;

            double beam_life = 2.5;   // 光束存活秒數（跟 HeroProjectile 那邊一樣）
            
            HeroProjectile* beam = new HeroProjectile(
                this,
                spawn_x,
                spawn_y,
                dir,
                0.0,                    // 光束不需要速度
                5,                      // 每次 tick 傷害
                ProjectileKind::ULTIMATE_PURPLE,
                2.5                     // 光束存在時間（秒）
            );

            DataCenter::get_instance()->hero_projectiles.push_back(beam);
            ultimate_beam_spawned = true; // 避免重複生成
            // ⭐ 記錄「大招結束時間」
            ultimate_end_time = al_get_time() + beam_life;
        }
    }
        // ========= ULTIMATE 結束判定 =========
    if (state == HeroState::ULTIMATE && ultimate_beam_spawned) {
        double now = al_get_time();
        if (now >= ultimate_end_time) {
            // 光束結束 → 英雄回到 idle
            state = HeroState::IDLE;
            frame_id = 0;
            frame_switch_counter = frame_switch_freq;
            ultimate_beam_spawned = false;
        } else {
            // 在光束持續期間，確保一直停在最後一張 pose
            int idx = static_cast<int>(HeroState::ULTIMATE);
            auto &frames = bitmap_img_ids[idx];
            if (!frames.empty()) {
                frame_id = (int)frames.size() - 1;
            }
        }
    }
    // ========= 4. 藍色場地（Goju_blue） =========
    // 說明：
    // PUNCH 的 5 張圖對應：
    //  frame_id = 0 → Goju_punch1（出拳，有 hitbox)
    //  frame_id = 3 → Goju_punch4（召喚藍洞）
    //  frame_id = 4 → Goju_punch5（召喚藍洞持續）
    //
    // 我們設計成：只要這次 PUNCH 播到第 4 幀以後（frame_id >= 3），
    // 並且還沒生成過藍洞，就在角色前方生成一個 BLUE_FIELD。
    if (state != HeroState::PUNCH1) {
        blue_spawned_this_punch = false;
        return;
    }

    // 這次出拳還沒生過 blue，而且動畫已經開始（frame_id >= 0）
    if (!blue_spawned_this_punch && frame_id >= 0 && shape) {
        auto* r = dynamic_cast<Rectangle*>(shape.get());
        double cx = r->center_x();
        double cy = r->y2;          // 腳底

        double dir = facing_right ? 1.0 : -1.0;

        HeroProjectile* field = new HeroProjectile(
            this,
            cx + dir * 40.0,        // 角色前方一點
            cy -500,               // 往上拉，避免埋到地板裡
            0.0,
            0.0,
            5,
            ProjectileKind::BLUE_FIELD,
            3.0                      // 存在 3 秒
        );

        DataCenter::get_instance()->hero_projectiles.push_back(field);
        blue_spawned_this_punch = true;   // 這次出拳只生一次
    }
}

// 英雄互打行為：由 OperationCenter 判定 hit 後呼叫這裡
void HeroGoju::interact(Object* other) {
    Hero* target = dynamic_cast<Hero*>(other);
    if (!target) return;

    // 只有在 PUNCH1 期間，近身 hitbox 才會造成傷害
    if (state != HeroState::PUNCH1) return;
    
    int dmg = 5;   // 想多痛自己調

    double my_x  = center_x();
    double tar_x = target->center_x();
    double dir   = (tar_x >= my_x) ? 1.0 : -1.0;

    target->take_damage(dmg, dir);
}
