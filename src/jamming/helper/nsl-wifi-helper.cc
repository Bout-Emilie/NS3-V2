/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "nsl-wifi-helper.h"
#include "ns3/nsl-wifi-phy.h"
#include "ns3/nsl-wifi-channel.h"
#include "ns3/error-rate-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/wifi-net-device.h"
#include "ns3/radiotap-header.h"
#include "ns3/pcap-file-wrapper.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/node.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NslWifiHelper");


static void
AsciiPhyTransmitSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> p,
  WifiMode mode,
  WifiPreamble preamble,
  uint8_t txLevel)
{
  NS_LOG_FUNCTION (stream << context << p << mode << preamble << txLevel);
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " "
      << context << " " << *p << std::endl;
}

static void
AsciiPhyTransmitSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> p,
  WifiMode mode,
  WifiPreamble preamble,
  uint8_t txLevel)
{
  NS_LOG_FUNCTION (stream << p << mode << preamble << txLevel);
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " "
      << *p << std::endl;
}

static void
AsciiPhyReceiveSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> p,
  double snr,
  WifiMode mode,
  enum WifiPreamble preamble)
{
  NS_LOG_FUNCTION (stream << context << p << snr << mode << preamble);
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " "
      << context << " " << *p << std::endl;
}

static void
AsciiPhyReceiveSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> p,
  double snr,
  WifiMode mode,
  enum WifiPreamble preamble)
{
  NS_LOG_FUNCTION (stream << p << snr << mode << preamble);
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " "
      << *p << std::endl;
}

//----------------------------------------------------------------------------//

NslWifiChannelHelper::NslWifiChannelHelper ()
{}

NslWifiChannelHelper
NslWifiChannelHelper::Default (void)
{
  NS_LOG_INFO("Creating Topology");
  NslWifiChannelHelper helper;
  helper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  helper.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  return helper;
}

void
NslWifiChannelHelper::AddPropagationLoss (std::string type,
             std::string n0, const AttributeValue &v0,
             std::string n1, const AttributeValue &v1,
             std::string n2, const AttributeValue &v2,
             std::string n3, const AttributeValue &v3,
             std::string n4, const AttributeValue &v4,
             std::string n5, const AttributeValue &v5,
             std::string n6, const AttributeValue &v6,
             std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_propagationLoss.push_back (factory);
}

void
NslWifiChannelHelper::SetPropagationDelay (std::string type,
              std::string n0, const AttributeValue &v0,
              std::string n1, const AttributeValue &v1,
              std::string n2, const AttributeValue &v2,
              std::string n3, const AttributeValue &v3,
              std::string n4, const AttributeValue &v4,
              std::string n5, const AttributeValue &v5,
              std::string n6, const AttributeValue &v6,
              std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_propagationDelay = factory;
}

Ptr<NslWifiChannel>
NslWifiChannelHelper::Create (void) const
{
  NS_LOG_INFO("Creating Topology");
  Ptr<NslWifiChannel> channel = CreateObject<NslWifiChannel> ();
  Ptr<PropagationLossModel> prev = 0;
  for (std::vector<ObjectFactory>::const_iterator i = m_propagationLoss.begin (); i != m_propagationLoss.end (); ++i)
    {
      Ptr<PropagationLossModel> cur = (*i).Create<PropagationLossModel> ();
      if (prev != 0)
  {
    prev->SetNext (cur);
  }
      if (m_propagationLoss.begin () == i)
  {
    channel->SetPropagationLossModel (cur);
  }
      prev = cur;
    }
  Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
  channel->SetPropagationDelayModel (delay);
  
  return channel;
}

int64_t
NslWifiChannelHelper::AssignStreams (Ptr<NslWifiChannel> c, int64_t stream)
{
  return c->AssignStreams (stream);
}

//----------------------------------------------------------------------------//

NslWifiPhyHelper::NslWifiPhyHelper ()
  : m_channel (0),
    m_pcapDlt(PcapHelper::DLT_IEEE802_11)
{
  m_phy.SetTypeId ("ns3::NslWifiPhy");
}

NslWifiPhyHelper
NslWifiPhyHelper::Default (void)
{
  NslWifiPhyHelper helper;
  helper.SetErrorRateModel ("ns3::YansErrorRateModel");
  return helper;
}

