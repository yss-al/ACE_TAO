// $Id$

#include "EC_Kokyu_Dispatching.h"
#include "EC_Event_Channel.h"
#include "EC_ProxySupplier.h"
#include "EC_QOS_Info.h"

#include "orbsvcs/Event_Service_Constants.h"
#include "orbsvcs/RtecSchedulerC.h"

#include "ace/Sched_Params.h"

#include "Kokyu/Kokyu.h"

#if ! defined (__ACE_INLINE__)
#include "EC_Kokyu_Dispatching.i"
#endif /* __ACE_INLINE__ */

ACE_RCSID(Event, EC_Kokyu_Dispatching, "$Id$")

TAO_EC_Kokyu_Dispatching::TAO_EC_Kokyu_Dispatching (TAO_EC_Event_Channel *ec)
  : dispatcher_(0)
{
  CORBA::Object_var tmp = ec->scheduler ();
  this->scheduler_ = RtecScheduler::Scheduler::_narrow (tmp.in ());
}

void
TAO_EC_Kokyu_Dispatching::activate (void)
{
  // Query the scheduler to get Config_Infos
  RtecScheduler::Config_Info_Set_var configs;
  this->scheduler_->get_config_infos(configs.out());
  ACE_TRY_CHECK;

  // Convert RtecScheduler::Config_Info_Set to Kokyu::ConfigInfoSet
  // OK to assume exact correspondence betwen Config_Info and ConfigInfo?
  Kokyu::ConfigInfoSet kconfigs;
  for(CORBA::ULong i=0; i<configs->length(); ++i) {
    kconfigs[i].preemption_priority_ = configs[i].preemption_priority;
    kconfigs[i].thread_priority_ = configs[i].thread_priority;
    switch (configs[i].dispatching_type) {
        case RtecScheduler::STATIC_DISPATCHING:
            kconfigs[i].dispatching_type_ = Kokyu::FIFO_DISPATCHING;
            break;
        case RtecScheduler::DEADLINE_DISPATCHING:
            kconfigs[i].dispatching_type_ = Kokyu::DEADLINE_DISPATCHING;
            break;
        case RtecScheduler::LAXITY_DISPATCHING:
            kconfigs[i].dispatching_type_ = Kokyu::LAXITY_DISPATCHING;
            break;
    }
  }

  // Create Kokyu::Dispatcher using factory
  this->dispatcher_ = Kokyu::Dispatcher_Factory::create_dispatcher(kconfigs);
}

void
TAO_EC_Kokyu_Dispatching::shutdown (void)
{
  this->dispatcher_->shutdown();
  //delete dispatcher_?
}

void
TAO_EC_Kokyu_Dispatching::push (TAO_EC_ProxyPushSupplier* proxy,
                                   RtecEventComm::PushConsumer_ptr consumer,
                                   const RtecEventComm::EventSet& event,
                                   TAO_EC_QOS_Info& qos_info
                                   ACE_ENV_ARG_DECL)
{
  RtecEventComm::EventSet event_copy = event;
  this->push_nocopy (proxy, consumer, event_copy, qos_info ACE_ENV_ARG_PARAMETER);
}

void
TAO_EC_Kokyu_Dispatching::push_nocopy (TAO_EC_ProxyPushSupplier* proxy,
                                          RtecEventComm::PushConsumer_ptr consumer,
                                          RtecEventComm::EventSet& event,
                                          TAO_EC_QOS_Info& qos_info
                                          ACE_ENV_ARG_DECL)
{
  if (this->dispatcher_ == 0)
    this->activate();

  // Create Dispatch_Command
  TAO_EC_Kokyu_Push_Command *cmd = new TAO_EC_Kokyu_Push_Command(proxy,consumer,event);

  // Convert TAO_EC_QOS_Info to QoSDescriptor
  RtecScheduler::RT_Info *rt_info = this->scheduler_->get(qos_info.rt_info);
  Kokyu::QoSDescriptor qosd;
  qosd.preemption_priority_ = qos_info.preemption_priority;
  qosd.deadline_ = rt_info->period;
  qosd.execution_time_ = rt_info->worst_case_execution_time;

  this->dispatcher_->dispatch(cmd,qosd);
}

// ****************************************************************

TAO_EC_Kokyu_Shutdown_Command::~TAO_EC_Kokyu_Shutdown_Command(void)
{
}

int
TAO_EC_Kokyu_Shutdown_Command::execute(void)
{
  return -1;
}

// ****************************************************************

TAO_EC_Kokyu_Push_Command::~TAO_EC_Kokyu_Push_Command(void)
{
  this->proxy_->_decr_refcnt ();
}

int
TAO_EC_Kokyu_Push_Command::execute (ACE_ENV_SINGLE_ARG_DECL)
{
  this->proxy_->push_to_consumer (this->consumer_.in (),
                                  this->event_
                                   ACE_ENV_ARG_PARAMETER);
  return 0;
}
