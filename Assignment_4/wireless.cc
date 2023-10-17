#include <fstream>
#include <string>
#include <time.h>
#include <map>
#include <iostream>
#include "ns3/enum.h"
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/packet-sink.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;
using namespace std;

//defining a log component named wireless
NS_LOG_COMPONENT_DEFINE ("wireless");

//Class for client application, Taken from seven.cc of the exmaple tutorial of ns-3.
class ClientApp : public Application
{
public:
  ClientApp (); //Constructor
  virtual ~ClientApp (); //Deconstructor

  //Initialize the object parameters
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;	//Socket
  Address         m_peer; //Address of receiver
  uint32_t        m_packetSize;	//Packet Size
  uint32_t        m_nPackets;	//Total number of packets to be sent
  DataRate        m_dataRate;	//Data rate
  EventId         m_sendEvent;
  bool            m_running;	//State of runnning
  uint32_t        m_packetsSent; //Number of packets sent
};

//Constructor provides initial value to all the variables.
ClientApp::ClientApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

ClientApp::~ClientApp ()
{
  m_socket = 0;
}

//Setup initializes all the parameters.
void
ClientApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

//Start the application
void
ClientApp::StartApplication (void)
{
  m_running = true; //Set the running state to true
  m_packetsSent = 0; //Set the number of packets sent to 0
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
    //Connect to the peer after binding
  m_socket->Connect (m_peer);
  //Send the packets
  SendPacket ();
}

//Stop Application function
void
ClientApp::StopApplication (void)
{
  m_running = false; //Set running state to false

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close (); //close the socket
    }
}

//Funciton to send the packet
void
ClientApp::SendPacket (void)
{
	//Create a new packet of given packet size and send it
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  //In case the last packet is also sent, schedule for closure.
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
ClientApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &ClientApp::SendPacket, this);
    }
}


void set_configuration(string agent){
	if(agent == "Westwood")
    {
    	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpWestwood"));
		cout<<"//////////////////////// TCP Westwood Wireless ////////////////////////"<<endl;
  	}
  	else if(agent == "Veno")
    {
  	  	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
		cout<<"//////////////////////// TCP Veno Wireless ////////////////////////"<<endl;
  	}
  	else if(agent == "Vegas")
    {
  	  	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
		cout<<"//////////////////////// TCP Vegas Wireless ////////////////////////"<<endl;
  	}
  	else
    {
      	NS_LOG_INFO("Invalid TCP Agent entered. Exiting.");
  	  	exit(0);
    }
	
    NS_LOG_INFO("TCP Agent set to TCP " + agent);
}
void print_header_in_trace_files(Ptr<OutputStreamWrapper> &fout,AsciiTraceHelper &traceHelper, string agent){
	fout = traceHelper.CreateFileStream ("wiredTcp" + agent + "_trace.txt");
  	*fout->GetStream () << "TCP " + agent + "\n" + "Packet Size     Throughput    Fairness Index\n-----------     ----------    --------------\n";
  	cout << "Packet Size     Throughput    Fairness Index\n-----------     ----------    --------------\n";
}
void set_plot_parameters(Gnuplot &plot, string agent){
	plot.SetLegend ("Packet Size (in bytes)", "Throughput (in Kbps)");
  	plot.AppendExtra ("set xrange [20:1520]");
	plot.SetTitle ("Throughput plot for TCP (Wireless)" + agent);
  	plot.SetTerminal ("png"); 	
}

void set_dataset_parameters(Gnuplot2dDataset &dataset, string agent){

	dataset.SetTitle ("TCP " + agent);
  	dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

}

void configure_wifi_parmeters(){
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("999999"));
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("999999"));
	Config::SetDefault("ns3::WifiMacQueue::DropPolicy", EnumValue(WifiMacQueue::DROP_NEWEST));
}

void set_attribute_for_link(PointToPointHelper &link, int packetSize, string DataRate, string Delay,int a, int b){
	
    int maxQueueSize = (a*b*1000)/(8*packetSize);
    link.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue(to_string(maxQueueSize)+"p")); 
	link.SetDeviceAttribute("DataRate" , StringValue(DataRate) );
    link.SetChannelAttribute("Delay" , StringValue(Delay) );

}

void set_location_of_devices(Ptr<ListPositionAllocator> &locationVector){
	locationVector -> Add(Vector(20.0, 20.0, 0.0));
	locationVector -> Add(Vector(0.0, 0.0, 0.0));
	locationVector -> Add(Vector(60.0, 60.0, 0.0));
	locationVector -> Add(Vector(40.0, 40.0, 0.0));	
}


void set_mobility(MobilityHelper &mobility,Ptr<ListPositionAllocator> &locationVector){
	mobility.SetPositionAllocator (locationVector);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

}

void install_nodes(MobilityHelper &mobility,NodeContainer &nodes){
	mobility.Install(nodes.Get(1));
	mobility.Install(nodes.Get(0));
    mobility.Install(nodes.Get(3));
    mobility.Install(nodes.Get(2));
}

