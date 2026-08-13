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

#include <fstream>
#include <sstream>
#include <map>
#include <utility>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Asg1PartB");

static NodeContainer g_ueNodes;
static NetDeviceContainer g_ueDevs;
static std::map<uint16_t, uint8_t> g_ueMcs;
static std::map<std::pair<uint64_t, uint32_t>, int> g_pred;
static std::ofstream g_dumpFile;
static std::string g_studentTag = "Student_2111EC015";

void DlSchedTrace(DlSchedulingCallbackInfo info)
{
  g_ueMcs[info.rnti] = info.mcsTb1;
}

int GetUeMcs(uint32_t ueIndex)
{
  Ptr<LteUeNetDevice> dev = g_ueDevs.Get(ueIndex)->GetObject<LteUeNetDevice>();
  uint16_t rnti = dev->GetRrc()->GetRnti();
  auto it = g_ueMcs.find(rnti);
  if (it == g_ueMcs.end())
  {
    return -1;
  }
  return static_cast<int>(it->second);
}

void SetMCS()
{
  double now = Simulator::Now().GetSeconds();
  for (uint32_t i = 0; i < g_ueNodes.GetN(); ++i)
  {
    Vector p = g_ueNodes.Get(i)->GetObject<MobilityModel>()->GetPosition();
    int mcs = GetUeMcs(i);
    if (mcs >= 0 && g_dumpFile.is_open())
    {
      g_dumpFile << now << "," << i << "," << p.x << "," << p.y << "," << mcs << "\n";
    }
  }
  Simulator::Schedule(Seconds(0.2), &SetMCS);
}

void GetMCS()
{
  uint64_t tms = Simulator::Now().GetMilliSeconds();
  for (uint32_t i = 0; i < g_ueNodes.GetN(); ++i)
  {
    int orig = GetUeMcs(i);
    auto pit = g_pred.find(std::make_pair(tms, i));
    if (orig >= 0 && pit != g_pred.end())
    {
      NS_LOG_INFO(g_studentTag << " original MCS value is " << orig);
      NS_LOG_INFO(g_studentTag << " Predicted MCS value is " << pit->second);
    }
  }
  Simulator::Schedule(Seconds(0.2), &GetMCS);
}

void LoadPredictions(std::string path)
{
  std::ifstream in(path.c_str());
  if (!in.is_open())
  {
    NS_LOG_ERROR("Cannot open predictions file " << path);
    return;
  }
  std::string line;
  std::getline(in, line);
  while (std::getline(in, line))
  {
    std::stringstream ss(line);
    std::string a, b, c;
    std::getline(ss, a, ',');
    std::getline(ss, b, ',');
    std::getline(ss, c, ',');
    if (a.empty() || b.empty() || c.empty())
    {
      continue;
    }
    uint64_t tms = static_cast<uint64_t>(std::stoll(a));
    uint32_t ue = static_cast<uint32_t>(std::stoul(b));
    int mcs = static_cast<int>(std::stod(c));
    g_pred[std::make_pair(tms, ue)] = mcs;
  }
}

