/*
 *  File: usb_bulk_device.h
 *
 * Written by duvallee.lee in 2018
 *
 */

#include "stdafx.h"
#include <winioctl.h>
#include <Setupapi.h>
#include <usbspec.h>
#include <usbdi.h>
#include <usb100.h>
#include <winusb.h>
#include <Strsafe.h>
#include "usb_bulk_device.h"

/* --------------------------------------------------------------------------
 * Name : gUsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
UsbBulkDevice& gUsbBulkDevice()
{
   static UsbBulkDevice sUsbBulkDevice;
   return sUsbBulkDevice;
};


/* --------------------------------------------------------------------------
 * Name : UsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
UsbBulkDevice::UsbBulkDevice()
{
   m_szDeviceName.Empty();
   m_hUsbBulkDeviceHandle                                = INVALID_HANDLE_VALUE;
   m_UsbBulkDeviceInterfaceHandle                        = INVALID_HANDLE_VALUE;
   m_dwErrorCode                                         = 0;

   m_hEvent_Read_Handle                                  = CreateEvent(NULL, FALSE, FALSE, NULL);
   m_hEvent_Write_Handle                                 = CreateEvent(NULL, FALSE, FALSE, NULL);;
}

/* --------------------------------------------------------------------------
 * Name : ~UsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
UsbBulkDevice::~UsbBulkDevice()
{
   UsbBulkDeviceClose();

   if (m_hEvent_Read_Handle != NULL)
   {
      CloseHandle(m_hEvent_Read_Handle);
   }
   if (m_hEvent_Write_Handle != NULL)
   {
      CloseHandle(m_hEvent_Write_Handle);
   }
}


/* --------------------------------------------------------------------------
 * Name : UsbBulkDeviceClose()
 *
 *
 * -------------------------------------------------------------------------- */
void UsbBulkDevice::UsbBulkDeviceClose()
{
   if (m_hUsbBulkDeviceHandle != INVALID_HANDLE_VALUE)
   {
      CloseHandle(m_hUsbBulkDeviceHandle);
      m_hUsbBulkDeviceHandle                             = INVALID_HANDLE_VALUE;
   }

   if (m_UsbBulkDeviceInterfaceHandle != INVALID_HANDLE_VALUE)
   {
      WinUsb_Free(m_UsbBulkDeviceInterfaceHandle);
      m_UsbBulkDeviceInterfaceHandle                     = INVALID_HANDLE_VALUE;
   }
}


