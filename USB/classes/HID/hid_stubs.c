/*
    Copyright � 2004, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Stub functions for PCI subsystem
    Lang: English
 */

#ifndef AROS_USE_OOP
#   define AROS_USE_OOP
#endif

#include <stdint.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>

#include <utility/tagitem.h>
#include <utility/hooks.h>

#include <oop/oop.h>

#include <hidd/hidd.h>
#include <usb/usb.h>
#include <usb/hid.h>

#include <proto/oop.h>

#undef OOPBase
#define OOPBase (OOP_OOPBASE(obj))

#ifdef AROS_CREATE_ROM
#error  Do not use stubs in ROM code!!!
#else
#define STATIC_MID  static OOP_MethodID mid
#endif

BOOL HIDD_USBHID_GetReportDescriptor(OOP_Object *obj, uint16_t length, void *buffer)
{
    STATIC_MID;
    struct pHidd_USBHID_GetReportDescriptor p;

    if (!mid) mid = OOP_GetMethodID((STRPTR)IID_Hidd_USBHID, moHidd_USBHID_GetReportDescriptor);

    p.mID = mid;
    p.length = length;
    p.buffer = buffer;

    return OOP_DoMethod(obj, &p.mID);
}

usb_hid_descriptor_t *HIDD_USBHID_GetHidDescriptor(OOP_Object *obj)
{
    STATIC_MID;
    struct pHidd_USBHID_GetHidDescriptor p;

    if (!mid) mid = OOP_GetMethodID((STRPTR)IID_Hidd_USBHID, moHidd_USBHID_GetHidDescriptor);

    p.mID = mid;

    return (usb_hid_descriptor_t *)OOP_DoMethod(obj, &p.mID);
}

void HIDD_USBHID_ParseReport(OOP_Object *obj, void *report)
{
    STATIC_MID;
    struct pHidd_USBHID_ParseReport p;

    if (!mid) mid = OOP_GetMethodID((STRPTR)IID_Hidd_USBHID, moHidd_USBHID_ParseReport);

    p.mID = mid;
    p.report = report;

    OOP_DoMethod(obj, &p.mID);
}

