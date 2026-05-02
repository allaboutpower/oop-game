#include "Hero.h"
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../data/SoundCenter.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>


using namespace std;

// ------------------- 建構子 -------------------

Hero::Hero(PlayerID pid)
    : HP(500), maxHP(500),
      MP(0), maxMP(100),
      speed(5.0),
      is_jumping(false),
      vx(0.0), vy(0.0),
      gravity(0.8),
      ground_y(0.0),
      jump_speed(-25.0),
      facing_right(pid == PlayerID::PLAYER1),  // P1 向右、P2 向左
      state(HeroState::IDLE),
      player_id(pid),
      hitstun_frames(0),
      knockback_vx(0.0),
      attack_has_hit(false),
      frame_switch_counter(0),
      frame_switch_freq(10),
      frame_id(0),
      mp_timer(0.0),
      attack_box(nullptr),
      attack_box_active(false)
{
    bitmap_img_ids.resize(static_cast<int>(HeroState::HEROSTATE_MAX));
}

// ------------------- 工具函式 -------------------

double Hero::center_x() const {
    if (!shape) return 0.0;
    auto *r = dynamic_cast<Rectangle*>(shape.get());
    return (r->x1 + r->x2) / 2.0;
}

double Hero::center_y() const {
    if (!shape) return 0.0;
    auto *r = dynamic_cast<Rectangle*>(shape.get());
    return (r->y1 + r->y2) / 2.0;
}

// ------------------- init -------------------

void Hero::init() {
    int idle_state_index = static_cast<int>(HeroState::IDLE);

    // 預設給一個安全的寬高（若載圖失敗就用這個）
    int w = 80;
    int h = 120;

    // 1. 確認 idle 狀態的 frame 有正確設定
    if (idle_state_index >= 0 &&
        idle_state_index < static_cast<int>(bitmap_img_ids.size()) &&
        !bitmap_img_ids[idle_state_index].empty())
    {
        int first_frame_id = bitmap_img_ids[idle_state_index][0];

        // 2. 取得 idle 的圖片路徑
        std::string path = get_image_path(HeroState::IDLE, first_frame_id);

        // 3. 嘗試載入圖片，如果成功才用它的寬高
        ImageCenter *IC = ImageCenter::get_instance();
        ALLEGRO_BITMAP *bmp = IC->get(path);   // get() 會自動載入

        if (bmp) {
            w = al_get_bitmap_width(bmp);
            h = al_get_bitmap_height(bmp);
        }
        // 若 bmp == nullptr，就保持預設 w/h，不會崩潰
    }
    // 若 bitmap_img_ids[IDLE] 根本沒設定，也會走上面的預設 w/h

    DataCenter *DC = DataCenter::get_instance();
    double field_w = DC->game_field_length;
    double field_h = DC->window_height;

    double cx = (player_id == PlayerID::PLAYER1) ? field_w * 0.33 : field_w * 0.66;
    ground_y = field_h;

    double x1 = cx - w / 2.0;
    double y1 = ground_y - h;
    double x2 = cx + w / 2.0;
    double y2 = ground_y;

    shape = std::unique_ptr<Rectangle>(new Rectangle(x1, y1, x2, y2));
}

// ------------------- 受擊相關 -------------------

bool Hero::update_hitstun() {
    if (hitstun_frames <= 0) return false;

    // 正在硬直中
    hitstun_frames--;

    // 被擊退
    if (shape) {
        auto *r = dynamic_cast<Rectangle*>(shape.get());
        double dx = knockback_vx;
        // 簡單阻力
        knockback_vx *= 0.9;

        r->x1 += dx;
        r->x2 += dx;

        // 維持在地面上
        double h = r->y2 - r->y1;
        r->y2 = ground_y;
        r->y1 = ground_y - h;
    }

    // 這幀不接受輸入
    return true;
}

// ------------------- 更新攻擊 hitbox -------------------



void Hero::update_attack_box() {
    attack_box_active = false;

    if (!shape) return;
    auto *r = dynamic_cast<Rectangle*>(shape.get());
    double w = r->x2 - r->x1;
    double h = r->y2 - r->y1;

    // 以角色中心為基準
    double cx = (r->x1 + r->x2) / 2.0;
    double cy = (r->y1 + r->y2) / 2.0;

    bool active = false;

    if (state == HeroState::PUNCH1 )
    {
        // 先簡單寫：每段都在 frame_id == 0 時有 hitbox
        if (frame_id == 0) {
            active = true;
        }
    }
    

    if (!active) {
        // 沒有攻擊幀就不要開啟 hitbox
        return;
    }

    attack_box_active = true;

    double atk_w = w * 0.6;
    double atk_h = h * 0.4;

    double ax1, ay1, ax2, ay2;
    if (facing_right) {   // ★ 這裡用成員 facing_right
        ax1 = cx;
        ax2 = cx + atk_w;
    } else {
        ax1 = cx - atk_w;
        ax2 = cx;
    }
    ay1 = cy - atk_h / 2.0;
    ay2 = cy + atk_h / 2.0;

    if (!attack_box) {
        attack_box.reset(new Rectangle(ax1, ay1, ax2, ay2));
    } else {
        attack_box->x1 = ax1;
        attack_box->y1 = ay1;
        attack_box->x2 = ax2;
        attack_box->y2 = ay2;
    }
}