/* --------------------------------------------------------------------------
 * Name : ReadUsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
BOOL UsbBulkDevice::FindUsbBulkDevice()
{
   DWORD flags                                           = DIGCF_DEVICEINTERFACE | DIGCF_PRESENT;
   GUID usb_class_guid                                   = MYDRONE_CLASS_GUID;
   HMODULE hModuleSetup;
   HDEVINFO hDevInfo;
   SP_DEVICE_INTERFACE_DATA interface_data;

   CString dev_full_name;
   CString dev_vid_pid_name;

   m_szDeviceName.Empty();

   dev_vid_pid_name.Format(_T("vid_%04x&pid_%04x"), MYDRONE_VID, MYDRONE_PID);
   dev_vid_pid_name.MakeLower();

   m_szDeviceName.Empty();
   hModuleSetup                                          = LoadLibrary(_T("SETUPAPI.dll"));
   if (hModuleSetup == NULL)
   {
      m_dwErrorCode                                      = GetLastError();
      return FALSE;
   }

   hDevInfo                                              = SetupDiGetClassDevs(&usb_class_guid, NULL, NULL, flags);

   if (hDevInfo == INVALID_HANDLE_VALUE)
   {
      m_dwErrorCode                                      = GetLastError();
      return FALSE;
   }

   interface_data.cbSize                                 = sizeof(SP_DEVICE_INTERFACE_DATA);
   for (DWORD dwIndex = 0; SetupDiEnumDeviceInterfaces(hDevInfo, 0, &usb_class_guid, dwIndex, &interface_data); dwIndex++)
   {
      if (((interface_data.Flags & SPINT_REMOVED) == 0) && (interface_data.Flags & SPINT_ACTIVE))
      {
         ULONG required_len = 0;
         SetupDiGetDeviceInterfaceDetail(hDevInfo, &interface_data, NULL, 0, &required_len, NULL);

         if (required_len == 0)
         {
            continue;
         }

         PSP_DEVICE_INTERFACE_DETAIL_DATA p_device_interface_detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new char[required_len];
         p_device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
         if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &interface_data, p_device_interface_detail_data, required_len, &required_len, NULL))
         {
            dev_full_name                                = p_device_interface_detail_data->DevicePath;
            dev_full_name.MakeLower();
            if (dev_full_name.Find(dev_vid_pid_name, 0) != -1)
            {
               m_szDeviceName                            = p_device_interface_detail_data->DevicePath;
               if (p_device_interface_detail_data != NULL)
               {
                  delete p_device_interface_detail_data;
               }
               break;
            }
         }
         if (p_device_interface_detail_data != NULL)
         {
            delete p_device_interface_detail_data;
         }
      }
   }
   if (hDevInfo != INVALID_HANDLE_VALUE)
   {
      SetupDiDestroyDeviceInfoList(hDevInfo);
   }
   if (hModuleSetup != NULL)
   {
      FreeLibrary(hModuleSetup);
   }
   if (m_szDeviceName.IsEmpty() == TRUE)
   {
      return FALSE;
   }
   return TRUE;
}


/* --------------------------------------------------------------------------
 * Name : ConnectUsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
BOOL UsbBulkDevice::ConnectUsbBulkDevice()
{
   DWORD ret_bytes                                       = 0;
   DWORD bytes_written                                   = 0;
   UCHAR interface_number                                = 0;
   USB_DEVICE_DESCRIPTOR usb_device_descriptor           = { 0, };
   USB_CONFIGURATION_DESCRIPTOR usb_config_descriptor    = { 0, };
   USB_INTERFACE_DESCRIPTOR usb_interface_descriptor     = { 0, };
   m_usb_in_endpoint                                     = 0;
   m_usb_out_endpoint                                    = 0;

   if (m_szDeviceName.IsEmpty() == TRUE)
   {
      return FALSE;
   }

   UsbBulkDeviceClose();

   m_hUsbBulkDeviceHandle                                = CreateFile(m_szDeviceName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
   if (m_hUsbBulkDeviceHandle == INVALID_HANDLE_VALUE)
   {
      m_dwErrorCode                                      = GetLastError();
      return FALSE;
   }

   if (WinUsb_Initialize(m_hUsbBulkDeviceHandle, &m_UsbBulkDeviceInterfaceHandle) == FALSE)
   {
      m_dwErrorCode                                      = GetLastError();
      UsbBulkDeviceClose();
      return FALSE;
   }
   // Cache USB device descriptor
   if (WinUsb_GetCurrentAlternateSetting(m_UsbBulkDeviceInterfaceHandle, &interface_number) == FALSE)
   {
      m_dwErrorCode                                      = GetLastError();
      UsbBulkDeviceClose();
      return FALSE;
   }

   if (WinUsb_GetDescriptor(m_UsbBulkDeviceInterfaceHandle, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0, reinterpret_cast<PUCHAR>(&usb_device_descriptor), sizeof(usb_device_descriptor), &bytes_written) == FALSE)
   {
      m_dwErrorCode                                      = GetLastError();
      UsbBulkDeviceClose();
      return FALSE;
   }

   // Cache USB configuration descriptor
   if (WinUsb_GetDescriptor(m_UsbBulkDeviceInterfaceHandle, USB_CONFIGURATION_DESCRIPTOR_TYPE, 0, 0, reinterpret_cast<PUCHAR>(&usb_config_descriptor), sizeof(usb_config_descriptor), &bytes_written) == FALSE)
   {
      m_dwErrorCode                                      = GetLastError();
      UsbBulkDeviceClose();
      return FALSE;
   }
   // Cache USB interface descriptor
   if (WinUsb_QueryInterfaceSettings(m_UsbBulkDeviceInterfaceHandle, interface_number, &usb_interface_descriptor) == FALSE)
   {
      m_dwErrorCode                                      = GetLastError();
      UsbBulkDeviceClose();
      return FALSE;
   }

   for (UCHAR endpoint = 0; endpoint < usb_interface_descriptor.bNumEndpoints; endpoint++)
   {
      WINUSB_PIPE_INFORMATION pipe_info;
      if (WinUsb_QueryPipe(m_UsbBulkDeviceInterfaceHandle, interface_number, endpoint, &pipe_info) == TRUE)
      {
         if (UsbdPipeTypeBulk == pipe_info.PipeType)
         {
            if (0 != (pipe_info.PipeId & USB_ENDPOINT_DIRECTION_MASK))
            {
               m_usb_in_endpoint                         = pipe_info.PipeId;
            }
            else
            {
               m_usb_out_endpoint                        = pipe_info.PipeId;
            }
         }
      }
   }
   if (m_usb_in_endpoint != BULK_IN_EP || m_usb_out_endpoint != BULK_OUT_EP)
   {
      m_dwErrorCode                                      = GetLastError();
      UsbBulkDeviceClose();
      return FALSE;
   }

   return TRUE;
}

/* --------------------------------------------------------------------------
* Name : ConnectUsbBulkDevice()
*
*
* -------------------------------------------------------------------------- */
BOOL UsbBulkDevice::DisConnectUsbBulkDevice()
{
   m_usb_in_endpoint                                     = 0;
   m_usb_out_endpoint                                    = 0;
   UsbBulkDeviceClose();
   return TRUE;
}


