#ifndef ___SETTINGS_IEICE_H___
#define ___SETTINGS_IEICE_H___

static const size_t KEY_SIZE      = 2;
static const size_t TEXT_SIZE     = 10;
static const size_t ITERATION_NUM = 100;

#define CALC_SPLIT_SIZE(NUM_SPLIT) ((1 << ((KEY_SIZE) * 8)) / (NUM_SPLIT))

#endif /* ___SETTINGS_IEICE_H___ */
