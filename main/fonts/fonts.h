/**
 * @file fonts.h
 * @brief Глобальные шрифты проекта
 * 
 * Шрифты сгенерированы через LVGL Font Converter с BPP=4
 */

#pragma once

#include "lvgl.h"

// ==================== Текстовые шрифты (Inter Medium) ====================
LV_FONT_DECLARE(inter_medium_14);   // Интерфейс, меню, треки (14px)
LV_FONT_DECLARE(inter_medium_18);   // Названия станций и треков (18px)

// ==================== Моноширинные шрифты (Roboto Mono) ====================
LV_FONT_DECLARE(roboto_mono_12);    // Статус-бар, битрейт (12px)
LV_FONT_DECLARE(roboto_mono_14);    // Значения dB, частота (14px)

// ==================== Акцентный шрифт (Oswald SB) ====================
LV_FONT_DECLARE(oswald_sb_24);     // Громкость, часы (24px)

// ==================== Указатели для удобства ====================
#define font_text_small      (&inter_medium_14)
#define font_text_medium     (&inter_medium_18)
#define font_mono_small      (&roboto_mono_12)
#define font_mono_medium     (&roboto_mono_14)
#define font_accent          (&oswald_sb_24)

// Функция инициализации
void init_fonts(void);