int main(int argc, char *argv[])
{
  uint16_t numberOfUes = 10;
  uint16_t numberOfEnbs = 1;
  double simTime = 500.0;
  double radius = 500.0;
  double speed = 10.0;
  double packetSize = 1500;
  double intervalMs = 1.0;
  std::string schedulerType = "PF";
  std::string phase = "dump";
  std::string dumpFile = "mcs_dataset.csv";
  std::string predFile = "predicted_mcs.csv";
  std::string studentTag = "Student_0000000";
  uint16_t RngRun = 15;

  CommandLine cmd(__FILE__);
  cmd.AddValue("speed", "UE speed in m/s", speed);
  cmd.AddValue("simTime", "Simulation duration in seconds", simTime);
  cmd.AddValue("rngRun", "RngRun value", RngRun);
  cmd.AddValue("schedulerType", "PF, RR, MT or PSS", schedulerType);
  cmd.AddValue("phase", "dump or log", phase);
  cmd.AddValue("dumpFile", "CSV path for dataset or coords when phase is dump", dumpFile);
  cmd.AddValue("predFile", "CSV path of predictions when phase is log", predFile);
  cmd.AddValue("studentTag", "Name_Enrollment tag for log lines", studentTag);
  cmd.Parse(argc, argv);

  g_studentTag = studentTag;

  RngSeedManager::SetSeed(5);
  RngSeedManager::SetRun(RngRun);

  std::string scheduler = "PfFfMacScheduler";
  if (schedulerType == "RR")
  {
    scheduler = "RrFfMacScheduler";
  }
  else if (schedulerType == "MT")
  {
    scheduler = "TdMtFfMacScheduler";
  }
  else if (schedulerType == "PSS")
  {
    scheduler = "PssFfMacScheduler";
  }

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

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

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
      ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(40.0));

  NodeContainer enbNodes;
  enbNodes.Create(numberOfEnbs);
  g_ueNodes.Create(numberOfUes);

  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
  enbPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator(enbPositionAlloc);
  enbMobility.Install(enbNodes);

  MobilityHelper ueMobility;
  ueMobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "X", DoubleValue(0.0), "Y",
                                  DoubleValue(0.0), "rho", DoubleValue(radius));
  ueMobility.SetMobilityModel(
      "ns3::RandomWalk2dMobilityModel", "Bounds",
      RectangleValue(Rectangle(-2500, 2500, -2500, 2500)), "Time", StringValue("1s"), "Mode",
      StringValue("Time"), "Speed",
      StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(speed) + "]"));
  ueMobility.Install(g_ueNodes);

  NetDeviceContainer enbDevs;
  lteHelper->SetSchedulerType("ns3::" + scheduler);
  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(50));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(50));
  enbDevs = lteHelper->InstallEnbDevice(enbNodes);
  g_ueDevs = lteHelper->InstallUeDevice(g_ueNodes);

  internet.Install(g_ueNodes);
  Ipv4InterfaceContainer ueIpIfaces =
      epcHelper->AssignUeIpv4Address(NetDeviceContainer(g_ueDevs));
  lteHelper->AttachToClosestEnb(g_ueDevs, enbDevs);

  uint16_t dlPort = 10000;
  Ptr<UniformRandomVariable> startTime = CreateObject<UniformRandomVariable>();
  startTime->SetAttribute("Min", DoubleValue(0));
  startTime->SetAttribute("Max", DoubleValue(0.010));

  for (uint32_t u = 0; u < g_ueNodes.GetN(); ++u)
  {
    Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(g_ueNodes.Get(u)->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

    ++dlPort;
    UdpClientHelper dlClient(ueIpIfaces.GetAddress(u), dlPort);
    dlClient.SetAttribute("MaxPackets", UintegerValue(1000000000));
    dlClient.SetAttribute("Interval", TimeValue(MilliSeconds(intervalMs)));
    dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));

    PacketSinkHelper dlSink("ns3::UdpSocketFactory",
                            InetSocketAddress(Ipv4Address::GetAny(), dlPort));

    ApplicationContainer clientApp = dlClient.Install(remoteHost);
    ApplicationContainer serverApp = dlSink.Install(g_ueNodes.Get(u));

    Ptr<EpcTft> tft = Create<EpcTft>();
    EpcTft::PacketFilter dlpf;
    dlpf.localPortStart = dlPort;
    dlpf.localPortEnd = dlPort;
    tft->Add(dlpf);
    EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
    lteHelper->ActivateDedicatedEpsBearer(g_ueDevs.Get(u), bearer, tft);

    Time t = Seconds(startTime->GetValue());
    clientApp.Start(t);
    serverApp.Start(t);
  }

  for (uint32_t k = 0; k < enbDevs.GetN(); ++k)
  {
    Ptr<LteEnbNetDevice> enbDev = enbDevs.Get(k)->GetObject<LteEnbNetDevice>();
    enbDev->GetMac()->TraceConnectWithoutContext("DlScheduling", MakeCallback(&DlSchedTrace));
  }

  if (phase == "dump")
  {
    g_dumpFile.open(dumpFile.c_str(), std::ios_base::out | std::ios_base::trunc);
    g_dumpFile << "time,ue,x,y,mcs\n";
    Simulator::Schedule(Seconds(0.2), &SetMCS);
  }
  else if (phase == "log")
  {
    LoadPredictions(predFile);
    Simulator::Schedule(Seconds(0.2), &GetMCS);
  }

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  if (g_dumpFile.is_open())
  {
    g_dumpFile.close();
  }

  Simulator::Destroy();
  return 0;
}
