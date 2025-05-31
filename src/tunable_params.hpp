#pragma once

#include <cstdint>

inline constexpr int TIME_CHECK_FREQUENCY = 1024;
inline constexpr int NMP_DEPTH_REDUCTION = 3;
static constexpr int RFP_SCALE = 100;
inline constexpr int FP_MAX_DEPTH = 6;
inline constexpr int FP_BASE = 100;
inline constexpr int FP_SCALE = 150;
inline constexpr int LMR_MIN_DEPTH = 2;
inline constexpr double LMR_A = 0.8;
inline constexpr double LMR_B = 0.4;
inline constexpr int ASP_DELTA = 30;
inline constexpr double ASP_MULTIPLIER = 2;
inline constexpr int64_t DATAGEN_SOFT_NODE_LIMIT = 5000;
inline constexpr int64_t DATAGEN_HARD_NODE_LIMIT =
    DATAGEN_SOFT_NODE_LIMIT * 100;
inline constexpr const char *NET_PATH = "nnue.bin";
inline constexpr int HL = 128, SCALE = 400, QA = 255, QB = 64;
