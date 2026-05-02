#ifndef HERO_H_INCLUDED
#define HERO_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include "../shapes/Point.h"
#include <vector>
#include <queue>
#include <memory>
#include <string>

enum class HeroState {
    IDLE,
    MOVING,
    PUNCH1,
    DEAD,
    HITSTUN,   // ⭐ 新增：受擊僵直
    JUMPING,
    DEFENDING,
    ULTIMATE,
    SHOOT,
    HEROSTATE_MAX
};

enum class PlayerID { PLAYER1, PLAYER2 };

/// Hero: 所有英雄共用的基底類別
class Hero : public Object
{
public:
    explicit Hero(PlayerID pid);
    virtual ~Hero() = default;
    
    // 初始化：載入第一張圖，建立身體 hitbox
    virtual void init();

    // 每幀更新：處理受擊 / 輸入 / 移動 / 動畫 / 攻擊框
    virtual void update();

    // 繪圖：根據當前 state + frame_id 畫出對應貼圖
    virtual void draw();

    // 受到 damage，dir = -1 or 1，代表被往左/右擊退
    virtual void take_damage(int dmg, double dir, bool unblockable = false);

    // 讓 OperationCenter 用來做英雄互打
    virtual void interact(Object* other) = 0;

    // 取得目前血量（給 UI 用）
    // 取得目前血量 / 魔力（給 UI 用）
    int get_HP()     const { return HP; }
    int get_maxHP()  const { return maxHP; }
    int get_MP()     const { return MP; }
    int get_maxMP()  const { return maxMP; }

    // 判斷目前是否在防禦
    bool is_defending() const { return state == HeroState::DEFENDING; }

    // 攻擊 hitbox 相關
    bool is_attack_box_active() const { return attack_box_active; }
    const Rectangle* get_attack_box() const { return attack_box.get(); }

    bool has_attack_hit() const { return attack_has_hit; }
    void mark_attack_hit() { attack_has_hit = true; }

    // 取得角色中心位置（給 AI / 相機用）
    double center_x() const;
    double center_y() const;

    // 給子類 & OperationCenter 用的公開狀態
    HeroState state;
    PlayerID  player_id;

protected:

    // ===== 數值屬性 =====
    int    HP;
    int    maxHP;
    int    MP;
    int    maxMP;
    double speed;

    // 垂直運動
    bool   is_jumping;
    double vx, vy;
    double gravity;
    double ground_y;   // 地板高度
    double jump_speed;

    // 朝向（true: 向右）
    bool   facing_right;

    // 受擊系統
    int    hitstun_frames;    // 還剩幾幀不能動
    double knockback_vx;      // 被水平擊退的速度

    // 攻擊系統
    bool attack_has_hit;      // 這次攻擊是否已經打到人

    // 動畫資料：bitmap_img_ids[state] = {frame_index1, frame_index2 ...}
    std::vector<std::vector<int>> bitmap_img_ids;

    int    frame_switch_counter;
    int    frame_switch_freq;
    int    frame_id;          // 在 bitmap_img_ids[state] 裡的 index

    double mp_timer;          // 之後如果要用時間制回 MP 可以用到

    // 攻擊 hitbox：只在攻擊幀啟用
    std::unique_ptr<Rectangle> attack_box;
    bool attack_box_active;

    // --- 子類別必須實作：給定 state + frame_index → 對應圖片路徑 ---
    virtual std::string get_image_path(HeroState state,
                                       int frame_index) const = 0;

    // 更新攻擊 hitbox（在 update() 最後呼叫）
    void update_attack_box();

    // 依照 state + frame_id 決定要畫哪張圖（並確保圖片已載入）
    void update_bitmap_from_state();

    // 處理 hitstun，如果還在硬直中就回傳 true（本幀不再處理輸入）
    bool update_hitstun();
};

#endif
