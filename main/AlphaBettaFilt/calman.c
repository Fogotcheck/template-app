#include "calman.h"

float dt = CALMAN_DT; // в секундах

float AlphaBettaFiltr(float xm)
{
    static float xk_1 = CALMAN_START_VAL, vk_1 = CALMAN_START_SPEED, alpha = CALMAL_ALPHA, betta = CALMAN_BETTA;
    static float xk, vk, rk;
    xk = xk_1 + (vk_1 * dt);
    vk = vk_1;
    rk = xm - xk;
    xk += alpha * rk;
    vk += (betta * rk) / dt;
    xk_1 = xk;
    vk_1 = vk;
    return xk_1;
}