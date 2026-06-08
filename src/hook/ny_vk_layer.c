/*
 * ny_vk_layer.c — NyFusion Monitor FPS Counter Vulkan Implicit Layer
 *
 * Implementa uma Vulkan Implicit Layer que conta chamadas a
 * vkQueuePresentKHR (= 1 frame por chamada) e escreve o contador em um
 * arquivo de memória compartilhada que o monitor lê para calcular FPS.
 *
 * Por ser uma "implicit layer", ela é carregada AUTOMATICAMENTE pelo
 * Vulkan Loader para TODO processo que use Vulkan — sem necessidade de
 * qualquer configuração por parte do usuário.
 *
 * Cobre:
 *   • DXVK       (DirectX 9/10/11 via Vulkan, usado por ~90% dos jogos Steam)
 *   • VKD3D-Proton (DirectX 12 via Vulkan)
 *   • Jogos Vulkan nativos (Linux)
 *
 * Interface implementada: Loader/Layer Interface v2
 * Referência: https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderLayerInterface.md
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
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

/* ══════════════════════════════════════════════════════════════════════════
 *  Protocolo de memória compartilhada (idêntico ao ny_fps_shared.h)
 * ══════════════════════════════════════════════════════════════════════════ */

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
} NyFpsData;

static NyFpsData*     g_shm       = NULL;
static pthread_once_t g_shm_once  = PTHREAD_ONCE_INIT;

