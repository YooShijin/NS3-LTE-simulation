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
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"

#include <fstream>
#include <sstream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Asg1PartA");

void PrintUeList(std::string filename)
{
  std::ofstream outFile;
  outFile.open(filename, std::ios_base::out | std::ios_base::trunc);
  for (auto it = NodeList::Begin(); it != NodeList::End(); ++it)
  {
    Ptr<Node> node = *it;
    for (int j = 0; j < (int)node->GetNDevices(); j++)
    {
      Ptr<LteUeNetDevice> uedev = node->GetDevice(j)->GetObject<LteUeNetDevice>();
      if (uedev)
      {
        Vector pos = node->GetObject<MobilityModel>()->GetPosition();
        outFile << pos.x << " " << pos.y << std::endl;
      }
    }
  }
}

void PrintEnbList(std::string filename)
{
  std::ofstream outFile;
  outFile.open(filename, std::ios_base::out | std::ios_base::trunc);
  for (auto it = NodeList::Begin(); it != NodeList::End(); ++it)
  {
    Ptr<Node> node = *it;
    for (int j = 0; j < (int)node->GetNDevices(); j++)
    {
      Ptr<LteEnbNetDevice> enbdev = node->GetDevice(j)->GetObject<LteEnbNetDevice>();
      if (enbdev)
      {
        Vector pos = node->GetObject<MobilityModel>()->GetPosition();
        outFile << pos.x << " " << pos.y << std::endl;
      }
    }
  }
}

void AppendAggregate(std::string filename, std::string scheduler, double throughput, double speed)
{
  std::ofstream outFile;
  outFile.open(filename, std::ios_base::out | std::ios_base::app);
  outFile << speed << " " << scheduler << " " << throughput << "\n";
}