void set_up_ipv4_address_on_node_containers(Ipv4InterfaceContainer &InterfaceLeft,Ipv4InterfaceContainer &InterfaceMiddle,Ipv4InterfaceContainer &InterfaceRight,Ipv4AddressHelper &addressHelper,NetDeviceContainer &pathLeft,NetDeviceContainer &pathRight,NetDeviceContainer &bsDevices){
	addressHelper.SetBase("10.1.1.0", "255.255.255.0");
	InterfaceLeft = addressHelper.Assign(pathLeft);
	    
	addressHelper.SetBase("10.1.2.0", "255.255.255.0");
	InterfaceMiddle = addressHelper.Assign(bsDevices);

	addressHelper.SetBase("10.1.3.0", "255.255.255.0");
	InterfaceRight = addressHelper.Assign(pathRight);

}

void set_sink_address(){

}

void write_data_to_file(int packetSize, double throughput, int fairness, Ptr<OutputStreamWrapper> &fout){
	*fout->GetStream () <<  "   " <<to_string(packetSize) << "\t\t" <<to_string(throughput) <<"\t    "<<fairness << "\n";
      	cout <<  "   " <<to_string(packetSize) << "\t\t" << to_string(throughput) <<"\t    "<<fairness << "\n";

}

void setup_application(Ptr<ClientApp> &source,Ptr<Socket> &tcpSocket,NodeContainer &nodes, Address &sink_addr, int packetSize){
	// Send a large number of packets to attain steady state
    	source -> Setup(tcpSocket, sink_addr, packetSize, 100000, DataRate("100Mbps"));
    	nodes.Get(0) -> AddApplication(source);

    	

}

void setup_timer(int s, int e,Ptr<ClientApp> &source){
	//setting start and stop times
    	source -> SetStartTime(Seconds(0.0));
    	source -> SetStopTime(Seconds(20.0));
}

double run_simulator_and_return_throughput(){
		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll();
		
		//Start the simulation
		Simulator::Stop (Seconds (20));
		Simulator::Run ();

		// Get all flow statistics
    	map < FlowId, FlowMonitor::FlowStats > flowStats = monitor -> GetFlowStats();

    	auto iter = flowStats.begin();
    	double throughput =  (8.0 * iter->second.rxBytes) / (1000*(iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds()));
		
		//end simulation
		Simulator::Destroy ();

		return throughput;

}

void add_dataset_and_generate_output(Gnuplot2dDataset &dataset, string agent,Gnuplot &plot){
	plot.AddDataset (dataset);

	string plotFileName = "wirelessTcp" + agent + ".plt";

  	ofstream plotFile (plotFileName.c_str());

  	plot.GenerateOutput (plotFile);

  	plotFile.close ();
}

void add_fairness_index_dataset_and_genetate_output(Gnuplot2dDataset &dataset, string agent,Gnuplot &plot){
	plot.AddDataset (dataset);

	string plotFileName = "wirelessTcp" + agent + "_fairness_index.plt";

  	ofstream plotFile (plotFileName.c_str());

  	plot.GenerateOutput (plotFile);

  	plotFile.close ();
}