static uint64_t monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void shm_init_impl(void) {
    int fd = open(NY_SHM_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return;
    if (ftruncate(fd, sizeof(NyFpsData)) < 0) { close(fd); return; }

    void* m = mmap(NULL, sizeof(NyFpsData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (m == MAP_FAILED) return;

    NyFpsData* d = (NyFpsData*)m;
    d->magic   = NY_SHM_MAGIC;
    d->version = NY_SHM_VERSION;
    d->pid     = (uint32_t)getpid();
    d->_pad    = 0;
    atomic_store(&d->frame_count,   0);
    atomic_store(&d->last_frame_ns, 0);
    g_shm = d;
}

static void ny_on_frame(void) {
    pthread_once(&g_shm_once, shm_init_impl);
    if (!g_shm) return;
    atomic_fetch_add_explicit(&g_shm->frame_count,   1,               memory_order_relaxed);
    atomic_store_explicit    (&g_shm->last_frame_ns, monotonic_ns(),  memory_order_relaxed);
}

/* ══════════════════════════════════════════════════════════════════════════
 *  Dispatch table per-VkDevice
 *  Apenas os ponteiros que precisamos manter na cadeia da layer.
 * ══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    PFN_vkGetDeviceProcAddr  GetDeviceProcAddr;
    PFN_vkQueuePresentKHR    QueuePresentKHR;
    PFN_vkDestroyDevice      DestroyDevice;
} NyDeviceDispatch;

/*
 * Mapa dispatch_key → NyDeviceDispatch.
 * Para simplicidade usamos um array pequeno (um jogo raramente cria mais
 * de 1-2 VkDevices). Protegido por mutex.
 */
#define NY_MAX_DEVICES 8
static struct { void* key; NyDeviceDispatch disp; } g_devices[NY_MAX_DEVICES];
static pthread_mutex_t g_dev_mutex = PTHREAD_MUTEX_INITIALIZER;

static NyDeviceDispatch* device_dispatch_find(void* key) {
    for (int i = 0; i < NY_MAX_DEVICES; ++i)
        if (g_devices[i].key == key) return &g_devices[i].disp;
    return NULL;
}

static NyDeviceDispatch* device_dispatch_insert(void* key) {
    for (int i = 0; i < NY_MAX_DEVICES; ++i) {
        if (!g_devices[i].key) {
            g_devices[i].key = key;
            return &g_devices[i].disp;
        }
    }
    return NULL; /* table full — fallback: sobrescreve slot 0 */
}

static void device_dispatch_remove(void* key) {
    for (int i = 0; i < NY_MAX_DEVICES; ++i)
        if (g_devices[i].key == key) { g_devices[i].key = NULL; return; }
}

/* dispatch_key = primeiro ponteiro dentro do objeto VkDevice/VkQueue */
static inline void* get_key(void* obj) {
    return *(void**)obj;
}

/* ══════════════════════════════════════════════════════════════════════════
 *  Funções interceptadas
 * ══════════════════════════════════════════════════════════════════════════ */

VKAPI_ATTR VkResult VKAPI_CALL ny_vkQueuePresentKHR(
    VkQueue                 queue,
    const VkPresentInfoKHR* pPresentInfo)
{
    void* key = get_key(queue);

    pthread_mutex_lock(&g_dev_mutex);
    NyDeviceDispatch* d = device_dispatch_find(key);
    PFN_vkQueuePresentKHR next = d ? d->QueuePresentKHR : NULL;
    pthread_mutex_unlock(&g_dev_mutex);

    VkResult res = next ? next(queue, pPresentInfo) : VK_SUCCESS;
    ny_on_frame();
    return res;
}

VKAPI_ATTR VkResult VKAPI_CALL ny_vkCreateDevice(
    VkPhysicalDevice          physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice*                 pDevice)
{
    /* Encontra o próximo vkCreateDevice na cadeia de layers */
    VkLayerDeviceCreateInfo* chain =
        (VkLayerDeviceCreateInfo*)pCreateInfo->pNext;
    while (chain && !(chain->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO
                   && chain->function == VK_LAYER_LINK_INFO)) {
        chain = (VkLayerDeviceCreateInfo*)chain->pNext;
    }
    if (!chain) return VK_ERROR_INITIALIZATION_FAILED;

    PFN_vkGetInstanceProcAddr gipa = chain->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr   gdpa = chain->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    chain->u.pLayerInfo = chain->u.pLayerInfo->pNext; /* avança layer chain */

    PFN_vkCreateDevice realCreateDevice =
        (PFN_vkCreateDevice)gipa(VK_NULL_HANDLE, "vkCreateDevice");
    if (!realCreateDevice) return VK_ERROR_INITIALIZATION_FAILED;

    VkResult res = realCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (res != VK_SUCCESS) return res;

    /* Constrói dispatch table para este device */
    void* key = get_key(*pDevice);
    pthread_mutex_lock(&g_dev_mutex);
    NyDeviceDispatch* dd = device_dispatch_insert(key);
    if (dd) {
        dd->GetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) gdpa(*pDevice, "vkGetDeviceProcAddr");
        dd->QueuePresentKHR   = (PFN_vkQueuePresentKHR)  gdpa(*pDevice, "vkQueuePresentKHR");
        dd->DestroyDevice     = (PFN_vkDestroyDevice)     gdpa(*pDevice, "vkDestroyDevice");
    }
    pthread_mutex_unlock(&g_dev_mutex);

    (void)gipa; /* silencia warning */
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL ny_vkDestroyDevice(
    VkDevice                     device,
    const VkAllocationCallbacks* pAllocator)
{
    void* key = get_key(device);

    pthread_mutex_lock(&g_dev_mutex);
    NyDeviceDispatch* dd = device_dispatch_find(key);
    PFN_vkDestroyDevice next = dd ? dd->DestroyDevice : NULL;
    device_dispatch_remove(key);
    pthread_mutex_unlock(&g_dev_mutex);

    if (next) next(device, pAllocator);
}

/* ══════════════════════════════════════════════════════════════════════════
 *  Entry points exigidos pelo Vulkan Loader
 * ══════════════════════════════════════════════════════════════════════════ */

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL ny_vkGetDeviceProcAddr(
    VkDevice device, const char* pName)
{
    if (strcmp(pName, "vkGetDeviceProcAddr")  == 0) return (PFN_vkVoidFunction)ny_vkGetDeviceProcAddr;
    if (strcmp(pName, "vkCreateDevice")       == 0) return (PFN_vkVoidFunction)ny_vkCreateDevice;
    if (strcmp(pName, "vkDestroyDevice")      == 0) return (PFN_vkVoidFunction)ny_vkDestroyDevice;
    if (strcmp(pName, "vkQueuePresentKHR")    == 0) return (PFN_vkVoidFunction)ny_vkQueuePresentKHR;

    /* Passa para o próximo layer/driver */
    if (!device) return NULL;
    void* key = get_key(device);
    pthread_mutex_lock(&g_dev_mutex);
    NyDeviceDispatch* dd = device_dispatch_find(key);
    PFN_vkGetDeviceProcAddr next = dd ? dd->GetDeviceProcAddr : NULL;
    pthread_mutex_unlock(&g_dev_mutex);
    return next ? next(device, pName) : NULL;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL ny_vkGetInstanceProcAddr(
    VkInstance instance, const char* pName)
{
    if (strcmp(pName, "vkGetInstanceProcAddr") == 0) return (PFN_vkVoidFunction)ny_vkGetInstanceProcAddr;
    if (strcmp(pName, "vkGetDeviceProcAddr")   == 0) return (PFN_vkVoidFunction)ny_vkGetDeviceProcAddr;
    if (strcmp(pName, "vkCreateDevice")        == 0) return (PFN_vkVoidFunction)ny_vkCreateDevice;
    if (strcmp(pName, "vkDestroyDevice")       == 0) return (PFN_vkVoidFunction)ny_vkDestroyDevice;
    if (strcmp(pName, "vkQueuePresentKHR")     == 0) return (PFN_vkVoidFunction)ny_vkQueuePresentKHR;

    /* Demais funções: busca no próximo layer/ICD */
    if (!instance) return NULL;

    /* Obtém GetInstanceProcAddr da próxima camada via dispatch table do instance.
     * Para simplificar, usamos dlsym diretamente na libvulkan (só como fallback) */
    typedef PFN_vkVoidFunction (*PFN_gipa)(VkInstance, const char*);
    static PFN_gipa real_gipa = NULL;
    if (!real_gipa) {
        void* libvk = dlopen("libvulkan.so.1", RTLD_LAZY | RTLD_NOLOAD);
        if (!libvk) libvk = dlopen("libvulkan.so", RTLD_LAZY | RTLD_NOLOAD);
        if (libvk) real_gipa = (PFN_gipa)dlsym(libvk, "vkGetInstanceProcAddr");
    }
    return real_gipa ? real_gipa(instance, pName) : NULL;
}

/*
 * Ponto de entrada principal: chamado pelo Vulkan Loader para negociar a
 * versão da interface e obter as funções de entry point da camada.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(
    VkNegotiateLayerInterface* pVersionStruct)
{
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->loaderLayerInterfaceVersion = 2;
        pVersionStruct->pfnGetInstanceProcAddr      = ny_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr        = ny_vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = NULL;
    }
    return VK_SUCCESS;
}

