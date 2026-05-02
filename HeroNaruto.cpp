#include "HeroNaruto.h"
#include "HeroProjectile.h"
#include "../data/DataCenter.h"
#include "../data/SoundCenter.h"
#include "../shapes/Rectangle.h"
#include <allegro5/allegro.h>
#include "../data/ImageCenter.h"
#include <string>
#include "../data/ImageCenter.h"
#include "../shapes/Rectangle.h"
#include <allegro5/allegro.h>

using std::string;
using std::to_string;

HeroNaruto::HeroNaruto(PlayerID pid)
    : Hero(pid)
{
    for (auto &vec : bitmap_img_ids) vec.clear();

    // 你素材幀數不一樣就改這裡（架構照 Goju）
    bitmap_img_ids[(int)HeroState::IDLE]      = {1,2,3,4};
    bitmap_img_ids[(int)HeroState::MOVING]    = {1,2};
    bitmap_img_ids[(int)HeroState::PUNCH1]    = {1,2};
    bitmap_img_ids[(int)HeroState::JUMPING]   = {1};
    bitmap_img_ids[(int)HeroState::HITSTUN]   = {1};
    bitmap_img_ids[(int)HeroState::DEFENDING] = {1};
    bitmap_img_ids[(int)HeroState::SHOOT]     = {1};                // 螺旋丸施放動作
    bitmap_img_ids[(int)HeroState::ULTIMATE]  = {1,2,3}; // 影分身施放動作
    bitmap_img_ids[(int)HeroState::DEAD]      = {1};

    maxHP = 100;
    HP    = maxHP;
    maxMP = 100;
    MP    = 0;
    speed = 8.0;

    punch_sfx_played = false;

    ultimate_clone_started = false;
    clone_spawned = 0;
    next_clone_spawn_time = 0.0;
    ultimate_end_time = 0.0;
}

std::string HeroNaruto::get_image_path(HeroState st, int frame_index) const {
    if (frame_index <= 0) frame_index = 1;

    switch (st) {
        case HeroState::IDLE: 
            return "assets/image/Naruto/Naruto_idle.png";

        case HeroState::MOVING:{
            int idx = frame_index;
            if (idx < 1) idx = 1;
            if (idx > 2) idx = 2;
            return "assets/image/Naruto/Naruto_move_" + to_string(idx) + ".png";
        }
        

        case HeroState::PUNCH1:
            return "assets/image/Naruto/Naruto_punch_3.png";

        case HeroState::HITSTUN:
            return "assets/image/Naruto/Naruto_hitstun.png";

        case HeroState::JUMPING:
            return "assets/image/Naruto/Naruto_jump.png";

        case HeroState::DEFENDING:
            return "assets/image/Naruto/Naruto_defend.png";

        case HeroState::SHOOT:
            return "assets/image/Naruto/Naruto_shoot.png";

        case HeroState::ULTIMATE:{
            int idx = frame_index;
            if (idx < 1) idx = 1;
            if (idx > 3) idx = 3;
            return "assets/image/Naruto/Naruto_ult_" + to_string(idx) + ".png";
        }

        case HeroState::DEAD:
            return "assets/image/Naruto/Naruto_die.png";

        default:
            return "assets/image/Naruto/Naruto_idle.png";
    }
}

void HeroNaruto::init() {
    Hero::init();
}

