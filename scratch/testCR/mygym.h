#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H

#include "ns3/stats-module.h"
#include "ns3/opengym-module.h"
#include "ns3/spectrum-module.h"


namespace ns3 {

class Node;
class WifiMacQueue;
class Packet;


class MyGymEnv : public OpenGymEnv
{
    public:
    MyGymEnv ();
    MyGymEnv (uint32_t channelNum);
    virtual ~MyGymEnv ();
    static TypeId GetTypeId (void);
    virtual void DoDispose ();

    Ptr<OpenGymSpace> GetActionSpace();
    Ptr<OpenGymSpace> GetObservationSpace();
    bool GetGameOver();
    Ptr<OpenGymDataContainer> GetObservation();
    float GetReward();
    std::string GetExtraInfo();
    bool ExecuteActions(Ptr<OpenGymDataContainer> action);

    static void PerformCca(Ptr<MyGymEnv> entity, uint32_t channelId, Ptr<const SpectrumValue> avgPowerSpectralDensity);
  void CollectChannelOccupation(uint32_t chanId, uint32_t occupied);
  bool CheckIfReady();
  void ClearObs();

private:
  void ScheduleNextStateRead();
  Ptr<WifiMacQueue> GetQueue(Ptr<Node> node);
  bool SetCw(Ptr<Node> node, uint32_t cwMinValue=0, uint32_t cwMaxValue=0);

  Time m_interval = Seconds(0.1);
  Ptr<Node> m_currentNode;
  uint64_t m_rxPktNum;
  uint32_t m_channelNum;
  std::vector<uint32_t> m_channelOccupation;
  uint32_t m_currentChannel;

  uint32_t m_collisionTh;
  std::vector<uint32_t> m_collisions = {0,0,0,0,0,0,0,0,0,0,};
};

}

#endif // MY_GYM_ENTITY_H