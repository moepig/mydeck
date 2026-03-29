#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#define CH32V003                    1
#define FUNCONF_SYSTEM_CORE_CLOCK   48000000
#define FUNCONF_SYSTICK_USE_HCLK    1

// デバッグ printf (SWIO/PD1 経由)
// ビルド時に -DMYDECK_DEBUG=1 を指定すると有効化
#ifdef MYDECK_DEBUG
#define FUNCONF_USE_DEBUGPRINTF     1
#define FUNCONF_DEBUGPRINTF_TIMEOUT (1024*64)
#else
#define FUNCONF_USE_DEBUGPRINTF     0
#endif

#endif
