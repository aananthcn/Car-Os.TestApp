/*
 * Created on Sat Jan 28 2023 4:13:15 PM
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 Aananth C N
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <osek.h>
#include <os_api.h>

#include <Spi.h>

#include <Eth.h>
#include <macphy.h>
#include <TcpIp.h>


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ethernet_test_app, LOG_LEVEL_DBG);



// 1 = read_eth_register()
// 2 = read_mac_mii_register()
// 3 = read_phy_register()
// * = TcpIp stack operations
#define ENC28J60_SPI_TEST 10


// test-case:1 enc28j60_write_reg(ERXSTH, HI_BYTE(RX_BUF_BEG));
void read_eth_register(void) {
	static u8 rd_ptr_l;

	// After reset the value @ERDPTL must be == 0xFA
	rd_ptr_l = enc28j60_read_reg(ERDPTL);
	LOG_DBG("ERDPTL: 0x%02x", rd_ptr_l);
}


// test-case:2 enc28j60_write_reg(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
// expected value: 0x00
void read_mac_mii_register(void) {
	static u8 m_reg;

	// If initialized as above, MACON1 should contain 0b0000_1101 = 0x0D, but 0x00 is the Read value
	m_reg = enc28j60_read_reg(MACON1);
	LOG_DBG("MACON1: 0x%02x", m_reg);

	// Max Frame Len. High Byte should be 0x06
	m_reg = enc28j60_read_reg(MAMXFLH);
	LOG_DBG("MAMXFLH: 0x%02x", m_reg);
}


// test-case:3 just read PHID1 and PHID2 and print PHY ID and REV.ID of the device. 
void read_phy_register(void) {
        static uint16 phid1, phid2;
	static uint32 phy_id;
	static uint8 phy_rev;

	phy_id = 0;
	phy_rev = 0;

        phid1 = enc28j60_read_phy(PHID1);
	LOG_DBG("PHID1: 0x%04x", phid1);

        phid2 = enc28j60_read_phy(PHID2);
	LOG_DBG("PHID2: 0x%04x", phid2);

        phy_id = ((phid2 & 0xFC00) >> 10) << 19 /*bits[15:10] --> bits[24:19]*/ |  phid1 << 3 /*bits[18:3]*/;
        LOG_DBG("PHY ID: 0x%08x", phy_id); // expected value 0x280418

        phy_rev = (uint8) phid2 & 0x0F;
        LOG_DBG("PHY RevID: 0x%02x", phy_rev);
}


// Called by Os for every 100 ms
TASK(Ethernet_Tasks) {
#if ENC28J60_SPI_TEST == 1
	read_eth_register();
#elif ENC28J60_SPI_TEST == 2
	read_mac_mii_register();
#elif ENC28J60_SPI_TEST == 3
	read_phy_register();
#else
 #if (ETH_DRIVER_MAX_CHANNEL > 0)
	TcpIp_MainFunction();
	macphy_periodic_fn();
 #endif
#endif
	k_sleep(K_SECONDS(1));
}