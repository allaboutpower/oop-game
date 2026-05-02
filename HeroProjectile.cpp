#include "HeroProjectile.h"
#include "Hero.h"
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../shapes/Rectangle.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

HeroProjectile::HeroProjectile(Hero* owner,
                               double x, double y,
                               double dir,
                               double speed,
                               int dmg,
                               ProjectileKind kind,
                               double life_seconds)
    : owner(owner),
      vx(0.0),
      vy(0.0),
      damage(dmg),
      alive(true),
      kind(kind),
      spawn_time(al_get_time()),
      life_time(life_seconds)
{
    double w = 0.0;
    double h = 0.0;

    if (kind == ProjectileKind::RED_SHOT) {
        // 小型紅色飛彈
        vx = dir * speed;
        vy = 0.0;
        w = 60.0;
        h = 60.0;

    } else if (kind == ProjectileKind::RASENGAN) {
        // Naruto 螺旋丸：水平射出
        vx = dir * speed;
        vy = 0.0;
        w = 70.0;
        h = 500.0;
    } else if (kind == ProjectileKind::NARUTO_CLONE) {
        // Naruto 影分身：衝刺用的分身投射物
        vx = dir * speed;
        vy = 0.0;
        w = 90.0;
        h = 90.0;
    } else if (kind == ProjectileKind::BLUE_FIELD) {
        // 藍色場地陷阱
        vx = 0.0;
        vy = 0.0;
        w = 60.0;
        h = 60.0;
    } else if (kind == ProjectileKind::ULTIMATE_PURPLE) {
        // 大招：紫色光束（長條形 hitbox）
        vx = 0.0;        // 光束不移動
        vy = 0.0;
        w  = 600.0;      // 光束長度
        h  = 140.0;      // 光束高度

        double x1, x2;
        if (dir >= 0) {
            x1 = x+100;
            x2 = x + w;
        } else {
            x1 = x - w;
            x2 = x-100;
        }

        double y1 = y - h / 2.0;
        double y2 = y + h / 2.0;

        shape = std::unique_ptr<Rectangle>(new Rectangle(x1, y1, x2, y2));
        return;
    }

    double x1 = x - w / 2.0;
    double y1 = y - h / 2.0;
    double x2 = x + w / 2.0;
    double y2 = y + h / 2.0;

    shape = std::unique_ptr<Rectangle>(new Rectangle(x1, y1, x2, y2));
}

void HeroProjectile::update() {
    if (!alive || !shape) return;

    DataCenter* DC = DataCenter::get_instance();
    double field_w = DC->game_field_length;
    double field_h = DC->window_height;

    Rectangle* r = dynamic_cast<Rectangle*>(shape.get());
    if (!r) return;

    // 會移動的投射物：RED_SHOT / RASENGAN / NARUTO_CLONE
if (kind == ProjectileKind::RED_SHOT ||
    kind == ProjectileKind::RASENGAN ||
    kind == ProjectileKind::NARUTO_CLONE) {

    r->x1 += vx;
    r->x2 += vx;
    r->y1 += vy;
    r->y2 += vy;

    // 飛出場外就死
    if (r->x2 < 0 || r->x1 > field_w ||
        r->y2 < 0 || r->y1 > field_h) {
        alive = false;
    }
}
// BLUE_FIELD、ULTIMATE_PURPLE：不移動（留空即可）

    // 超過存活時間就死（場地 3 秒、紅彈 2 秒、光束 2~3 秒…）
    double now = al_get_time();
    if (now - spawn_time > life_time) {
        alive = false;
    }
}

void HeroProjectile::draw() {
    if (!alive || !shape) return;

    ImageCenter* IC = ImageCenter::get_instance();
    const char* path = nullptr;

    if (kind == ProjectileKind::RED_SHOT) {
        path = "assets/image/Goju/Goju_red1.png";
    } else if (kind == ProjectileKind::BLUE_FIELD) {
        // Goju_blue1 / 2 間簡單切幀
        double t = al_get_time() - spawn_time;
        int frame = static_cast<int>(t * 4.0) % 2;
        path = (frame == 0)
             ? "assets/image/Goju/Goju_blue1.png"
             : "assets/image/Goju/Goju_blue2.png";
    } else if (kind == ProjectileKind::ULTIMATE_PURPLE) {
        // 大招光束圖（記得準備這張）
        path = "assets/image/Goju/Goju_ult.png";

    } else if (kind == ProjectileKind::RASENGAN) {
        path = "assets/image/Naruto/Rasengan.png";
    } else if (kind == ProjectileKind::NARUTO_CLONE) {
        path = "assets/image/Naruto/Naruto_clone.png";
    }

    ALLEGRO_BITMAP* bmp = IC->get(path);
    if (!bmp) return;

    Rectangle* r = dynamic_cast<Rectangle*>(shape.get());
    if (!r) return;

    float draw_x = static_cast<float>(r->x1);
    float draw_y = static_cast<float>(r->y1);

   

    // ⭐ 若角色向左 → 水平翻轉
    int flags = (vx<0) ? ALLEGRO_FLIP_HORIZONTAL : 0;

    al_draw_bitmap(bmp, draw_x, draw_y, flags);

    // 若要 debug hitbox 可以打開
    /*
    al_draw_rectangle(r->x1, r->y1, r->x2, r->y2,
                      al_map_rgb(0, 255, 255), 1);
    */
}
