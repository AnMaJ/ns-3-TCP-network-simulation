#include <fstream>
#include <string>
#include <time.h>
#include <iostream>
#include <map>
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/packet-sink.h"
#include "ns3/network-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;
using namespace std;

//defining a log component named wired
NS_LOG_COMPONENT_DEFINE ("wired");

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
		cout<<"//////////////////////// TCP Westwood Wired ////////////////////////"<<endl;
  	}
  	else if(agent == "Veno")
    {
  	  	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
		cout<<"//////////////////////// TCP Veno Wired ////////////////////////"<<endl;
  	}
  	else if(agent == "Vegas")
    {
  	  	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
		cout<<"//////////////////////// TCP Vegas Wired ////////////////////////"<<endl;
  	}
  	else
    {
      	NS_LOG_INFO("Invalid TCP Agent entered. Exiting.");
  	  	exit(0);
    }
	
    NS_LOG_INFO("TCP Agent set to TCP " + agent);
}
void set_plot_parameters(Gnuplot &plot, string agent){
	plot.SetLegend ("Packet Size (in bytes)", "Throughput (in Kbps)");
	plot.SetTitle ("Throughput plot for TCP (Wired)" + agent);
	plot.AppendExtra ("set xrange [20:1520]");
  	plot.SetTerminal ("png");
  	
}
void set_dataset_parameters(Gnuplot2dDataset &dataset, string agent){

	dataset.SetTitle ("TCP " + agent);
  	dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

}

void print_header_in_trace_files(Ptr<OutputStreamWrapper> &fout,AsciiTraceHelper &traceHelper, string agent){
	fout = traceHelper.CreateFileStream ("wiredTcp" + agent + "_trace.txt");
  	*fout->GetStream () << "TCP " + agent + "\n" + "Packet Size     Throughput    Fairness Index\n-----------     ----------    --------------\n";
  	cout << "Packet Size     Throughput    Fairness Index\n-----------     ----------    --------------\n";
}

void set_attribute_for_link(PointToPointHelper &link, int packetSize, string DataRate, string Delay,int a, int b){
	
    int maxQueueSize = (a*b*1000)/(8*packetSize);
    link.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue(to_string(maxQueueSize)+"p")); 
	link.SetDeviceAttribute("DataRate" , StringValue(DataRate) );
    link.SetChannelAttribute("Delay" , StringValue(Delay) );

}

void set_up_ipv4_address_on_node_containers(Ipv4AddressHelper &ipv4_Node2R1,Ipv4AddressHelper &ipv4_R1R2,Ipv4AddressHelper &ipv4_R2Node3){

	ipv4_Node2R1.SetBase( "10.1.1.0" , "255.255.255.0" );
	ipv4_R1R2.SetBase( "10.1.2.0" , "255.255.255.0" );
	ipv4_R2Node3.SetBase( "10.1.3.0" , "255.255.255.0" );

}

void set_server_address(Address &server_addr, Address &any_addr,Ipv4InterfaceContainer &R2Node3Interface){
	int random_number=9000 + (rand()%2000);
	server_addr = InetSocketAddress (R2Node3Interface.GetAddress(1),random_number);
	any_addr = InetSocketAddress(Ipv4Address::GetAny(),random_number);
}
void set_timer(Ptr<ClientApp> &clientApp,int s, int e){
	clientApp->SetStartTime (Seconds (s));
    clientApp->SetStopTime (Seconds (e));
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

	string plotFileName = "wiredTcp" + agent + ".plt";

  	ofstream plotFile (plotFileName.c_str());

  	plot.GenerateOutput (plotFile);

  	plotFile.close ();
}