int main(int argc, char *argv[])
{
  uint16_t numberOfUes = 40;
  uint16_t numberOfEnbs = 4;
  double simTime = 30.000;
  bool generateRem = false;
  bool fullBuffer = false;
  double distance = 5000;
  double radius = 500;
  double speed = 10;
  double maxPacketSize = 1500;
  double intervalTime = 10.0;
  std::string schedulerType = "PF";
  std::string scheduler = "PfFfMacScheduler";
  uint16_t RngRun = 15;

  Box macroUeBox = Box(-distance * 0.5, distance * 1.5, -distance * 0.5, distance * 1.5, 1.5, 1.5);

  CommandLine cmd(__FILE__);
  cmd.AddValue("speed", "Speed of the UEs", speed);
  cmd.AddValue("simTime", "Total duration of the simulation in seconds", simTime);
  cmd.AddValue("generateRem", "if true, will generate a REM", generateRem);
  cmd.AddValue("rngRun", "The value of RngRun", RngRun);
  cmd.AddValue("schedulerType", "PF, RR, MT or PSS", schedulerType);
  cmd.AddValue("fullBuffer", "Whether using full buffer case or not", fullBuffer);
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed(5);
  RngSeedManager::SetRun(RngRun);

  if (schedulerType == "RR")
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

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);
  lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");

  Ptr<Node> pgw = epcHelper->GetPgwNode();

  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
      ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(40.0));

  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(numberOfEnbs);
  ueNodes.Create(numberOfUes);

  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
  enbPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
  enbPositionAlloc->Add(Vector(distance, 0.0, 0.0));
  enbPositionAlloc->Add(Vector(distance, distance, 0.0));
  enbPositionAlloc->Add(Vector(0.0, distance, 0.0));

  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator(enbPositionAlloc);
  enbMobility.Install(enbNodes);

  std::string speedStr = "ns3::ConstantRandomVariable[Constant=" + std::to_string(speed) + "]";
  double centres[4][2] = {{0.0, 0.0}, {distance, 0.0}, {distance, distance}, {0.0, distance}};

  MobilityHelper ueMobility;
  for (int c = 0; c < 4; ++c)
  {
    ueMobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "X",
                                    DoubleValue(centres[c][0]), "Y",
                                    DoubleValue(centres[c][1]), "rho", DoubleValue(radius));
    ueMobility.SetMobilityModel(
        "ns3::RandomWalk2dMobilityModel", "Bounds",
        RectangleValue(Rectangle(-2500, 7500, -2500, 7500)), "Time", StringValue("1s"), "Mode",
        StringValue("Time"), "Speed", StringValue(speedStr));
    for (int i = c * 10; i < (c + 1) * 10; ++i)
    {
      ueMobility.Install(ueNodes.Get(i));
    }
  }

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType("ns3::" + scheduler);
  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(50));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(50));
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  ueDevs = lteHelper->InstallUeDevice(ueNodes);

  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));
  lteHelper->AttachToClosestEnb(ueDevs, enbDevs);

  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  if (fullBuffer)
  {
    intervalTime = 1.0;
  }

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable>();
  startTimeSeconds->SetAttribute("Min", DoubleValue(0));
  startTimeSeconds->SetAttribute("Max", DoubleValue(0.010));

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
  {
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(ueNodes.Get(u)->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

    ++dlPort;
    ++ulPort;

    UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);
    dlClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000000));
    dlClientHelper.SetAttribute("Interval", TimeValue(MilliSeconds(intervalTime)));
    dlClientHelper.SetAttribute("PacketSize", UintegerValue(maxPacketSize));

    PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory",
                                        InetSocketAddress(Ipv4Address::GetAny(), dlPort));

    ApplicationContainer clientApps = dlClientHelper.Install(remoteHost);
    ApplicationContainer serverApps = dlPacketSinkHelper.Install(ueNodes.Get(u));

    Ptr<EpcTft> tft = Create<EpcTft>();
    EpcTft::PacketFilter dlpf;
    dlpf.localPortStart = dlPort;
    dlpf.localPortEnd = dlPort;
    tft->Add(dlpf);
    EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
    lteHelper->ActivateDedicatedEpsBearer(ueDevs.Get(u), bearer, tft);

    Time startTime = Seconds(startTimeSeconds->GetValue());
    serverApps.Start(startTime);
    clientApps.Start(startTime);
  }

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (generateRem)
  {
    PrintEnbList("output/enbs_lte2.txt");
    PrintUeList("output/ues_lte2.txt");

    remHelper = CreateObject<RadioEnvironmentMapHelper>();
    Ptr<LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get(0)
                                               ->GetObject<LteEnbNetDevice>()
                                               ->GetPhy()
                                               ->GetDownlinkSpectrumPhy()
                                               ->GetObject<LteSpectrumPhy>();
    Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel();
    remHelper->SetAttribute("Channel", PointerValue(dlChannel));
    remHelper->SetAttribute("OutputFile", StringValue("output/lte_assign2.rem"));
    remHelper->SetAttribute("XMin", DoubleValue(macroUeBox.xMin));
    remHelper->SetAttribute("XMax", DoubleValue(macroUeBox.xMax));
    remHelper->SetAttribute("YMin", DoubleValue(macroUeBox.yMin));
    remHelper->SetAttribute("YMax", DoubleValue(macroUeBox.yMax));
    remHelper->SetAttribute("Z", DoubleValue(1.5));
    remHelper->SetAttribute("XRes", UintegerValue(500));
    remHelper->SetAttribute("YRes", UintegerValue(500));
    remHelper->Install();
  }

  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add(remoteHost);
  endpointNodes.Add(ueNodes);
  Ptr<FlowMonitor> monitor = flowmonHelper.Install(endpointNodes);

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  std::ofstream outFile;
  outFile.open("output/throughput.txt", std::ios_base::out | std::ios_base::trunc);
  outFile << "Scheduler: " << scheduler << "  Speed: " << speed << "  RngRun: " << RngRun << "\n";

  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

  double sumThr = 0.0;
  double sumThrSq = 0.0;
  int nThrFlows = 0;
  double sumDelay = 0.0;
  int nDelayFlows = 0;

  std::ofstream cdfFile;
  cdfFile.open("output/per_ue_throughput.txt", std::ios_base::out | std::ios_base::trunc);

  for (auto i = stats.begin(); i != stats.end(); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
    outFile << "\nFlow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> "
            << t.destinationAddress << ":" << t.destinationPort << ")\n";
    outFile << "  Tx Packets: " << i->second.txPackets << "\n";
    outFile << "  Rx Packets: " << i->second.rxPackets << "\n";

    if (i->second.rxPackets > 0)
    {
      double rxDuration = i->second.timeLastRxPacket.GetSeconds() -
                          i->second.timeFirstTxPacket.GetSeconds();
      double thr = i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
      double delay = 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;

      sumThr += thr;
      sumThrSq += thr * thr;
      nThrFlows++;
      sumDelay += delay;
      nDelayFlows++;

      cdfFile << thr << "\n";

      outFile << "  Throughput: " << thr << " Mbps\n";
      outFile << "  Mean delay: " << delay << " ms\n";
    }
    else
    {
      outFile << "  Throughput: 0 Mbps\n";
      outFile << "  Mean delay: 0 ms\n";
    }
  }

  double jfi = 0.0;
  if (nThrFlows > 0 && sumThrSq > 0.0)
  {
    jfi = (sumThr * sumThr) / (nThrFlows * sumThrSq);
  }
  double meanDelay = (nDelayFlows > 0) ? sumDelay / nDelayFlows : 0.0;

  outFile << "\n==== Summary ====\n";
  outFile << "Active flows: " << nThrFlows << "\n";
  outFile << "Aggregate throughput: " << sumThr << " Mbps\n";
  outFile << "Mean per flow throughput: " << (nThrFlows > 0 ? sumThr / nThrFlows : 0.0) << " Mbps\n";
  outFile << "Jains Fairness Index: " << jfi << "\n";
  outFile << "Mean delay: " << meanDelay << " ms\n";

  AppendAggregate("output/data.txt", scheduler, sumThr, speed);

  Simulator::Destroy();
  return 0;
}
