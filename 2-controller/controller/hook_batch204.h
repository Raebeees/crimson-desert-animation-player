#pragma once
#include "MinHook.h"
// Caller serializes startup, holds the readiness gate closed and keeps
// callbacks, modules and all four captured trampolines alive throughout.
// Targets have already been identity checked and created in this MinHook instance.
typedef struct BootsHookBatchResult {
    MH_STATUS activation;
    MH_STATUS rollback;
} BootsHookBatchResult;

static MH_STATUS boots_disable_batch(void *const targets[4]) {
    MH_STATUS first = MH_OK;
    unsigned i;
    for (i = 0; i < 4; ++i) {
        MH_STATUS status = MH_QueueDisableHook(targets[i]);
        if (first == MH_OK && status != MH_OK) first = status;
    }
    // An uncorrected queued enable could otherwise be applied here.
    if (first != MH_OK) return first;
    return MH_ApplyQueued();
}

static BootsHookBatchResult boots_enable_batch(void *const targets[4]) {
    BootsHookBatchResult result = {MH_OK, MH_OK};
    unsigned i;
    for (i = 0; i < 4; ++i) {
        unsigned j;
        if (!targets[i]) { result.activation = MH_ERROR_NOT_CREATED; return result; }
        for (j = 0; j < i; ++j) {
            if (targets[i] == targets[j]) { result.activation = MH_ERROR_ALREADY_CREATED; return result; }
        }
    }
    for (i = 0; i < 4; ++i) {
        result.activation = MH_QueueEnableHook(targets[i]);
        if (result.activation != MH_OK) break;
    }
    if (result.activation == MH_OK) result.activation = MH_ApplyQueued();
    if (result.activation != MH_OK) result.rollback = boots_disable_batch(targets);
    return result;
}