int
main(int argc, char** argv)
{

  	Time::SetResolution (Time::NS);
  	//enabling the log component
  	LogComponentEnable("wireless" , LOG_INFO);

  	//string variable which stores the TCP agent
  	string agent;

  	//Taking the TCP agent via a command line parameter
  	CommandLine cmd;
  	cmd.AddValue("agent" , "Specifies the TCP Agent to be used, available options are Westwood,Veno and Vegas" , agent);
  	cmd.Parse (argc, argv);
  	
  	//setting TCP agent entered by user and checking if invalid input given
  	set_configuration(agent);


    //setting seed of rand() using system time
    srand(time(NULL));

	//setting parameters for the plot
  	Gnuplot plot ("wirelessTcp" + agent + ".png");
	set_plot_parameters(plot,agent);

	//making plot for fairness index
	Gnuplot plot_f ("wirelessTcp_fairness_index" + agent + ".png");
	Gnuplot2dDataset dataset_fairness;
	dataset_fairness.SetTitle ("TCP " + agent+ "fairness index");
  	dataset_fairness.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  	Gnuplot2dDataset dataset;
  	set_dataset_parameters(dataset, agent);
	
  	//trace helper for creating trace files
  	AsciiTraceHelper traceHelper;

  	//setting output stream which will write into file
 	 Ptr<OutputStreamWrapper> fout;
	 print_header_in_trace_files(fout,traceHelper,agent);


  	//setting packet size values as given in question
  	int size[10] = {40, 44, 48, 52, 60, 552, 576, 628, 1420, 1500};
	int i=0;

  	while(i<10)
    {
      	int packetSize=size[i];
	    // Configure Default Wifi Parameters
		configure_wifi_parmeters();

    	//creating nodes
    	NodeContainer nodes;
    	nodes.Create(4);
    
    	//setting up Wired link

    	//setting up p2p link
    	PointToPointHelper p2pForBS;
		set_attribute_for_link(p2pForBS,packetSize,"10Mbps","100ms",10,100);
    	
    	//creating net devices by setting up links between nodes 
    	NetDeviceContainer bsDevices = p2pForBS.Install(nodes.Get(1), nodes.Get(2));

    	//setting up Wireless link
    	YansWifiChannelHelper channelHelperFirst,channelHelperSecond;
    	Ptr<YansWifiChannel> channelFirst,channelSecond;
    
		channelHelperSecond = YansWifiChannelHelper::Default();
		channelHelperFirst = YansWifiChannelHelper::Default();
		channelFirst = channelHelperFirst.Create();
		channelSecond = channelHelperSecond.Create();
    	
    	//creating physical helpers
		YansWifiPhyHelper phyHelperFirst, phyHelperSecond;
      	phyHelperFirst.SetErrorRateModel ("ns3::NistErrorRateModel");
      	phyHelperSecond.SetErrorRateModel ("ns3::NistErrorRateModel");

    	//creating wifi helper
    	WifiHelper wifi;
    	WifiMacHelper mac;

    	//setting access point configuration
    	mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(Ssid("ns3-wifi")));

    	phyHelperFirst.SetChannel(channelFirst);
    	phyHelperSecond.SetChannel(channelSecond);

    	// This is the device on the side of BS0 from N0
    	NetDeviceContainer accessPointFirst,accessPointSecond;
		accessPointSecond = wifi.Install (phyHelperSecond, mac, nodes.Get(2));
		accessPointFirst = wifi.Install (phyHelperFirst, mac, nodes.Get(1));
		
    	//Station configuration for N0/N1
    	mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue(Ssid("ns3-wifi")));

    	// This is the device on the side of N0 to BS0
    	NetDeviceContainer endDeviceFirst,endDeviceSecond; 
		endDeviceFirst = wifi.Install(phyHelperFirst, mac, nodes.Get(0));
		endDeviceSecond = wifi.Install(phyHelperSecond, mac, nodes.Get(3));
    
    	// Set constant positions for all the devices
    	// AnimationInterface anim ("animation.xml");
    	MobilityHelper mobility;

    	Ptr<ListPositionAllocator> locationVector = CreateObject<ListPositionAllocator> ();

		//set location of the 4 devices in the network
		set_location_of_devices(locationVector);
	    
		//set mobility 
		set_mobility(mobility,locationVector);
    	
		//install nodes to the mobility model
		install_nodes(mobility,nodes);
    	
    	//setting up internet stack and installing all nodes onto it
    	InternetStackHelper stackHelper;
    	stackHelper.Install(nodes); 

    	//setting up ipv4 ip addresses
    	Ipv4AddressHelper addressHelper;

    	NetDeviceContainer pathLeft(endDeviceFirst, accessPointFirst),pathRight(accessPointSecond, endDeviceSecond);

		Ipv4InterfaceContainer InterfaceLeft,InterfaceMiddle,InterfaceRight;

		set_up_ipv4_address_on_node_containers(InterfaceLeft,InterfaceMiddle,InterfaceRight,addressHelper, pathLeft, pathRight,bsDevices);

    	//populating routing tables
    	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    	//choosing a random port number using rand()
    	uint16_t port_num = 8888 + (rand()%2000);
 
    	//setting up sink address

    	Address sink_addr;

		sink_addr = InetSocketAddress(InterfaceRight.GetAddress(1), port_num);


    	//setting up packet helper for sink
    	PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port_num));
    	ApplicationContainer Sink;
		Sink = sink.Install(nodes.Get(3));

    	// Set timeframe for sink Application
    	Sink.Start(Seconds(0.0));
    	Sink.Stop(Seconds(20.0));


    	// Create a TCP based client that works on the Sender side.
    	Ptr<Socket> tcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
    	Ptr<ClientApp> source = CreateObject<ClientApp>();

		//setup application
		setup_application(source, tcpSocket, nodes, sink_addr, packetSize);
    	
		//setup timer
		setup_timer(0,20,source);

    	
		//Start the simulation
		double throughput=run_simulator_and_return_throughput();
		double fairness=1;

		//adding values to dataset
      	dataset.Add (packetSize, throughput);
		dataset_fairness.Add(packetSize,fairness);

		//writing out to file
      	write_data_to_file(packetSize, throughput, fairness, fout);

      	//move to next packet size
		++i;

    }
    //adding dataset and generating output file
  	add_dataset_and_generate_output(dataset, agent, plot);

	//add fairness index dataset and genetate output
	add_fairness_index_dataset_and_genetate_output(dataset_fairness,agent,plot_f);

  	return 0;  
}