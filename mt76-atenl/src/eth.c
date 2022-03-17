#include <sys/ioctl.h>
#include <sys/socket.h>

#include "atenl.h"

int atenl_eth_init(struct atenl *an)
{
	struct sockaddr_ll addr = {};
	struct ifreq ifr = {};
	int ret;
 
	memcpy(ifr.ifr_name, BRIDGE_NAME, strlen(BRIDGE_NAME));
	ret = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RACFG));
	if (ret < 0) {
		perror("socket");
		goto out;
	}
	an->sock_eth = ret;

	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = if_nametoindex(BRIDGE_NAME);

	ret = bind(an->sock_eth, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		perror("bind");
		goto out;
	}

	ret = ioctl(an->sock_eth, SIOCGIFHWADDR, &ifr);
	if (ret < 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		goto out;
	}

	memcpy(an->mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	atenl_info("Open Ethernet socket success on %s, mac addr = " MACSTR "\n",
		   BRIDGE_NAME, MAC2STR(an->mac_addr));

	ret = 0;
out:
	return ret;
}

int atenl_eth_recv(struct atenl *an, struct atenl_data *data)
{
	char buf[RACFG_PKT_MAX_SIZE];
	int len = recvfrom(an->sock_eth, buf, sizeof(buf), 0, NULL, NULL);
	struct ethhdr *hdr = (struct ethhdr *)buf;

	atenl_dbg("[%d]%s: recv len = %d\n", getpid(), __func__, len);

	if (len >= ETH_HLEN + RACFG_HLEN) {
		if (hdr->h_proto == htons(ETH_P_RACFG) &&
		    (ether_addr_equal(an->mac_addr, hdr->h_dest) ||
		     is_broadcast_ether_addr(hdr->h_dest))) {
			data->len = len;
			memcpy(data->buf, buf, len);

			return 0;
		}
	}

	atenl_err("%s: packet len is too short\n", __func__);
	return -EINVAL;
}

int atenl_eth_send(struct atenl *an, struct atenl_data *data)
{
	struct ethhdr *ehdr = (struct ethhdr *)data->buf;
	struct sockaddr_ll addr = {};
	int ret, len = data->len;

	if (an->unicast)
		ether_addr_copy(ehdr->h_dest, ehdr->h_source);
	else
		eth_broadcast_addr(ehdr->h_dest);

	ether_addr_copy(ehdr->h_source, an->mac_addr);
	ehdr->h_proto = htons(ETH_P_RACFG);

	if (len < 60)
		len = 60;
	else if (len > 1514) {
		atenl_err("%s: response ethernet length is too long\n", __func__);
		return -1;
	}

	atenl_dbg_print_data(data, __func__, len);

	addr.sll_family = PF_PACKET;
	addr.sll_protocol = htons(ETH_P_RACFG);
	addr.sll_ifindex = if_nametoindex(BRIDGE_NAME);
	addr.sll_pkttype = PACKET_BROADCAST;
	addr.sll_hatype = ARPHRD_ETHER;
	addr.sll_halen = ETH_ALEN;
	memset(addr.sll_addr, 0, 8);
	eth_broadcast_addr(addr.sll_addr);

	ret = sendto(an->sock_eth, data->buf, len, 0,
		     (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		perror("sendto");
		return ret;
	}
	
	atenl_dbg("[%d]%s: send length = %d\n", getpid(), __func__, len);

	return 0;
}