void add_fairness_index_dataset_and_genetate_output(Gnuplot2dDataset &dataset, string agent,Gnuplot &plot){
	plot.AddDataset (dataset);

	string plotFileName = "wiredTcp" + agent + "_fairness_index.plt";

  	ofstream plotFile (plotFileName.c_str());

  	plot.GenerateOutput (plotFile);

  	plotFile.close ();
}
void write_data_to_file(int packetSize, double throughput, int fairness, Ptr<OutputStreamWrapper> &fout){
	*fout->GetStream () <<  "   " <<to_string(packetSize) << "\t\t" <<to_string(throughput) <<"\t    "<<fairness << "\n";
      	cout <<  "   " <<to_string(packetSize) << "\t\t" << to_string(throughput) <<"\t    "<<fairness << "\n";

}
int
main (int argc, char *argv[])
{

	Time::SetResolution (Time::NS);
	//enabling the log component
	LogComponentEnable("wired" , LOG_INFO);

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
  	Gnuplot plot ("wiredTcp" + agent + ".png");
	set_plot_parameters(plot, agent);
  	
	//making plot for fairness index
	Gnuplot plot_f ("wiredTcp_fairness_index" + agent + ".png");
	Gnuplot2dDataset dataset_fairness;
	dataset_fairness.SetTitle ("TCP " + agent+ "fairness index");
  	dataset_fairness.SetStyle (Gnuplot2dDataset::LINES_POINTS);

	//setting parameters for the dataset
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
      	//setting segment size
      	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));

      	//creating nodes
      	NodeContainer p2pNodes;
      	p2pNodes.Create (4);

      	//creating different types of links
      	PointToPointHelper NodeRouterLink,RouterRouterLink;
		set_attribute_for_link(NodeRouterLink,packetSize,"100Mbps","20ms",100,20);
		set_attribute_for_link(RouterRouterLink,packetSize,"10Mbps","50ms",10,50);
      	
      	//creating net devices by setting up links between nodes 
		NetDeviceContainer Node2R1 = NodeRouterLink.Install( p2pNodes.Get(0) , p2pNodes.Get(1) );
		NetDeviceContainer R1R2 = RouterRouterLink.Install(p2pNodes.Get(1) , p2pNodes.Get(2) );
		NetDeviceContainer R2Node3 = NodeRouterLink.Install(p2pNodes.Get(2) , p2pNodes.Get(3) );
      	
      
      	//setting up internet stack and installing all nodes onto it
      	InternetStackHelper internet;
      	internet.Install(p2pNodes);

      	//setting up ipv4 ip addresses
      	Ipv4AddressHelper ipv4_Node2R1,ipv4_R1R2,ipv4_R2Node3;
		set_up_ipv4_address_on_node_containers(ipv4_Node2R1,ipv4_R1R2,ipv4_R2Node3);
      	

      	Ipv4InterfaceContainer Node2R1Interface,R1R2Interface,R2Node3Interface;  
		R1R2Interface = ipv4_R1R2.Assign ( R1R2 );
		R2Node3Interface = ipv4_R2Node3.Assign ( R2Node3 );
		Node2R1Interface = ipv4_Node2R1.Assign ( Node2R1 );
		

      	
      	//setting up server address
      	Address server_addr,any_addr;
		set_server_address(server_addr,any_addr,R2Node3Interface);

      	//populating routing tables
      	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      	//creating application container for sink
      	ApplicationContainer Sink;

      	//setting up packet helper for sink
      	PacketSinkHelper sink ("ns3::TcpSocketFactory", any_addr);
      	Sink = (sink.Install(p2pNodes.Get(3)));

      	//Create a client socket
      	Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (p2pNodes.Get (0), TcpSocketFactory::GetTypeId ());

      	//Create an application
      	Ptr<ClientApp> clientApp = CreateObject<ClientApp> ();

      	//Setup the application
      	clientApp->Setup (ns3TcpSocket, server_addr, packetSize, 100000, DataRate ("20Mbps"));

      	//Add the application onto the client
      	p2pNodes.Get(0)->AddApplication (clientApp);


      	//Set the start and stop times for the client
		set_timer(clientApp,1,20);
      	
       
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
	add_dataset_and_generate_output(dataset,agent, plot);

	//add fairness index dataset and genetate output
	add_fairness_index_dataset_and_genetate_output(dataset_fairness,agent,plot_f);
  	
  	return 0;  
}
