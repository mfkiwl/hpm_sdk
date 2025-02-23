/*
 * Copyright (c) 2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include <stdio.h>
#include "hpm_debug_console.h"
#include "hpm_sysctl_drv.h"
#include "hpm_pwm_drv.h"
#include "hpm_trgm_drv.h"
#include "hpm_clock_drv.h"

#ifndef HRPWM
#define HRPWM BOARD_APP_HRPWM
#define PWM_CLOCK_NAME BOARD_APP_HRPWM_CLOCK_NAME
#define PWM_OUTPUT_PIN1 BOARD_APP_HRPWM_OUT1
#define PWM_OUTPUT_PIN2 BOARD_APP_HRPWM_OUT2
#define TRGM BOARD_APP_HRPWM_TRGM
#endif

#ifndef TEST_LOOP
#define TEST_LOOP (100)
#endif

#define PWM_PERIOD_IN_MS (0.01)
#define HRPWM_SET_IN_PWM_CLK (128)
uint32_t reload;

void config_hw_event(uint8_t cmp_index, uint32_t cmp)
{
    pwm_cmp_config_t cmp_config = {0};
    cmp_config.mode = pwm_cmp_mode_output_compare;
    cmp_config.cmp = cmp;
    cmp_config.update_trigger = pwm_shadow_register_update_on_modify;
    pwm_config_cmp(HRPWM, cmp_index, &cmp_config);
}

void reset_pwm_counter(void)
{
    pwm_enable_reload_at_synci(HRPWM);
    trgm_output_update_source(TRGM, TRGM_TRGOCFG_PWM_SYNCI, 1);
    trgm_output_update_source(TRGM, TRGM_TRGOCFG_PWM_SYNCI, 0);
}

void generate_edge_aligned_waveform(void)
{
    uint8_t cmp_index = 0;
    uint32_t duty, duty_step;
    pwm_cmp_config_t cmp_config[2] = {0};
    pwm_config_t pwm_config = {0};

    pwm_stop_counter(HRPWM);
    pwm_disable_hrpwm(HRPWM);
    reset_pwm_counter();
    pwm_get_default_pwm_config(HRPWM, &pwm_config);

    pwm_config.enable_output = true;
    pwm_config.dead_zone_in_half_cycle = 0;
    pwm_config.invert_output = false;

    pwm_cal_hrpwm_chn_start(HRPWM, cmp_index);
    pwm_cal_hrpwm_chn_start(HRPWM, cmp_index + 1);
    pwm_cal_hrpwm_chn_wait(HRPWM, cmp_index);
    pwm_cal_hrpwm_chn_wait(HRPWM, cmp_index + 1);

    pwm_enable_hrpwm(HRPWM);
    /*
     * reload and start counter
     */
    pwm_set_hrpwm_reload(HRPWM, 0, reload);
    pwm_set_start_count(HRPWM, 0, 0);

    /*
     * config cmp = RELOAD + 1
     */
    cmp_config[0].mode = pwm_cmp_mode_output_compare;
    cmp_config[0].cmp = reload + 1;
    cmp_config[0].enable_hrcmp = true;
    cmp_config[0].hrcmp = 0;
    cmp_config[0].update_trigger = pwm_shadow_register_update_on_hw_event;

    cmp_config[1].mode = pwm_cmp_mode_output_compare;
    cmp_config[1].cmp = reload;
    cmp_config[1].update_trigger = pwm_shadow_register_update_on_modify;
    /*
     * config pwm as output driven by cmp
     */
    if (status_success != pwm_setup_waveform(HRPWM, PWM_OUTPUT_PIN1, &pwm_config, cmp_index, &cmp_config[0], 1)) {
        printf("failed to setup waveform\n");
        while (1) {
        };
    }
    cmp_config[0].cmp = reload >> 1;
    /*
     * config pwm as reference
     */
    if (status_success != pwm_setup_waveform(HRPWM, PWM_OUTPUT_PIN2, &pwm_config, cmp_index + 1, &cmp_config[0], 1)) {
        printf("failed to setup waveform\n");
        while (1) {
        };
    }
    pwm_load_cmp_shadow_on_match(HRPWM, cmp_index + 2, &cmp_config[1]);

    pwm_start_counter(HRPWM);
    pwm_issue_shadow_register_lock_event(HRPWM);
    duty_step = reload / TEST_LOOP;
    duty = reload / TEST_LOOP;
    for (uint32_t i = 0; i < TEST_LOOP; i++) {
        if ((duty + duty_step) >= reload) {
            duty = duty_step;
        } else {
            duty += duty_step;
        }
        pwm_update_raw_hrcmp_edge_aligned(HRPWM, cmp_index, reload - duty, HRPWM_SET_IN_PWM_CLK);
        pwm_update_raw_hrcmp_edge_aligned(HRPWM, cmp_index + 1, reload - duty, 0);
        board_delay_ms(100);
    }
}

