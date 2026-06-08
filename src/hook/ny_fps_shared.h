#pragma once
/**
 * @file ny_fps_shared.h
 * @brief Protocolo de memória compartilhada entre libny_fps_hook.so e
 *        FrameMetricsService.
 *
 * O hook criou o arquivo NY_FPS_SHM_PATH e o mapeou em memória.
 * O monitor abre o arquivo somente-leitura, verifica o magic/pid e
 * lê frame_count a cada segundo para calcular FPS.
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/** Caminho do arquivo de memória compartilhada criado pelo hook. */
static constexpr const char* NY_FPS_SHM_PATH = "/tmp/.ny_fps_hook";

/** Número mágico para validar que o arquivo foi criado pelo hook. */
static constexpr uint32_t NY_FPS_SHM_MAGIC   = 0x4E594650u;   // "NYFP"
static constexpr uint32_t NY_FPS_SHM_VERSION = 1u;

/**
 * Layout do arquivo mapeado (32 bytes, sem padding implícito — usa _pad explícito).
 * Campos atômicos estão alinhados a 8 bytes para acesso sem UB.
 *
 * Offset | Campo           | Bytes
 * -------|-----------------|------
 *   0    | magic           | 4
 *   4    | version         | 4
 *   8    | pid             | 4
 *  12    | _pad            | 4
 *  16    | frame_count     | 8
 *  24    | last_frame_ns   | 8
 * Total  |                 | 32
 */
typedef struct {
    uint32_t magic;           /**< NY_FPS_SHM_MAGIC                              */
    uint32_t version;         /**< NY_FPS_SHM_VERSION                            */
    uint32_t pid;             /**< PID do processo do jogo que criou este arquivo */
    uint32_t _pad;            /**< padding para alinhar campos de 64 bits        */
    uint64_t frame_count;     /**< Total de frames apresentados (atômico, crescente) */
    uint64_t last_frame_ns;   /**< Timestamp (CLOCK_MONOTONIC ns) do último frame */
} NyFpsShmData;

#ifdef __cplusplus
} // extern "C"
#endif