void HeroNaruto::update() {
    DataCenter* DC = DataCenter::get_instance();
    if (state == HeroState::JUMPING) {
        Hero::update();  // 讓基底處理物理/落地
        return;
    }
    static double shoot_end_time = 0.0;

// ✅ 只要在 SHOOT，就優先處理「何時結束」
// 這段放最前面，避免被後面各種 return 擋掉
    if (state == HeroState::SHOOT) {
        // 先讓基底跑動畫/物理（不然 frame 也可能不動）
        Hero::update();

        if (al_get_time() >= shoot_end_time) {
            state = HeroState::IDLE;
            frame_id = 0;
            frame_switch_counter = frame_switch_freq;
        }
        return; // ✅ 防止後面邏輯又把 state 改回 SHOOT 或卡住
    }
    // ========= 0. 普攻音效（架構照 Goju） =========
    if (state == HeroState::PUNCH1 && !punch_sfx_played) {
        SoundCenter::get_instance()->play("./assets/sound/Naruto_punch.mp3", ALLEGRO_PLAYMODE_ONCE);
        punch_sfx_played = true;
    } else if (state != HeroState::PUNCH1) {
        punch_sfx_played = false;
    }

    // ========= 1. 大招（影分身） =========
    int key_ult = (player_id == PlayerID::PLAYER1) ? ALLEGRO_KEY_U : ALLEGRO_KEY_M;
    bool ult_pressed = DC->key_state[key_ult] && !DC->prev_key_state[key_ult];
    
    static bool ultimate_started = false;
    static bool ultimate_hit_done = false;
    static double ultimate_end_time = 0.0;
    static double dash_target_x = 0.0;
    static double dash_dir = 0.0;   // +1 往右、-1 往左
    
    if (ult_pressed && MP >= 100 && state != HeroState::DEAD && shape) {
        MP = 0;
        // 音效可留可刪
        // SoundCenter::get_instance()->play("./assets/sound/Naruto_ult.mp3", ALLEGRO_PLAYMODE_ONCE);
    
        state = HeroState::ULTIMATE;
        frame_id = 0;
        frame_switch_counter = frame_switch_freq;
        attack_has_hit = false;
    
        ultimate_started = false;
        ultimate_hit_done = false;
        ultimate_end_time = 0.0;
    }
    
    // 放大招期間行為
    if (state == HeroState::ULTIMATE && shape) {
        Hero* enemy = (player_id == PlayerID::PLAYER1) ? DC->hero2 : DC->hero1;
        if (!enemy || !enemy->shape) {
            state = HeroState::IDLE;
            ultimate_started = false;
            ultimate_hit_done = false;
            return;
        }
    
        Rectangle* me = static_cast<Rectangle*>(shape.get());
        Rectangle* er = static_cast<Rectangle*>(enemy->shape.get());
    
        // ① 第一次進 ULT：瞬移到敵人後方
        if (!ultimate_started) {
            ultimate_started = true;
    
            // 「判斷敵人面向」：目前讀不到 enemy->facing_right（protected）
            // 所以用常見假設：敵人面向你 → 由相對位置推後方在哪側
            double myx = me->center_x();
            double ex  = er->center_x();
    
            double behind_dir = (myx < ex) ? +1.0 : -1.0; // 我在敵人左邊→後方在右邊
            dash_dir = -behind_dir;                       // 從背後穿到另一側
    
            double offset = 140.0;                        // 背後距離（可調）
            double w = me->x2 - me->x1;
            double h = me->y2 - me->y1;
    
            // 瞬移定位：敵人後方
            double new_cx = ex + behind_dir * offset;
            double new_cy = er->center_y();
    
            me->x1 = new_cx - w * 0.5;
            me->x2 = new_cx + w * 0.5;
            me->y1 = new_cy - h * 0.5;
            me->y2 = new_cy + h * 0.5;
    
            // dash 目標：敵人另一側
            dash_target_x = ex + dash_dir * offset;
    
            // 整個大招時間（3格動畫）
            ultimate_end_time = al_get_time() + 0.45;
    
            // 動畫加速（想更快/更慢改這裡）
            frame_switch_freq = 6;
        }
    
        // ② 跑到另一邊（dash）
        {
            double dash_speed = 22.0; // 可調
            double cx = me->center_x();
            double step = dash_dir * dash_speed;
    
            if ((dash_dir > 0 && cx + step > dash_target_x) ||
                (dash_dir < 0 && cx + step < dash_target_x)) {
                step = dash_target_x - cx;
            }
            me->x1 += step;
            me->x2 += step;
        }
    
        // ③ 造成傷害：只打一段（你要打第幾格就改 frame_id 門檻）
        if (!ultimate_hit_done && frame_id >= 1) {
            ultimate_hit_done = true;
    
            int dmg = 20; // 可調
            double knock_dir = (enemy->center_x() >= center_x()) ? 1.0 : -1.0;
            enemy->take_damage(dmg, knock_dir, true); // true=不可防(你不想就改 false)
        }
    
        // ④ 結束
        if (al_get_time() >= ultimate_end_time) {
            state = HeroState::IDLE;
            frame_id = 0;
            frame_switch_counter = frame_switch_freq;
    
            ultimate_started = false;
            ultimate_hit_done = false;
        }
    }

    // ========= 2. 遠程攻擊（螺旋丸） =========
int key_proj = (player_id == PlayerID::PLAYER1) ? ALLEGRO_KEY_I : ALLEGRO_KEY_N;
bool shoot_pressed = DC->key_state[key_proj] && !DC->prev_key_state[key_proj];

// 用 static 當冷卻，不用改 .h
static double rasengan_cd_end = 0.0;
double now = al_get_time();

if (shoot_pressed && now >= rasengan_cd_end && MP >= 10 && state != HeroState::DEAD && shape) {
    MP -= 10;
    rasengan_cd_end = now + 0.25; // 冷卻 0.25 秒（可調）

    SoundCenter::get_instance()->play("./assets/sound/Naruto_Rasengan.mp3", ALLEGRO_PLAYMODE_ONCE);

    Rectangle* r = static_cast<Rectangle*>(shape.get());
    double dir = facing_right ? 1.0 : -1.0;

    // 螺旋丸從手的位置射出（你說太低就調這裡）
    double spawn_x = r->center_x() + dir * 80.0;
    double spawn_y = r->y1 + (r->y2 - r->y1) * 0.25; // 上 1/4 身高

    HeroProjectile* proj = new HeroProjectile(
        this,
        spawn_x,
        spawn_y,
        dir,
        12.0,                    // 速度
        4,                       // 傷害
        ProjectileKind::RASENGAN,
        1.5                      // 存活
    );
    DC->hero_projectiles.push_back(proj);

    // 地面才切 SHOOT 動作；空中不要切狀態（避免干擾跳躍）
    if (state != HeroState::ULTIMATE && state != HeroState::PUNCH1) {
        state = HeroState::SHOOT;
        frame_id = 0;
        frame_switch_counter = frame_switch_freq;
    
        shoot_end_time = al_get_time() + 0.35; // ⭐ 施放動作持續 0.35 秒（可調）
    }
    

}

    // ========= 3. 其餘基本行為交給共用 Hero =========
    Hero::update();

    // ========= 3-1. ULTIMATE：播到最後一張 pose 才開始「分段生分身」 =========
    if (state == HeroState::ULTIMATE && shape) {
        // 等動畫播到最後一張（Goju 是 frame_id>=8）
        if (!ultimate_clone_started && frame_id >= 8) {
            ultimate_clone_started = true;

            // 影分身聯招總時長（可調）
            double now = al_get_time();
            ultimate_end_time = now + 2.2;

            // 第一個立刻生，之後每 0.08 秒生一個
            next_clone_spawn_time = now;
            clone_spawned = 0;
        }

        if (ultimate_clone_started) {
            double now = al_get_time();

            // 在大招持續期間，停在最後一張 pose（照 Goju）
            int idx = static_cast<int>(HeroState::ULTIMATE);
            auto &frames = bitmap_img_ids[idx];
            if (!frames.empty()) frame_id = (int)frames.size() - 1;

            // 分段生成分身 projectile（形成 combo）
            const int total_clones = 6;       // 生成幾個分身（你要“好個”就調這裡）
            const double interval = 0.08;     // 每個分身間隔
            while (clone_spawned < total_clones && now >= next_clone_spawn_time) {
                auto* r = dynamic_cast<Rectangle*>(shape.get());
                double cx  = r->center_x();
                double cy  = r->center_y();
                double dir = facing_right ? 1.0 : -1.0;

                // 分身從角色附近散開一點（視覺更像連招）
                double spread = 30.0 * (clone_spawned % 3); // 0,30,60...
                double spawn_x = cx + dir * (160.0 + spread);
                double spawn_y = cy - (160.0 - 20.0 * (clone_spawned % 2));

                HeroProjectile* clone_dash = new HeroProjectile(
                    this,
                    spawn_x,
                    spawn_y,
                    dir,
                    14.0,                         // 衝刺速度
                    2,                            // 單段傷害（靠數量堆 combo）
                    ProjectileKind::NARUTO_CLONE,  // ★ 新增 kind
                    0.9                           // 存活時間（短衝刺）
                );
                DC->hero_projectiles.push_back(clone_dash);

                clone_spawned++;
                next_clone_spawn_time += interval;
            }

            // 到點結束大招
            if (now >= ultimate_end_time) {
                state = HeroState::IDLE;
                frame_id = 0;
                frame_switch_counter = frame_switch_freq;
                ultimate_clone_started = false;
            }
        }
    }
}


void HeroNaruto::interact(Object* other) {
    Hero* target = dynamic_cast<Hero*>(other);
    if (!target) return;

    // 近身普攻只在 PUNCH1 期間生效（照 Goju）
    if (state != HeroState::PUNCH1) return;

    int dmg = 5;

    double my_x  = center_x();
    double tar_x = target->center_x();
    double dir   = (tar_x >= my_x) ? 1.0 : -1.0;

    target->take_damage(dmg, dir);
}