void generate_central_aligned_waveform(void)
{
    uint8_t cmp_index = 0;
    uint32_t duty, duty_step;
    pwm_cmp_config_t cmp_config[4] = {0};
    pwm_config_t pwm_config = {0};

    pwm_stop_counter(HRPWM);
    reset_pwm_counter();
    pwm_disable_hrpwm(HRPWM);
    pwm_get_default_pwm_config(HRPWM, &pwm_config);

    pwm_cal_hrpwm_chn_start(HRPWM, cmp_index);
    pwm_cal_hrpwm_chn_start(HRPWM, cmp_index + 1);
    pwm_cal_hrpwm_chn_wait(HRPWM, cmp_index);
    pwm_cal_hrpwm_chn_wait(HRPWM, cmp_index + 1);


    pwm_enable_hrpwm(HRPWM);
    /*
     * reload and start counter
     */
    pwm_set_reload(HRPWM, 0, reload);
    pwm_set_start_count(HRPWM, 0, 0);

    /*
     * config cmp1 and cmp2 and cmp3
     */
    cmp_config[0].mode = pwm_cmp_mode_output_compare;
    cmp_config[0].cmp = reload + 1;
    cmp_config[0].enable_hrcmp = true;
    cmp_config[0].hrcmp = 0;
    cmp_config[0].update_trigger = pwm_shadow_register_update_on_hw_event;

    cmp_config[1].mode = pwm_cmp_mode_output_compare;
    cmp_config[1].cmp = reload + 1;
    cmp_config[1].enable_hrcmp = true;
    cmp_config[1].hrcmp = 0;
    cmp_config[1].update_trigger = pwm_shadow_register_update_on_hw_event;

    cmp_config[3].mode = pwm_cmp_mode_output_compare;
    cmp_config[3].cmp = reload;
    cmp_config[3].update_trigger = pwm_shadow_register_update_on_modify;

    pwm_config.enable_output = true;
    pwm_config.dead_zone_in_half_cycle = 0;
    pwm_config.invert_output = false;
    /*
     * config pwm
     */
    if (status_success != pwm_setup_waveform(HRPWM, PWM_OUTPUT_PIN1, &pwm_config, cmp_index, cmp_config, 2)) {
        printf("failed to setup waveform\n");
        while (1) {
        };
    }
    /*
     * config pwm as reference
     */
    if (status_success != pwm_setup_waveform(HRPWM, PWM_OUTPUT_PIN2, &pwm_config, cmp_index + 2, cmp_config, 2)) {
        printf("failed to setup waveform\n");
        while (1) {
        };
    }
    pwm_load_cmp_shadow_on_match(HRPWM, cmp_index + 3,  &cmp_config[3]);
    pwm_start_counter(HRPWM);
    pwm_issue_shadow_register_lock_event(HRPWM);
    duty_step = reload / TEST_LOOP;
    duty = reload / TEST_LOOP;
    for (uint32_t i = 0; i < TEST_LOOP; i++) {

        if ((duty + duty_step) >= reload) {
            duty = duty_step;
        } else {
            duty += duty_step;
        }
        pwm_update_raw_hrcmp_central_aligned(HRPWM, cmp_index, cmp_index + 1, (reload - duty) >> 1, (reload + duty) >> 1, 0, HRPWM_SET_IN_PWM_CLK);
        board_delay_ms(100);
    }
}

void test_pwm_force_output(void)
{
    pwm_config_force_cmd_timing(HRPWM, pwm_force_immediately);
    pwm_enable_pwm_sw_force_output(HRPWM, PWM_OUTPUT_PIN1);
    pwm_enable_pwm_sw_force_output(HRPWM, PWM_OUTPUT_PIN2);

    printf("Output high\n");
    pwm_set_force_output(HRPWM,
            PWM_FORCE_OUTPUT(PWM_OUTPUT_PIN1, pwm_output_1)
            | PWM_FORCE_OUTPUT(PWM_OUTPUT_PIN2, pwm_output_1));
    pwm_enable_sw_force(HRPWM);
    board_delay_ms(5000);
    printf("Output low\n");
    pwm_set_force_output(HRPWM,
            PWM_FORCE_OUTPUT(PWM_OUTPUT_PIN1, pwm_output_0)
            | PWM_FORCE_OUTPUT(PWM_OUTPUT_PIN2, pwm_output_0));
    board_delay_ms(5000);
    pwm_disable_sw_force(HRPWM);

    pwm_disable_pwm_sw_force_output(HRPWM, PWM_OUTPUT_PIN1);
    pwm_disable_pwm_sw_force_output(HRPWM, PWM_OUTPUT_PIN2);
}

void disable_all_pwm_output(void)
{
    pwm_disable_output(HRPWM, PWM_OUTPUT_PIN1);
    pwm_disable_output(HRPWM, PWM_OUTPUT_PIN2);
}

int main(void)
{
    uint32_t freq;
    board_init();
    init_hrpwm_pins(HRPWM);
    printf("hr pwm example\n");

    freq = clock_get_frequency(PWM_CLOCK_NAME);
    reload = freq / 1000 * PWM_PERIOD_IN_MS - 1;

    printf("\n\n>> Test force HRPWM output on P%d and P%d\n", PWM_OUTPUT_PIN1, PWM_OUTPUT_PIN2);
    test_pwm_force_output();
    printf("\n\n>> Generate edge aligned waveform\n");
    printf("Two waveforms will be generated, HRPWM P%d is the target waveform\n", PWM_OUTPUT_PIN1);
    printf("whose duty cycle will be updated from 0 - 100; HRPWM P%d is a reference\n", PWM_OUTPUT_PIN2);
    generate_edge_aligned_waveform();
    printf("\n\n>> Generate central aligned waveform\n");
    printf("Two waveforms will be generated, HRPWM P%d is the target waveform\n", PWM_OUTPUT_PIN1);
    printf("whose duty cycle will be updated from 0 - 100; HRPWM P%d is a reference\n", PWM_OUTPUT_PIN2);
    generate_central_aligned_waveform();
    disable_all_pwm_output();
    printf("test done\n");
    while (1) {
    };
    return 0;
}