void
NslWifiPhyHelper::SetChannel (Ptr<NslWifiChannel> channel)
{
  m_channel = channel;
}
void
NslWifiPhyHelper::SetChannel (std::string channelName)
{
  Ptr<NslWifiChannel> channel = Names::Find<NslWifiChannel> (channelName);
  m_channel = channel;
}
void
NslWifiPhyHelper::Set (std::string name, const AttributeValue &v)
{
  m_phy.Set (name, v);
}

void
NslWifiPhyHelper::SetErrorRateModel (std::string name,
                                     std::string n0, const AttributeValue &v0,
                                     std::string n1, const AttributeValue &v1,
                                     std::string n2, const AttributeValue &v2,
                                     std::string n3, const AttributeValue &v3,
                                     std::string n4, const AttributeValue &v4,
                                     std::string n5, const AttributeValue &v5,
                                     std::string n6, const AttributeValue &v6,
                                     std::string n7, const AttributeValue &v7)
{
  m_errorRateModel = ObjectFactory ();
  m_errorRateModel.SetTypeId (name);
  m_errorRateModel.Set (n0, v0);
  m_errorRateModel.Set (n1, v1);
  m_errorRateModel.Set (n2, v2);
  m_errorRateModel.Set (n3, v3);
  m_errorRateModel.Set (n4, v4);
  m_errorRateModel.Set (n5, v5);
  m_errorRateModel.Set (n6, v6);
  m_errorRateModel.Set (n7, v7);
}

Ptr<WifiPhy>
NslWifiPhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
{
  Ptr<NslWifiPhy> phy = m_phy.Create<NslWifiPhy> ();
  Ptr<ErrorRateModel> error = m_errorRateModel.Create<ErrorRateModel> ();
  phy->SetErrorRateModel (error);
  phy->SetChannel (m_channel);
  //phy->SetMobility (node);
  phy->SetDevice (device);
  phy->SetNode (node);
  return phy;
}

/*static void
PcapSniffTxEvent (
  Ptr<PcapFileWrapper> file,
  Ptr<const Packet>   packet,
  uint16_t            channelFreqMhz,
  uint16_t            channelNumber,
  uint32_t            rate,
  bool                isShortPreamble)
{
  uint32_t dlt = file->GetDataLinkType ();
  switch (dlt)
    {
    case PcapHelper::DLT_IEEE802_11:
      file->Write (Simulator::Now (), packet);
      return;
    case PcapHelper::DLT_PRISM_HEADER:
      {
        NS_FATAL_ERROR ("PcapSniffTxEvent(): DLT_PRISM_HEADER not implemented");
        return;
      }
    case PcapHelper::DLT_IEEE802_11_RADIO:
      {
        Ptr<Packet> p = packet->Copy ();
        RadiotapHeader header;
        uint8_t frameFlags = RadiotapHeader::FRAME_FLAG_NONE;
        header.SetTsft (Simulator::Now ().GetMicroSeconds ());
        // Our capture includes the FCS, so we set the flag to say so.
        frameFlags |= RadiotapHeader::FRAME_FLAG_FCS_INCLUDED;
        if (isShortPreamble)
          {
            frameFlags |= RadiotapHeader::FRAME_FLAG_SHORT_PREAMBLE;
          }
        header.SetFrameFlags (frameFlags);
        header.SetRate (rate);
        if (channelFreqMhz < 2500)
          {
            header.SetChannelFrequencyAndFlags (channelFreqMhz,
              RadiotapHeader::CHANNEL_FLAG_SPECTRUM_2GHZ | RadiotapHeader::CHANNEL_FLAG_CCK);
          }
        else
          {
            header.SetChannelFrequencyAndFlags (channelFreqMhz,
              RadiotapHeader::CHANNEL_FLAG_SPECTRUM_5GHZ | RadiotapHeader::CHANNEL_FLAG_OFDM);
          }
        p->AddHeader (header);
        file->Write (Simulator::Now (), p);
        return;
      }
    default:
      NS_ABORT_MSG ("PcapSniffTxEvent(): Unexpected data link type " << dlt);
    }
}
static void
PcapSniffRxEvent (
  Ptr<PcapFileWrapper> file,
  Ptr<const Packet> packet,
  uint16_t channelFreqMhz,
  uint16_t channelNumber,
  uint32_t rate,
  bool isShortPreamble,
  double signalDbm,
  double noiseDbm)
{
  uint32_t dlt = file->GetDataLinkType ();
  switch (dlt)
    {
    case PcapHelper::DLT_IEEE802_11:
      file->Write (Simulator::Now (), packet);
      return;
    case PcapHelper::DLT_PRISM_HEADER:
      {
        NS_FATAL_ERROR ("PcapSniffRxEvent(): DLT_PRISM_HEADER not implemented");
        return;
      }
    case PcapHelper::DLT_IEEE802_11_RADIO:
      {
        Ptr<Packet> p = packet->Copy ();
        RadiotapHeader header;
        uint8_t frameFlags = RadiotapHeader::FRAME_FLAG_NONE;
        header.SetTsft (Simulator::Now ().GetMicroSeconds ());
        // Our capture includes the FCS, so we set the flag to say so.
        frameFlags |= RadiotapHeader::FRAME_FLAG_FCS_INCLUDED;
        if (isShortPreamble)
          {
            frameFlags |= RadiotapHeader::FRAME_FLAG_SHORT_PREAMBLE;
          }
        header.SetFrameFlags (frameFlags);
        header.SetRate (rate);
        if (channelFreqMhz < 2500)
          {
            header.SetChannelFrequencyAndFlags (channelFreqMhz,
              RadiotapHeader::CHANNEL_FLAG_SPECTRUM_2GHZ | RadiotapHeader::CHANNEL_FLAG_CCK);
          }
        else
          {
            header.SetChannelFrequencyAndFlags (channelFreqMhz,
              RadiotapHeader::CHANNEL_FLAG_SPECTRUM_5GHZ | RadiotapHeader::CHANNEL_FLAG_OFDM);
          }
        header.SetAntennaSignalPower (signalDbm);
        header.SetAntennaNoisePower (noiseDbm);
        p->AddHeader (header);
        file->Write (Simulator::Now (), p);
        return;
      }
    default:
      NS_ABORT_MSG ("PcapSniffRxEvent(): Unexpected data link type " << dlt);
    }
}*/

