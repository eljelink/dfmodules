/**
 * @file TriggerInhibitAgent.hpp TriggerInhibitAgent Class
 *
 * The TriggerInhibitAgent class provides functionality to determine
 * if a TriggerInhibit needs to be generated.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef DFMODULES_SRC_DFMODULES_TRIGGERINHIBITAGENT_HPP_
#define DFMODULES_SRC_DFMODULES_TRIGGERINHIBITAGENT_HPP_

#include "iomanager/Sender.hpp"
#include "iomanager/Receiver.hpp"
#include "utilities/NamedObject.hpp"
#include "daqdataformats/Types.hpp"
#include "dfmessages/TriggerDecision.hpp"
#include "dfmessages/TriggerInhibit.hpp"
#include "utilities/WorkerThread.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

namespace dunedaq {
namespace dfmodules {

/**
 * @brief
 */
class TriggerInhibitAgent : public utilities::NamedObject
{
public:
  using trigdecreceiver_t = iomanager::ReceiverConcept<dfmessages::TriggerDecision>;
  using triginhsender_t = iomanager::SenderConcept<dfmessages::TriggerInhibit>;

  /**
   * @brief TriggerInhibitAgent Constructor
   */
  explicit TriggerInhibitAgent(const std::string&,
			       std::shared_ptr<trigdecreceiver_t>,
			       std::shared_ptr<triginhsender_t>);

  TriggerInhibitAgent(const TriggerInhibitAgent&) = delete; ///< TriggerInhibitAgent is not copy-constructible
  TriggerInhibitAgent& operator=(const TriggerInhibitAgent&) = delete; ///< TriggerInhibitAgent is not copy-assignable
  TriggerInhibitAgent(TriggerInhibitAgent&&) = delete;            ///< TriggerInhibitAgent is not move-constructible
  TriggerInhibitAgent& operator=(TriggerInhibitAgent&&) = delete; ///< TriggerInhibitAgent is not move-assignable

  void start_checking();

  void stop_checking();

  void set_threshold_for_inhibit(uint32_t value) // NOLINT
  {
    m_threshold_for_inhibit.store(value);
  }

  void set_latest_trigger_number(daqdataformats::trigger_number_t trig_num)
  {
    m_trigger_number_at_end_of_processing_chain.store(trig_num);
  }

private:
  // Threading
  dunedaq::utilities::WorkerThread m_thread;
  void do_work(std::atomic<bool>&);

  // Configuration
  std::chrono::milliseconds m_queue_timeout;
  std::atomic<uint32_t> m_threshold_for_inhibit; // NOLINT

  // Queue(s)
  std::shared_ptr<trigdecreceiver_t> m_trigger_decision_receiver;
  std::shared_ptr<triginhsender_t> m_trigger_inhibit_sender;

  // Internal data
  std::atomic<daqdataformats::trigger_number_t> m_trigger_number_at_start_of_processing_chain;
  std::atomic<daqdataformats::trigger_number_t> m_trigger_number_at_end_of_processing_chain;
};
} // namespace dfmodules
} // namespace dunedaq

#endif // DFMODULES_SRC_DFMODULES_TRIGGERINHIBITAGENT_HPP_
