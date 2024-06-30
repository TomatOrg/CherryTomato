#pragma once

/**
 * Initialize performance monitoring
 */
void perf_init();

/**
 * Start counting
 */
void perf_start();

/**
 * Finish counting
 */
long long perf_stop();