void
NslWifiPhyHelper::SetPcapDataLinkType (enum SupportedPcapDataLinkTypes dlt)
{
  switch (dlt)
    {
    case DLT_IEEE802_11:
      m_pcapDlt = PcapHelper::DLT_IEEE802_11;
      return;
    case DLT_PRISM_HEADER:
      m_pcapDlt = PcapHelper::DLT_PRISM_HEADER;
      return;
    case DLT_IEEE802_11_RADIO:
      m_pcapDlt = PcapHelper::DLT_IEEE802_11_RADIO;
      return;
    default:
      NS_ABORT_MSG ("NslWifiPhyHelper::SetPcapFormat(): Unexpected format");
    }
}

void
NslWifiPhyHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type WifiNetDevice.
  //
  Ptr<WifiNetDevice> device = nd->GetObject<WifiNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("NslWifiHelper::EnablePcapInternal(): Device " << &device << " not of type ns3::WifiNetDevice");
      return;
    }

  Ptr<WifiPhy> phy = device->GetPhy ();
  NS_ABORT_MSG_IF (phy == 0, "NslWifiPhyHelper::EnablePcapInternal(): Phy layer in WifiNetDevice must be set");

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_IEEE802_11);

  phy->TraceConnectWithoutContext ("PromiscSnifferTx", MakeBoundCallback (&PcapSniffTxEvent, file));
  phy->TraceConnectWithoutContext ("PromiscSnifferRx", MakeBoundCallback (&PcapSniffRxEvent, file));
}

void
NslWifiPhyHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type CsmaNetDevice.
  //
  Ptr<WifiNetDevice> device = nd->GetObject<WifiNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("NslWifiHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::WifiNetDevice");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to make sure
  // that is turned on.
  //
  Packet::EnablePrinting ();

  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and write our traces
  // without a context since there will be one file per context and therefore
  // the context would be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      //
      // We could go poking through the phy and the state looking for the
      // correct trace source, but we can let Config deal with that with
      // some search cost.  Since this is presumably happening at topology
      // creation time, it doesn't seem much of a price to pay.
      //
      oss.str ("");
      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WifiNetDevice/Phy/State/RxOk";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyReceiveSinkWithoutContext, stream));

      oss.str ("");
      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WifiNetDevice/Phy/State/Tx";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyTransmitSinkWithoutContext, stream));

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with coming up with a context.
  //
  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WifiNetDevice/Phy/State/RxOk";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WifiNetDevice/Phy/State/Tx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyTransmitSinkWithContext, stream));
}

} // namespace ns3