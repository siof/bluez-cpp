/* 
 * File:   BluezBluetooth.cpp
 * Author: tomko
 * 
 * Created on 26 marzec 2012, 23:21
 */

#include "BluezBluetooth.h"
#include "equalMAC.h"
#include "equalName.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <algorithm>

BluezBluetooth::BluezBluetooth()
{
}

BluezBluetooth::BluezBluetooth(const BluezBluetooth& orig)
{
}

BluezBluetooth::~BluezBluetooth()
{
}

std::list<Device> BluezBluetooth::getDevices()
{
  return devices;
}

void BluezBluetooth::scanDevices() throw (BluezBluetooth::BluetoothError)
{
  scan();
}

void BluezBluetooth::scan() throw (BluezBluetooth::BluetoothError)
{
  devices.clear();
  inquiry_info *ii = NULL;
  int max_rsp, num_rsp;
  int dev_id, sock, len, flags;
  int i;
  char addr[19] = {0};
  char name[248] = {0};

  dev_id = hci_get_route(NULL);
  sock = hci_open_dev(dev_id);
  if (dev_id < 0 || sock < 0)
  {
    throw BluetoothError("Cannot open socket");
  }

  len = 8;
  max_rsp = 255;
  flags = IREQ_CACHE_FLUSH;
  ii = (inquiry_info*) malloc(max_rsp * sizeof (inquiry_info));

  num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
  if (num_rsp < 0) throw BluetoothError("hci_inquiry");
  for (i = 0; i < num_rsp; i++)
  {
    ba2str(&(ii + i)->bdaddr, addr);
    memset(name, 0, sizeof (name));
    if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof (name), name, 0) < 0)
    {
      strcpy(name, "[unknown]");
    }
    devices.push_back(Device(addr, name));
  }

  free(ii);
  close(sock);
}

Device BluezBluetooth::findByMAC(std::string MAC) throw (BluezBluetooth::NotFound)
{
  std::list<Device>::iterator found = find_if(devices.begin(), devices.end(), EqalMAC(MAC));
  if (found == devices.end())
  {
    throw BluezBluetooth::NotFound();
  }
  return *found;
}

Device BluezBluetooth::findByName(std::string name) throw (BluezBluetooth::NotFound)
{
  std::list<Device>::iterator found = find_if(devices.begin(), devices.end(), EqalName(name));
  if (found == devices.end())
  {
    throw BluezBluetooth::NotFound();
  }
  return *found;
}
