#include <stdint.h>
#include <stdlib.h>

#ifndef __PACKET_DRIVER
#define __PACKET_DRIVER

// Callback - void* pContext, uint8_t* sender_addres, uint8_t* packet 
#define PPACKETRECEIVEDCALLBACK(fn) bool (*fn)(void*, uint8_t*, uint8_t*)
#define PACKETRECEIVEDCALLBACK(fn) bool (fn)(void*, uint8_t*, uint8_t*)
#ifndef PACKET_DRIVER_MAX_ADDRESS_LEN
#define PACKET_DRIVER_MAX_ADDRESS_LEN 0
#endif

#ifndef PACKET_DRIVER_MAX_PAYLOAD
#define PACKET_DRIVER_MAX_PAYLOAD 0
#endif

class IPacketDriver{
public:
  IPacketDriver(){m_callbackfn = NULL; m_callbackcontext = NULL;m_address_len = 0;}
  virtual bool initialise(uint8_t *device, uint8_t *broadcast, uint8_t length) = 0 ;
  virtual bool shutdown() = 0;
  virtual bool send_mode() = 0;
  virtual bool data_received_interrupt() = 0;
  virtual bool send(const uint8_t *receiver, uint8_t *data, uint8_t len) = 0;
  virtual bool set_payload_width(uint8_t width) = 0;
  virtual uint8_t get_payload_width() = 0 ;
  virtual bool listen_mode() = 0;
  virtual void set_callback_context(void * pcontext){m_callbackcontext = pcontext;}
  virtual void set_data_received_callback(PPACKETRECEIVEDCALLBACK(fn)){m_callbackfn = fn ;}
  virtual uint8_t get_address_len() { return m_address_len;}
  virtual uint8_t *get_broadcast() = 0;
  virtual uint8_t *get_address() = 0;
  
protected:
  PPACKETRECEIVEDCALLBACK(m_callbackfn) ;
  void *m_callbackcontext ;
  uint8_t m_address_len ;
  
private:
  
  
};





#endif