// ------------------- 動畫貼圖更新 -------------------

void Hero::update_bitmap_from_state() {
    int idx = static_cast<int>(state);
    if (idx < 0 || idx >= (int)bitmap_img_ids.size()) return;
    auto &frames = bitmap_img_ids[idx];
    if (frames.empty()) return;

    if (frame_id < 0 || frame_id >= (int)frames.size())
        frame_id = 0;

    int frame_index = frames[frame_id];

    // 確保圖片已載入（ImageCenter 內部會做快取）
    std::string path = get_image_path(state, frame_index);
    ImageCenter *IC = ImageCenter::get_instance();
    (void)IC->get(path);  // 只要確保載入即可
}

// ------------------- update -------------------

void Hero::update() {
    DataCenter *DC = DataCenter::get_instance();
    double field   = DC->game_field_length;

    // 0. 如果正在硬直，直接處理擊退並返回
    if (update_hitstun()) {
        update_attack_box();
        return;
    }

    // 1. 讀取鍵盤輸入
    int key_left, key_right, key_jump, key_attack, key_defend, key_ult,key_shoot;
    if (player_id == PlayerID::PLAYER1) {
        key_left   = ALLEGRO_KEY_A;
        key_right  = ALLEGRO_KEY_D;
        key_jump   = ALLEGRO_KEY_W;
        key_defend = ALLEGRO_KEY_S;
        key_attack = ALLEGRO_KEY_J;
        key_shoot  = ALLEGRO_KEY_I;
        key_ult    = ALLEGRO_KEY_U;
    } else {
        key_left   = ALLEGRO_KEY_LEFT;
        key_right  = ALLEGRO_KEY_RIGHT;
        key_jump   = ALLEGRO_KEY_UP;
        key_defend = ALLEGRO_KEY_DOWN;
        key_attack = ALLEGRO_KEY_B;
        key_shoot  = ALLEGRO_KEY_N;
        key_ult    = ALLEGRO_KEY_M;
    }

    bool moving_left  = DC->key_state[key_left];
    bool moving_right = DC->key_state[key_right];

    bool moving_horiz = moving_left ^ moving_right;

    // ★ 根據水平輸入更新角色面向方向
    if (moving_left && !moving_right) {
        facing_right = false;   // 往左走就面向左
    } else if (moving_right && !moving_left) {
        facing_right = true;    // 往右走就面向右
    }
    // 2. 位置更新
    double cx = center_x();

    vx = 0.0;
    if (moving_left)  vx -= speed;
    if (moving_right) vx += speed;

    cx += vx;

    // 邊界處理
    if (shape) {
        auto *r = dynamic_cast<Rectangle*>(shape.get());
        double half_w = (r->x2 - r->x1) / 2.0;
        if (cx - half_w < 0)     cx = half_w;
        if (cx + half_w > field) cx = field - half_w;

        // 水平更新
        double dx = cx - (r->x1 + r->x2) / 2.0;
        r->x1 += dx;
        r->x2 += dx;
    }

    // 3. 跳躍 / 重力
    bool press_jump =
        DC->key_state[key_jump] && !DC->prev_key_state[key_jump];

    if (!is_jumping && press_jump) {
        is_jumping = true;
        vy = jump_speed;
        state = HeroState::JUMPING;
        frame_id = 0;
        frame_switch_counter = frame_switch_freq;
    }

    if (is_jumping && shape) {
        vy += gravity;
        auto *r = dynamic_cast<Rectangle*>(shape.get());
        r->y1 += vy;
        r->y2 += vy;

        if (r->y2 >= ground_y) {
            double h = r->y2 - r->y1;
            r->y2 = ground_y;
            r->y1 = ground_y - h;
            is_jumping = false;
            vy = 0.0;
        }
    }

    // 4. 防禦鍵
    bool defending = DC->key_state[key_defend];

    // 5. 攻擊鍵（一次按下觸發）
    bool attack_pressed =
        DC->key_state[key_attack] && !DC->prev_key_state[key_attack];

    if (state != HeroState::DEAD) {
        if (state == HeroState::SHOOT || state == HeroState::ULTIMATE) {
        // 可以在這裡保留移動、重力等等（程式已在前面處理過）
        // 但不要改變 state
    }
    else if (defending && !is_jumping) {
        state = HeroState::DEFENDING;
    }
    else if (attack_pressed) {// 普攻狀態改變
            if (state != HeroState::SHOOT && state != HeroState::ULTIMATE) {
            state = HeroState::PUNCH1;
            frame_id = 0;
            // 用一般動畫速度就好，不要 200 這麼久
            frame_switch_counter = frame_switch_freq;
            attack_has_hit = false;
            //SoundCenter::get_instance()->play("./assets/sound/Goju_blue.mp3",ALLEGRO_PLAYMODE_ONCE);
        }
            
        
        } else if (is_jumping) {
            state = HeroState::JUMPING;
        } else if (moving_horiz) {
            state = HeroState::MOVING;
        } else {
            if (state != HeroState::PUNCH1&&state != HeroState::ULTIMATE)
                state = HeroState::IDLE;
        }
    }
    

    // 6. 動畫切換：如果在 PUNCH，播放完一輪就回到 IDLE
    frame_switch_counter--;
    if (frame_switch_counter <= 0) {
        frame_switch_counter = frame_switch_freq;
        frame_id++;

        int idx = static_cast<int>(state);
        auto &frames = bitmap_img_ids[idx];
        if (frames.empty()) {
            frame_id = 0;
            return;
        }

        if (frame_id >= (int)frames.size()) {
            if (state == HeroState::ULTIMATE) {
                // ⭐ 大招：停在最後一張 pose（例如 Goju_ult_pose9）
                frame_id = (int)frames.size() - 1;
            } else {
                // 普攻播完回到 idle
                if (state == HeroState::PUNCH1) {
                    state = HeroState::IDLE;
                }
                frame_id = 0;
                attack_has_hit = false;
            }
        }
    }

    // 7. MP 回復（目前簡單版：每幀 +1，之後你可以改成時間制）
    if (MP < maxMP) MP++;

    // 8. 更新攻擊框、預載入當前幀 bitmap
    update_attack_box();
    update_bitmap_from_state();
}

