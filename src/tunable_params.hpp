#pragma once

#include <cstdint>

static constexpr int TIME_CHECK_FREQUENCY = 1024;
static constexpr int NMP_DEPTH_REDUCTION = 3;
static constexpr int RFP_SCALE = 100;
static constexpr int FP_MAX_DEPTH = 6;
static constexpr int FP_BASE = 100;
static constexpr int FP_SCALE = 150;
static constexpr int LMR_MIN_DEPTH = 2;
static constexpr double LMR_A = 0.8;
static constexpr double LMR_B = 0.4;
static constexpr int ASP_DELTA = 30;
static constexpr double ASP_MULTIPLIER = 2;
static constexpr int64_t DATAGEN_SOFT_NODE_LIMIT = 5000;
static constexpr int64_t DATAGEN_HARD_NODE_LIMIT =
    DATAGEN_SOFT_NODE_LIMIT * 100;
static constexpr const char *NET_PATH = "nnue.bin";
static constexpr int HL = 128, SCALE = 400, QA = 255, QB = 64;
