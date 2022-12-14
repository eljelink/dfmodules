/**
 * @file FragmentReceiver.cpp FragmentReceiver class implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "FragmentReceiver.hpp"
#include "dfmodules/CommonIssues.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/app/Nljs.hpp"
#include "dfmodules/fragmentreceiver/Nljs.hpp"
#include "dfmodules/fragmentreceiver/Structs.hpp"
#include "dfmodules/fragmentreceiverinfo/InfoNljs.hpp"
#include "logging/Logging.hpp"
#include "iomanager/IOManager.hpp"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FragmentReceiver" // NOLINT
enum
{
  TLVL_ENTER_EXIT_METHODS = 5,
  TLVL_CONFIG = 7,
  TLVL_WORK_STEPS = 10
};

namespace dunedaq {
namespace dfmodules {

FragmentReceiver::FragmentReceiver(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
  , m_queue_timeout(100)
  , m_run_number(0)
{
  register_command("conf", &FragmentReceiver::do_conf);
  register_command("start", &FragmentReceiver::do_start);
  register_command("stop", &FragmentReceiver::do_stop);
  register_command("scrap", &FragmentReceiver::do_scrap);
}

void
FragmentReceiver::init(const data_t& iniobj)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  m_input_connection = appfwk::connection_inst(iniobj, "input");
  m_fragment_output = iomanager::IOManager::get()->get_sender<internal_data_t>( appfwk::connection_inst(iniobj, "output") );
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
FragmentReceiver::do_conf(const data_t& payload)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_conf() method";
  fragmentreceiver::ConfParams parsed_conf = payload.get<fragmentreceiver::ConfParams>();

  m_queue_timeout = std::chrono::milliseconds(parsed_conf.general_queue_timeout);

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_conf() method";
}

void
FragmentReceiver::do_start(const data_t& payload)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";

  m_received_fragments = 0;
  m_run_number = payload.value<dunedaq::daqdataformats::run_number_t>("run", 0);

  iomanager::IOManager::get()->add_callback<internal_data_t>(m_input_connection,
							     std::bind(&FragmentReceiver::dispatch_fragment, this, std::placeholders::_1));
  TLOG() << get_name() << " successfully started for run number " << m_run_number;
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
FragmentReceiver::do_stop(const data_t& /*args*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";

  iomanager::IOManager::get()->remove_callback<internal_data_t>(m_input_connection);
  TLOG() << get_name() << " successfully stopped";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

void
FragmentReceiver::do_scrap(const data_t& /*args*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_scrap() method";
  TLOG() << get_name() << " successfully stopped";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_scrap() method";
}

void
FragmentReceiver::get_info(opmonlib::InfoCollector& ci, int /*level*/)
{
  fragmentreceiverinfo::Info info;
  info.fragments_received = m_received_fragments;
  ci.add(info);
}

void
FragmentReceiver::dispatch_fragment(internal_data_t & fragment)
{
  m_fragment_output->send(std::move(fragment), m_queue_timeout);
  m_received_fragments++;
}

} // namespace dfmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::dfmodules::FragmentReceiver)
