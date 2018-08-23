/*
 *  File: usb_bulk_device.h
 *
 * Written by duvallee.lee in 2018
 *
 */
#pragma once

// ----------------------------------------------------------------------------------------
#define MYDRONE_VID                                      0x0483
#define MYDRONE_PID                                      0x0401

#define BULK_IN_EP                                       0x81        // EP1 for data IN
#define BULK_OUT_EP                                      0x02        // EP1 for data OUT

#define MYDRONE_CLASS_GUID                               { 0x77E3B6D1, 0xA991, 0x4C0A, { 0x88, 0xBA, 0xE1, 0xD3, 0x54, 0x8C, 0x2F, 0x41 } }

#define USB_BULK_DEVICE_PACKET_SIZE                      64          // in USB Full-Speed

#define USB_BULK_DEVICE_TIME_OUT                         1000

// ----------------------------------------------------------------------------------------
class MUTEX
{
   DWORD lock_count;
   CRITICAL_SECTION critical_section;

public:
   MUTEX()
   {
      InitializeCriticalSection(&critical_section);
      lock_count = 0;
   };

   ~MUTEX()
   {
      if (lock_count != 0)
      {
         LeaveCriticalSection(&critical_section);
      }
      DeleteCriticalSection(&critical_section);
   };

   void LOCK()
   {
      if (lock_count != 0)
      {
         return;
      }
      lock_count = 1;
      EnterCriticalSection(&critical_section);
   };

   void UNLOCK()
   {
      if (lock_count == 0)
      {
         return;
      }
      LeaveCriticalSection(&critical_section);
      lock_count = 0;
   };
};


class UsbBulkDevice
{
   // Full Path Name of USB Bulk Device
   CString m_szDeviceName;
   // for last errorcode
   DWORD m_dwErrorCode;

   // handle of usb bulk device
   HANDLE m_hUsbBulkDeviceHandle;
   HANDLE m_UsbBulkDeviceInterfaceHandle;

   // for endpoint of USB
   UCHAR m_usb_in_endpoint;
   UCHAR m_usb_out_endpoint;

   // for read & write
   HANDLE m_hEvent_Read_Handle;
   HANDLE m_hEvent_Write_Handle;

   void UsbBulkDeviceClose();

public :
   UsbBulkDevice();
   virtual ~UsbBulkDevice();

   // get error code
   DWORD GetErrorCode()
   {
      return m_dwErrorCode;
   };

   // 
   BOOL FindUsbBulkDevice();
   BOOL ConnectUsbBulkDevice();
   BOOL DisConnectUsbBulkDevice();

   //
   BOOL WriteUsbBulkDevice(unsigned char* pdata, DWORD dwSize);
   BOOL ReadUsbBulkDevice(unsigned char* pdata, DWORD dwSize);
};

extern UsbBulkDevice& gUsbBulkDevice();




