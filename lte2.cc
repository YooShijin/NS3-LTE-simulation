
#include "ns3/lte-helper.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/spectrum-module.h"
#include <ns3/buildings-helper.h>
#include <random>
#include <iostream>
#include <fstream>

//Flow Monitor Headers
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NS3AssingmentLTE_v2");

void
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename, std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (auto it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject<LteUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << pos.x << " " << pos.y << std::endl;
            }
        }
    }
}

void
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename, std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (auto it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject<LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << pos.x << " " << pos.y << std::endl;
            }
        }
    }
}

void
PrintGnuplottableThroughputToFile (std::string filename, std::string scheduler, double throughput,
                                   double speed)
{
  std::ofstream outFile;
  outFile.open (filename, std::ios_base::out | std::ios_base::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << speed;
  outFile << " ";
  outFile << scheduler;
  outFile << " ";
  outFile << throughput;
  outFile << "\n";
}
// Flowmonitor
// Ptr<FlowMonitor> monitor;
// FlowMonitorHelper flowHelper;
// void
// monitoringSetup ()
// {

//   monitor->CheckForLostPackets ();
//   // Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
//   // // Ptr<Ipv4FlowClassifier> classifier = dynamic_cast<Ptr<Ipv4FlowClassifier>>(flowHelper.GetClassifier ());

//   // std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

//   // for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
//   // {
//   // 	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
//   // 		std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
//   // 		std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
//   // 		std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
//   // 		std::cout << "  Throughput:   " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps\n";
//   // 		std::cout << "  Throughput:   " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()) / 1048576 << " Mbps\n";
//   // 		std::cout << "  Throughput:   " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()) / 1073741824 << " Gbps\n";
//   // }

//   monitor->SerializeToXmlFile ("./output/flow.xml", true, true);
// }

int
main (int argc, char *argv[])
{

  uint16_t numberOfUes = 40;
  uint16_t numberofEnbs = 4;
  double simTime = 5.000;
  bool generateRem = false;
  bool fullBuffer = false;
  int32_t remRbId = -1;
  // uint16_t bandwidth = 50;
  double distance = 5000;
  double radius = 500;
  double speed = 10;
  double MaxPacketSize = 1500;
  double intervalTime = 10.0;
  std::string schedulerType = "PF";
  std::string scheduler;

  

  uint16_t RngRun = 15;

  Box macroUeBox = Box (-distance * 0.5, distance * 1.5, -distance * 0.5, distance * 1.5, 1.5, 1.5);

  // Command line arguments
  CommandLine cmd (__FILE__);

  cmd.AddValue ("speed", "Speed of the UE's", speed);
  cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)", simTime);
  cmd.AddValue ("generateRem", "if true, will generate a REM.", generateRem);
  cmd.AddValue ("rngRun", "The value of RngRun", RngRun);
  cmd.AddValue ("schedulerType",
                "Scheduler type (PF for PfFfMacScheduler(default), RR for RrFfMacScheduler,MT for TdMtFfMacScheduler, PSS for PssFfMacScheduler)",
                schedulerType);
  cmd.AddValue ("fullBuffer", "Whether using full Buffer case or not", fullBuffer);

  cmd.Parse (argc, argv);

  RngSeedManager::SetSeed (5);
  RngSeedManager::SetRun (RngRun);

  if (schedulerType == "PF")
        {
             scheduler = "PfFfMacScheduler"; // Default scheduler type
        }
        else if (schedulerType == "RR")
        {
            scheduler = "RrFfMacScheduler";
        }
        else if (schedulerType == "PSS")
        {
            scheduler = "PssFfMacScheduler";
        }
        else if (schedulerType == "MT")
        {
            scheduler = "TdMtFfMacScheduler";
        }

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetHandoverAlgorithmType (
      "ns3::A3RsrpHandoverAlgorithm"); //enable automatic handover using A3RsrpHandoverAlgorithm,
  // lteHelper->SetHandoverAlgorithmType("ns3::NoOpHandoverAlgorithm"); // disable automatic handover

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);

  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
      ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  //setting transmission power
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (40.0));

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer randomUeNodes;
  enbNodes.Create (numberofEnbs);
  randomUeNodes.Create (numberOfUes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 0.0)); // eNB1
  enbPositionAlloc->Add (Vector (distance, 0.0, 0.0)); // eNB2
  enbPositionAlloc->Add (Vector (distance, distance, 0.0)); // eNB3
  enbPositionAlloc->Add (Vector (0.0, distance, 0.0)); // eNB4

  // Adding constant mobility model to all 4 enbs
  MobilityHelper mobility1;
  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility1.SetPositionAllocator (enbPositionAlloc);
  mobility1.Install (enbNodes);

  //Adding RandomWalk2d mobility model to all Ues
  MobilityHelper mobility;
  // mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  //   mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds",
  //                              RectangleValue (Rectangle (-2500, 7500, -2500, 7500)), "Time",
  //                              StringValue ("1s"), "Mode", StringValue ("Time"), "Speed",
  //                              StringValue ("ns3::ConstantRandomVariable[Constant=10.0]"));

  // Seed the random number generator
  std::random_device rd;
  std::mt19937 gen (rd ());

  // Define a uniform distribution to generate 0 or 10
  std::uniform_int_distribution<> dis (0, 1);

  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", "X", DoubleValue (0.0), "Y",
                                 DoubleValue (0.0), "rho", DoubleValue (radius));

  for (int i = 0; i < 10; i++)
    {

      // Generate a random number (either 0 or 1)
      int randomNumber = dis (gen);

      // Multiply by 10 to get either 0 or 10
      int result = randomNumber * 10;

      mobility.SetMobilityModel (
          "ns3::RandomWalk2dMobilityModel", "Bounds",
          RectangleValue (Rectangle (-2500, 7500, -2500, 7500)), "Time", StringValue ("1s"), "Mode",
          StringValue ("Time"), "Speed",
          StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (speed) + "]"));

      mobility.Install (randomUeNodes.Get (i));
    }

  //10 - 20 ues
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", "X", DoubleValue (distance),
                                 "Y", DoubleValue (0.0), "rho", DoubleValue (radius));

  for (int i = 10; i < 20; i++)
    {

      // Generate a random number (either 0 or 1)
      int randomNumber = dis (gen);

      // Multiply by 10 to get either 0 or 10
      int result = randomNumber * 10;

      mobility.SetMobilityModel (
          "ns3::RandomWalk2dMobilityModel", "Bounds",
          RectangleValue (Rectangle (-2500, 7500, -2500, 7500)), "Time", StringValue ("1s"), "Mode",
          StringValue ("Time"), "Speed",
          StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (speed) + "]"));
      mobility.Install (randomUeNodes.Get (i));
    }

  //20-30 ues
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", "X", DoubleValue (distance),
                                 "Y", DoubleValue (distance), "rho", DoubleValue (radius));

  for (int i = 20; i < 30; i++)
    {

      // Generate a random number (either 0 or 1)
      int randomNumber = dis (gen);

      // Multiply by 10 to get either 0 or 10
      int result = randomNumber * 10;

      mobility.SetMobilityModel (
          "ns3::RandomWalk2dMobilityModel", "Bounds",
          RectangleValue (Rectangle (-2500, 7500, -2500, 7500)), "Time", StringValue ("1s"), "Mode",
          StringValue ("Time"), "Speed",
          StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (speed) + "]"));
      mobility.Install (randomUeNodes.Get (i));
    }

  //30-40 ues
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", "X", DoubleValue (0.0), "Y",
                                 DoubleValue (distance), "rho", DoubleValue (radius));

  for (int i = 30; i < 40; i++)
    {

      // Generate a random number (either 0 or 1)
      int randomNumber = dis (gen);

      // Multiply by 10 to get either 0 or 10
      int result = randomNumber * 10;

      mobility.SetMobilityModel (
          "ns3::RandomWalk2dMobilityModel", "Bounds",
          RectangleValue (Rectangle (-2500, 7500, -2500, 7500)), "Time", StringValue ("1s"), "Mode",
          StringValue ("Time"), "Speed",
          StringValue ("ns3::ConstantRandomVariable[Constant=" + std::to_string (speed) + "]"));
      mobility.Install (randomUeNodes.Get (i));
    }

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer randomUeDevs;
  lteHelper->SetSchedulerType ("ns3::" + scheduler);

  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (50));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (50));

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  randomUeDevs = lteHelper->InstallUeDevice (randomUeNodes);

  NodeContainer ueNodes;
  ueNodes.Add (randomUeNodes);
  NetDeviceContainer ueDevs;
  ueDevs.Add (randomUeDevs);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  // Attach a UE to a eNB
  lteHelper->AttachToClosestEnb (ueDevs, enbDevs);

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // randomize a bit start times to avoid simulation artifacts
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
          ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      if (fullBuffer)
        {
          intervalTime = 1.0;
        }

      // Set the UDP Application for throughput calculation
      for (uint32_t b = 0; b < 1; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          dlClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
          dlClientHelper.SetAttribute ("Interval", TimeValue (MilliSeconds (intervalTime)));
          dlClientHelper.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (remoteHost));
          serverApps.Add (dlPacketSinkHelper.Install (ue));

          // UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          // ulClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
          // ulClientHelper.SetAttribute ("Interval", TimeValue (MilliSeconds (1.0)));
          // clientApps.Add (ulClientHelper.Install (ue));
          // PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
          //                                      InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          // serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
          //Data radio bearer
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueDevs.Get (u), bearer, tft);

          Time startTime = Seconds (startTimeSeconds->GetValue ());
          serverApps.Start (startTime);
          clientApps.Start (startTime);
        }
    }

  // AnimationInterface anim ("result/model15.xml"); // Mandatory
  // anim.EnablePacketMetadata(true);
  NS_LOG_INFO ("Simulation started !");

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (generateRem)
    {
      PrintGnuplottableEnbListToFile ("./output/enbs_lte2.txt");
      PrintGnuplottableUeListToFile ("./output/ues_lte2.txt");

      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      Ptr<LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)
                                                 ->GetObject<LteEnbNetDevice> ()
                                                 ->GetPhy ()
                                                 ->GetDownlinkSpectrumPhy ()
                                                 ->GetObject<LteSpectrumPhy> ();
      Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();
      uint32_t dlChannelId = dlChannel->GetId ();
      NS_LOG_INFO ("DL ChannelId: " << dlChannelId);
      remHelper->SetAttribute ("Channel", PointerValue (dlChannel));
      remHelper->SetAttribute ("OutputFile", StringValue ("./output/lte_assign2.rem"));
      remHelper->SetAttribute ("XMin", DoubleValue (macroUeBox.xMin));
      remHelper->SetAttribute ("XMax", DoubleValue (macroUeBox.xMax));
      remHelper->SetAttribute ("YMin", DoubleValue (macroUeBox.yMin));
      remHelper->SetAttribute ("YMax", DoubleValue (macroUeBox.yMax));
      remHelper->SetAttribute ("Z", DoubleValue (1.5));
      remHelper->SetAttribute ("XRes", UintegerValue (500));
      remHelper->SetAttribute ("YRes", UintegerValue (500));

      // set view map;
      // set term x11;
      // set xlabel "X"
      // set ylabel "Y"
      // set cblabel "SINR (dB)"
      // plot "lte_assign.rem" using ($1):($2):(10*log10($4)) with image

      // if (remRbId >= 0)
      //   {
      //     remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
      //     remHelper->SetAttribute ("RbId", IntegerValue (remRbId));
      //   }

      remHelper->Install ();
    }
  // else
  //   {
  //     Simulator::Stop (Seconds (simTime));
  //   }
  // monitor = flowHelper.Install(randomUeNodes);

  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (randomUeNodes);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  std::string outputFile = "./output/throughput.txt";
  std::ofstream outFile;
  outFile.open (outputFile, std::ios_base::out | std::ios_base::trunc);
  outFile << scheduler;

  //   Print per-flow statistics

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  double averageFlowThroughput = 0.0;
  double TotalaverageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;
  double TotalaverageFlowDelay = 0.0;
  double PacketsLost = 0;
  double TotalPacketsLost = 0;
  double averagePacketLoss = 0;
  int totalPackets = 0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::stringstream protoStream;
      protoStream << (uint16_t) t.protocol;
      if (t.protocol == 6)
        {
          protoStream.str ("TCP");
        }
      if (t.protocol == 17)
        {
          protoStream.str ("UDP");
        }
      outFile << "\nFlow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> "
              << t.destinationAddress << ":" << t.destinationPort << ") proto "
              << protoStream.str () << "\n";
      outFile << "  Tx Packets: " << i->second.txPackets << "\n";
      outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
      //   outFile << "  TxOffered:  "
      //           << i->second.txBytes * 8.0 / (simTime - udpAppStartTime) / 1000 / 1000 << " Mbps\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          double rxDuration =
              i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();

          averageFlowThroughput += i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;

          averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

          outFile << "  Throughput: " << i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000
                  << " Mbps\n";
          outFile << "   Mean flow Throughput: " << averageFlowThroughput / stats.size ()
                  << " Mbps\n";

          outFile << "  Mean delay:  "
                  << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
          outFile << "  Mean jitter:  "
                  << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets << " ms\n";
        }
      else
        {
          outFile << "  Throughput:  0 Mbps\n";
          outFile << "  Mean delay:  0 ms\n";
          outFile << "  Mean upt:  0  Mbps \n";
          outFile << "  Mean jitter: 0 ms\n";
        }
      outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
      double packetLoss = i->second.txPackets - i->second.rxPackets;
      double lossRate = (packetLoss / i->second.txPackets) * 100;
      outFile << " Loss Rate: " << lossRate << "\n";
      averagePacketLoss += packetLoss / i->second.txPackets;
    }

  PrintGnuplottableThroughputToFile ("./output/data.txt", scheduler,
                                     averageFlowThroughput, speed);

  Simulator::Destroy ();
  NS_LOG_INFO ("Simulator Terminated !");
  return 0;
}

       