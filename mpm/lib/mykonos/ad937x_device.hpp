//
// Copyright 2017 Ettus Research (National Instruments)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include "config/ad937x_config_t.hpp"
#include "config/ad937x_fir.hpp"
#include "config/ad937x_gain_ctrl_config.hpp"
#include "mpm/ad937x/adi_ctrl.hpp"
#include "ad937x_device_types.hpp"
#include "mpm/ad937x/ad937x_ctrl_types.hpp"
#include "adi/t_mykonos.h"
#include "adi/t_mykonos_gpio.h"
#include "adi/mykonos_debug/t_mykonos_dbgjesd.h"

#include <mpm/spi/spi_iface.hpp>
#include <mpm/exception.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include <functional>

class ad937x_device : public boost::noncopyable
{
public:
    enum class gain_mode_t { MANUAL, AUTOMATIC, HYBRID };
    enum pll_t : uint8_t
    {
        CLK_SYNTH   = 0x01,
        RX_SYNTH    = 0x02,
        TX_SYNTH    = 0x04,
        SNIFF_SYNTH = 0x08,
        CALPLL_SDM  = 0x10,
    };

    ad937x_device(
        mpm::types::regs_iface* iface,
        mpm::ad937x::gpio::gain_pins_t gain_pins
    );

    void begin_initialization();
    void finish_initialization();
    void setup_cal(uint32_t init_cals_mask, uint32_t tracking_cals_mask, uint32_t timeout);
    void start_jesd_rx();
    void start_jesd_tx();
    void start_radio();
    void stop_radio();

    uint8_t get_multichip_sync_status();
    uint8_t get_framer_status();
    uint8_t get_deframer_status();
    uint16_t get_ilas_config_match();
    void enable_jesd_loopback(uint8_t enable);

    uint8_t get_product_id();
    uint8_t get_device_rev();
    mpm::ad937x::device::api_version_t get_api_version();
    mpm::ad937x::device::arm_version_t get_arm_version();

    double set_bw_filter(uhd::direction_t direction, mpm::ad937x::device::chain_t chain, double value);
    void set_agc_mode(uhd::direction_t direction, gain_mode_t mode);
    double set_clock_rate(double value);
    void enable_channel(uhd::direction_t direction, mpm::ad937x::device::chain_t chain, bool enable);

    double set_gain(uhd::direction_t direction, mpm::ad937x::device::chain_t chain, double value);
    double get_gain(uhd::direction_t direction, mpm::ad937x::device::chain_t chain);

    double tune(uhd::direction_t direction, double value, bool wait_for_lock);
    double get_freq(uhd::direction_t direction);

    bool get_pll_lock_status(uint8_t pll, bool wait_for_lock = false);

    void set_fir(uhd::direction_t direction, int8_t gain, const std::vector<int16_t> & fir);
    std::vector<int16_t> get_fir(uhd::direction_t direction, int8_t &gain);

    int16_t get_temperature();

    void set_enable_gain_pins(uhd::direction_t direction, mpm::ad937x::device::chain_t chain, bool enable);
    void set_gain_pin_step_sizes(uhd::direction_t direction, mpm::ad937x::device::chain_t chain, double inc_step, double dec_step);
    void update_rx_lo_source(uint8_t rxPllUseExternalLo);
    void update_tx_lo_source(uint8_t rxPllUseExternalLo);
    uint8_t get_rx_lo_source();
    uint8_t get_tx_lo_source();
    void set_master_clock_rate(const double rate);
    const static double MIN_FREQ;
    const static double MAX_FREQ;
    const static double MIN_RX_GAIN;
    const static double MAX_RX_GAIN;
    const static double RX_GAIN_STEP;
    const static double MIN_TX_GAIN;
    const static double MAX_TX_GAIN;
    const static double TX_GAIN_STEP;

private:
    enum class multichip_sync_t { PARTIAL, FULL };
    enum class radio_state_t { OFF, ON };

    ad9371_spiSettings_t full_spi_settings;
    ad937x_config_t mykonos_config;
    ad937x_gain_ctrl_config_t gain_ctrl;

    void _apply_gain_pins(uhd::direction_t direction, mpm::ad937x::device::chain_t chain);
    void _setup_rf();
    void _call_api_function(const std::function<mykonosErr_t()>& func);
    void _call_gpio_api_function(const std::function<mykonosGpioErr_t()>& func);

    std::string _get_arm_binary_path();
    std::vector<uint8_t> _get_arm_binary();

    void _verify_product_id();
    void _verify_multichip_sync_status(multichip_sync_t mcs);

    radio_state_t _move_to_config_state();
    void _restore_from_config_state(radio_state_t state);

    static uint8_t _convert_rx_gain_to_mykonos(double gain);
    static double _convert_rx_gain_from_mykonos(uint8_t gain);
    static uint16_t _convert_tx_gain_to_mykonos(double gain);
    static double _convert_tx_gain_from_mykonos(uint16_t gain);
};
