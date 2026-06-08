/**
 * @file ny_fps_hook.c
 * @brief LD_PRELOAD hook — conta frames reais do jogo
 *
 * Intercepta as funções de apresentação de frame das três APIs gráficas
 * principais e incrementa um contador em um arquivo de memória compartilhada
 * que o ny-fusion-monitor lê para calcular FPS exato.
 *
 * Cobertura:
 *   • OpenGL/X11   — glXSwapBuffers
 *   • OpenGL/EGL   — eglSwapBuffers
 *   • Vulkan       — vkQueuePresentKHR  (cobre DXVK, VKD3D-Proton, Vulkan nativo)
 *
 * Como usar no Steam:
 *   Opções de lançamento do jogo:
 *     LD_PRELOAD=/caminho/para/libny_fps_hook.so %command%
 *
 * Protocolo do arquivo compartilhado:
 *   Caminho fixo: /tmp/.ny_fps_hook
 *   Layout: NyFpsSharedData (24 bytes, PACKED)
 *   Acesso: qualquer processo pode ler; apenas o hook escreve.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/* ═══════════════════ Protocolo de memória compartilhada ══════════════════ */

#define NY_SHM_MAGIC    0x4E594650u   /* "NYFP" */
#define NY_SHM_VERSION  1u
#define NY_SHM_PATH     "/tmp/.ny_fps_hook"

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t pid;
    uint32_t _pad;
    _Atomic uint64_t frame_count;
    _Atomic uint64_t last_frame_ns;
} NyFpsSharedData;

/* Sem __attribute__((packed)) — campos de 64-bit ficam naturalmente alinhados. */
/* sizeof deve ser 32 bytes. */

/* ═══════════════════ Variáveis globais do hook ═══════════════════════════ */

static NyFpsSharedData* g_shm   = NULL;
static pthread_once_t   g_once  = PTHREAD_ONCE_INIT;

/* ═══════════════════ Helpers ═════════════════════════════════════════════ */

static uint64_t monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void hook_init_impl(void) {
    int fd = open(NY_SHM_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("[ny_hook] open " NY_SHM_PATH);
        return;
    }

    if (ftruncate(fd, sizeof(NyFpsSharedData)) < 0) {
        perror("[ny_hook] ftruncate");
        close(fd);
        return;
    }

    void* m = mmap(NULL, sizeof(NyFpsSharedData),
                   PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (m == MAP_FAILED) {
        perror("[ny_hook] mmap");
        return;
    }

    NyFpsSharedData* d = (NyFpsSharedData*)m;
    d->magic   = NY_SHM_MAGIC;
    d->version = NY_SHM_VERSION;
    d->pid     = (uint32_t)getpid();
    d->_pad    = 0;
    atomic_store(&d->frame_count,    0);
    atomic_store(&d->last_frame_ns,  0);

    g_shm = d;
    /* fprintf(stderr, "[ny_hook] inicializado para PID %u\n", d->pid); */
}

/** Chamada a cada frame apresentado — thread-safe via atomic. */
static void ny_on_frame(void) {
    pthread_once(&g_once, hook_init_impl);
    if (!g_shm) return;

    atomic_fetch_add_explicit(&g_shm->frame_count, 1, memory_order_relaxed);
    atomic_store_explicit(&g_shm->last_frame_ns, monotonic_ns(), memory_order_relaxed);
}

/* ═══════════════════ Intercepts ══════════════════════════════════════════ */

/* ── GLX ─────────────────────────────────────────────────────────────────── */

void glXSwapBuffers(void* dpy, unsigned long drawable) {
    typedef void (*fn_t)(void*, unsigned long);
    static fn_t real = NULL;
    if (!real) { *(void**)&real = dlsym(RTLD_NEXT, "glXSwapBuffers"); }
    if (real) { real(dpy, drawable); }
    ny_on_frame();
}

/* ── EGL ─────────────────────────────────────────────────────────────────── */

/* EGLBoolean = unsigned int */
unsigned int eglSwapBuffers(void* dpy, void* surface) {
    typedef unsigned int (*fn_t)(void*, void*);
    static fn_t real = NULL;
    if (!real) { *(void**)&real = dlsym(RTLD_NEXT, "eglSwapBuffers"); }
    unsigned int ret = real ? real(dpy, surface) : 0u;
    ny_on_frame();
    return ret;
}

/* ── Vulkan ──────────────────────────────────────────────────────────────── */

/*
 * Declarações mínimas para não depender dos headers Vulkan.
 * VkResult = int32_t no spec; VkQueue = ponteiro opaco.
 * A struct VkPresentInfoKHR não precisa ser dereferenciada — passamos
 * o ponteiro diretamente para a função real.
 */
typedef int32_t VkResult;
typedef void*   VkQueue;
/* pPresentInfo: passamos opaco — o tipo real não importa para o intercept */

VkResult vkQueuePresentKHR(VkQueue queue, const void* pPresentInfo) {
    typedef VkResult (*fn_t)(VkQueue, const void*);
    static fn_t real = NULL;
    if (!real) { *(void**)&real = dlsym(RTLD_NEXT, "vkQueuePresentKHR"); }
    VkResult ret = real ? real(queue, pPresentInfo) : 0;
    ny_on_frame();
    return ret;
}

