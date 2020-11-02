
#include "render.h"

#include <math.h>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include "util.h"

// useful for converting degrees to radians
#define PI 3.14159265358979323846

// useful for mapping f into the range [0, k)
float float_mod(float f, int k) {
    // map into (-k, k)
    float res = fmodf(f, k);

    // fold into [0, k]
    if (res < 0) {
        res += k;
    }

    // accommodate for floating point rounding errors
    if (res >= k) {
        res = 0.0;
    }
    return res;
}
int checkPointRange(float x,float y,int mapsize){
    if(x<0.0f||x>=(float)(mapsize)){
        return 404;
    }
    if(y<0.0f||y>=(float)(mapsize)){
        return 404;
    }
    return 111;
}
void coreAlg(float x,float y,const player_t* p,ctx_t* c,const float tempw,const float temph,float tempD,int u){
     if((checkPointRange(x,y,c->map_size))==111){
            int tempx = (int)x;
            int tempy = (int)y;
            uint8_t tempQz = *(c->height_map + tempx + tempy*(c->map_size));
            int tempPz = p->height;
            float tempres01 = (float)(tempQz-tempPz);
            uint32_t tempCo = *(c->color_map + tempx + tempy*(c->map_size));
            float tempres02 = (tempw/2)*(tempres01/tempD)+(temph/2);

             int v = (int)tempres02;
             if(v>(c->scr_height)){
                    v = c->scr_height;
             }else if(v<0){
                    v = 0;
             }
             draw_line(c,u,0,v,tempCo);

        }
}
/** Move the player according to its velocities.
 */
void update_player(player_t* p, ctx_t const* ctx) {
    int tempv_an;
    if(p->v_angular<0){
        tempv_an = 0 - p->v_angular;
        tempv_an = 360 - (tempv_an%360);
    }else{tempv_an = p->v_angular;}
    p->angle = ((tempv_an + p->angle)%360);
    float tempAngle = (float)(p->angle);
    float tempCur = (tempAngle/180.0f)*(PI);
    float velo = (float)(p->v);
    p->x = p->x + (velo*cos(tempCur));
    if((p->x)>=((float)(ctx->map_size))){
        p->x = float_mod(p->x,ctx->map_size);
    }
    if(p->x < 0.0f){
        p->x = (float)ctx->map_size - float_mod((-(p->x)),ctx->map_size);
    }
    p->y = p->y - (velo*sin(tempCur));
    if(p->y < 0.0f){
        p->y = (float)ctx->map_size - float_mod((-(p->y)),ctx->map_size);
    }
    if(p->y>=((float)(ctx->map_size))){
        p->y = float_mod(p->y,ctx->map_size);
    }
    int tempX = (int)(p->x);
    int tempY = (int)(p->y);
    p->height = p->height + p->v_height;
    int currentHM = (int)(*(ctx->height_map + tempX + tempY*(ctx->map_size)));
    if(p->height<(currentHM+20)){
        p->height = currentHM + 20;
    }
}

/** Draw a vertical line into the context's out buffer in the screen column u
 *  from screen row v_from (exclusively) to screen row v_to (inclusively).
 *
 *  For every call to this function, v_from <= v_to has to hold.
 *  If v_from == v_to holds, nothing is drawn.
 *
 *  Coordinates have their origin in the bottom left corner of the screen and
 *  are 1-based.
 */
void draw_line(ctx_t* c, int u, int v_from, int v_to, uint32_t color) {
   if(v_from > v_to){
        return;
   }
   if(v_from == v_to){
        return;
   }
    uint32_t* tempStart = u - 1 + (c->scr_height - v_to)*(c->scr_width) + c->out;
    int tempAmount = v_to - v_from;
    for(int i=0;i<tempAmount;i++){
        *tempStart = color;
        tempStart = tempStart + c->scr_width;
    }

}

/** Render the scene from player's perspective into the context's out buffer.
 */
void render(const player_t* p, ctx_t* c) {
    int Amount = (c->scr_height)*(c->scr_width);
    float tempw = (float)c->scr_width;
    float temph = (float)c->scr_height;
    /*INITIALIZE THE SKY_COLOR*/
    for(int i=0;i<Amount;i++){
        *((c->out)+i) = c->sky_color;
    }
    float tempCur = (((float)(p->angle))/180.0f)*(PI);
    float tempD = (float)(c->distance);
    for(int i=0;i<(c->distance);i++){
        float a = tempD*(cos(tempCur));
        float b = tempD*(sin(tempCur));
        float Lx = p->x + a - b;
        float Ly = p->y - b - a;
        float Rx = p->x + a + b;
        float Ry = p->y - b + a;
        float deltaX = Lx - Rx;
        float varX = deltaX/(tempw-1);
        float deltaY = Ly - Ry;
        float varY = deltaY/(tempw-1);
        coreAlg(Lx,Ly,p,c,tempw,temph,tempD,1);
        coreAlg(Rx,Ry,p,c,tempw,temph,tempD,c->scr_width);
        for(int t=0;t<((c->scr_width)-2);t++){
            Lx = Lx - varX;
            Ly = Ly - varY;
            coreAlg(Lx,Ly,p,c,tempw,temph,tempD,t+2);
        }
        tempD = tempD - 1;
    }
}


int bonus_implemented(void) {
    // TODO change to 1 if the bonus exercise is implemented.
    return 0;
}