// ------------------- draw -------------------

void Hero::draw() {
    if (!shape) return;

    int idx = static_cast<int>(state);
    if (idx < 0 || idx >= (int)bitmap_img_ids.size()) return;
    auto &frames = bitmap_img_ids[idx];
    if (frames.empty()) return;

    if (frame_id < 0 || frame_id >= (int)frames.size())
        frame_id = 0;

    int frame_index = frames[frame_id];
    std::string path = get_image_path(state, frame_index);

    ImageCenter *IC = ImageCenter::get_instance();
    ALLEGRO_BITMAP *bmp = IC->get(path);
    if (!bmp) return;

    auto *r = dynamic_cast<Rectangle*>(shape.get());
    double x = r->x1;
    double y = r->y1;

    // ★ 根據 facing_right 決定是否水平翻轉
    int flags = 0;
    if (!facing_right) {
        flags |= ALLEGRO_FLIP_HORIZONTAL;
    }

    al_draw_bitmap(bmp, (float)x, (float)y, flags);

    // 如需要 debug hitbox，可打開這段
    
    al_draw_rectangle(r->x1, r->y1, r->x2, r->y2,
                      al_map_rgb(255, 0, 0), 1);
    if (attack_box_active && attack_box) {
        al_draw_rectangle(attack_box->x1, attack_box->y1,
                          attack_box->x2, attack_box->y2,
                          al_map_rgb(0, 255, 0), 1);
    }
    
}

// ------------------- take_damage -------------------

void Hero::take_damage(int dmg, double dir, bool unblockable) {
    // 死人就不要再扣血了
    if (state == HeroState::DEAD || dmg <= 0) return;

    // ⭐ 防禦中 & 這一擊可以被防禦 → 傷害變成 1/3
    if (!unblockable && state == HeroState::DEFENDING) {
        dmg =0;               // 變成原本的 1/3
        //if (dmg <= 0) dmg = 1;  // 你不想要 0 傷害的話就留著
    }

    HP -= dmg;
    if (HP < 0) HP = 0;

    if (HP == 0) {
        // ⭐ 死亡：進入 DEAD 狀態，顯示 Goju_dead.png
        state = HeroState::DEAD;

        // 不要再硬直、不要再被擊退
        hitstun_frames = 0;
        knockback_vx   = 0.0;
        return;
    }

    // 還活著 → 進入受擊硬直
    state = HeroState::HITSTUN;
    frame_id = 0;
    frame_switch_counter = frame_switch_freq;

    // 設定硬直幀數與擊退速度
    hitstun_frames = 20;
    knockback_vx   = dir * 6.0;
}