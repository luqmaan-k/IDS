extern crate send_packet;
extern crate pnet;
extern crate pnet_datalink;
extern crate text_io;

use send_packet::payload::PayloadData;
use send_packet::*;
use pnet::packet::Packet;
use pnet::util::MacAddr;
use pnet_datalink::Channel::Ethernet;
use pnet_datalink::NetworkInterface;
use std::env;
use text_io::read;

fn main() {
    //trace_macros!(true);
    let if_name = env::args()
        .nth(1)
        .expect("Usage: ./sendpacket <interface name>");

    let interfaces = pnet_datalink::interfaces();
    let interface = interfaces
        .into_iter()
        .filter(|iface: &NetworkInterface| iface.name == if_name)
        .next()
        .unwrap_or_else(|| panic!("No such network interface: {}", if_name));

    let (mut sender, mut _receiver) = match pnet_datalink::channel(&interface, Default::default()) {
        Ok(Ethernet(tx, rx)) => (tx, rx),
        Ok(_) => panic!("packetdump: unhandled channel type"),
        Err(e) => panic!("packetdump: unable to create channel: {}", e),
    };

    print!("Enter Source Mac Adress : ");
    let mac_addr_source: String = read!();
    print!("Enter Destination Mac Adress : ");
    let mac_addr_destination: String = read!();
    let mac_addr_source: Vec<u8> = mac_addr_source
        .split(':')
        .map(|x| x.parse().expect("Not a valid address!"))
        .collect();
    let mac_addr_destination: Vec<u8> = mac_addr_destination
        .split(':')
        .map(|x| x.parse().expect("Not a valid address!"))
        .collect();

    print!("Enter Source ipv4: ");
    let ipv4_source: String = read!();
    print!("Enter Destination ipv4 : ");
    let ipv4_destination: String = read!();

    loop {
        // Generate a UDP packet with data
        let mut pkt_buf = [0u8; 1500];
        let pkt = send_packet!(
            pkt_buf,
            ether({set_destination => MacAddr(mac_addr_destination[0],mac_addr_destination[1],mac_addr_destination[2],mac_addr_destination[3],mac_addr_destination[4],mac_addr_destination[5]),
                set_source => MacAddr(mac_addr_destination[0],mac_addr_destination[1],mac_addr_destination[2],mac_addr_destination[3],mac_addr_destination[4],mac_addr_destination[5])}) /
                ipv4({set_source => ipv4addr!(ipv4_source),
                set_destination => ipv4addr!(ipv4_destination) }) /
                udp({set_source => 12312, set_destination => 143}) /
                payload({"hello world!".to_string().into_bytes()})
        );
        sender.send_to(pkt.packet(), None).unwrap().unwrap();
    }
}