/* --------------------------------------------------------------------------
 * Name : WriteUsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
BOOL UsbBulkDevice::WriteUsbBulkDevice(unsigned char* pdata, DWORD dwWriteBytes)
{
   DWORD dwWritenBytes                                   = 0;
   DWORD dwRet                                           = 0;
   DWORD dwWaitRet                                       = 0;
   MUTEX mutex;
   static OVERLAPPED write_overlapped;

   if (m_UsbBulkDeviceInterfaceHandle == INVALID_HANDLE_VALUE)
   {
      return FALSE;
   }

   mutex.LOCK();
   memset(&write_overlapped, 0, sizeof(write_overlapped));
   write_overlapped.hEvent                               = m_hEvent_Write_Handle;

   ResetEvent(m_hEvent_Write_Handle);
   dwRet                                                 = WinUsb_WritePipe(m_UsbBulkDeviceInterfaceHandle, m_usb_out_endpoint, (PUCHAR) pdata, dwWriteBytes, &dwWritenBytes, &write_overlapped);

   if (dwRet == FALSE)
   {
      CString szErrMsg;
      DWORD dwErrorCode                                  = GetLastError();
      if (dwErrorCode != ERROR_IO_PENDING)
      {
         return FALSE;
      }
      if (WinUsb_GetOverlappedResult(m_UsbBulkDeviceInterfaceHandle, &write_overlapped, &dwWritenBytes, FALSE) == FALSE)
      {
         dwErrorCode                                     = GetLastError();
         if (dwErrorCode != ERROR_IO_INCOMPLETE)
         {
            return FALSE;
         }
      }
   }
   dwWaitRet                                             = WaitForSingleObject(m_hEvent_Write_Handle, USB_BULK_DEVICE_TIME_OUT);
   if (dwWaitRet != WAIT_OBJECT_0)
   {
      return FALSE;
   }
   mutex.UNLOCK();
   return TRUE;
}

/* --------------------------------------------------------------------------
 * Name : ReadUsbBulkDevice()
 *
 *
 * -------------------------------------------------------------------------- */
BOOL UsbBulkDevice::ReadUsbBulkDevice(unsigned char* pdata, DWORD dwReadBytes)
{
   DWORD dwRet                                           = 0;
   DWORD dwWaitRet                                       = 0;
   MUTEX mutex;
   static OVERLAPPED read_overlapped;

   mutex.LOCK();
   memset(&read_overlapped, 0, sizeof(read_overlapped));
   read_overlapped.hEvent                                = m_hEvent_Read_Handle;

   ResetEvent(m_hEvent_Read_Handle);
   dwRet                                                 = WinUsb_ReadPipe(m_UsbBulkDeviceInterfaceHandle, m_usb_in_endpoint, pdata, USB_BULK_DEVICE_PACKET_SIZE, &dwReadBytes, &read_overlapped);

   if (dwRet == FALSE)
   {
      CString szErrMsg;
      DWORD dwErrorCode                                  = GetLastError();
      if (dwErrorCode != ERROR_IO_PENDING)
      {
         return FALSE;
      }
      if (WinUsb_GetOverlappedResult(m_UsbBulkDeviceInterfaceHandle, &read_overlapped, &dwReadBytes, FALSE) == FALSE)
      {
         dwErrorCode                                     = GetLastError();
         if (dwErrorCode != ERROR_IO_INCOMPLETE)
         {
            return FALSE;
         }
      }
   }
   dwRet                                                 = WaitForSingleObject(m_hEvent_Read_Handle, USB_BULK_DEVICE_TIME_OUT);
   if (dwRet == WAIT_OBJECT_0)
   {
      return TRUE;
   }
   mutex.UNLOCK();
   return FALSE;
}








