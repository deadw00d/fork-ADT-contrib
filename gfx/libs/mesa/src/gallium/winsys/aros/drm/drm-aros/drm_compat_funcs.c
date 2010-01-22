/*
    Copyright 2009, The AROS Development Team. All rights reserved.
    $Id$
*/

#include "drm_compat_funcs.h"
#include "drm_aros_config.h"

#define PIO_RESERVED                (IPTR)0x40000UL

void iowrite32(u32 val, void * addr)
{
    if ((IPTR)addr >= PIO_RESERVED)
        writel(val, addr);
    else
        IMPLEMENT("PIO\n");
}

unsigned int ioread32(void * addr)
{
    if ((IPTR)addr >= PIO_RESERVED)
        return readl(addr);
    else
        IMPLEMENT("PIO\n");
    
    return 0xffffffff;
}

void iowrite16(u16 val, void * addr)
{
    if ((IPTR)addr >= PIO_RESERVED)
        writew(val, addr);
    else
        IMPLEMENT("PIO\n");
}

unsigned int ioread16(void * addr)
{
    if ((IPTR)addr >= PIO_RESERVED)
        return readw(addr);
    else
        IMPLEMENT("PIO\n");
    
    return 0xffff;
}

void iowrite8(u8 val, void * addr)
{
    if ((IPTR)addr >= PIO_RESERVED)
        writeb(val, addr);
    else
        IMPLEMENT("PIO\n");
}

unsigned int ioread8(void * addr)
{
    if ((IPTR)addr >= PIO_RESERVED)
        return readb(addr);
    else
        IMPLEMENT("PIO\n");
    
    return 0xff;
}

/* Bit operations */
void clear_bit(int nr, volatile void * addr)
{
    unsigned long mask = 1 << nr;
    
    *(unsigned long*)addr &= ~mask;
}

void set_bit(int nr, volatile void *addr)
{
    unsigned long mask = 1 << nr;
    
    *(unsigned long*)addr |= mask;
}

int test_bit(int nr, volatile void *addr)
{
    unsigned long mask = 1 << nr;
    
    return (*(unsigned long*)addr) & mask;
}



/* Page handling */
void __free_page(struct page * p)
{
    if (p->allocated_buffer)
        FreeVec(p->allocated_buffer);
    p->allocated_buffer = NULL;
    p->address = NULL;
    FreeVec(p);
}

struct page * create_page_helper()
{
    struct page * p;
    p = AllocVec(sizeof(*p), MEMF_PUBLIC | MEMF_CLEAR);
    p->allocated_buffer = AllocVec(PAGE_SIZE + PAGE_SIZE - 1, MEMF_PUBLIC | MEMF_CLEAR);
    p->address = PAGE_ALIGN(p->allocated_buffer);
    return p;
}

/* IDR handling */
int idr_pre_get_internal(struct idr *idp)
{
    if (idp->size <= idp->occupied + idp->last_starting_id)
    {
        /* Create new table */
        ULONG newsize = idp->size ? idp->size * 2 : 128;
        IPTR * newtab = AllocVec(newsize * sizeof(IPTR), MEMF_PUBLIC | MEMF_CLEAR);
        
        if (newtab == NULL)
            return 0;
        
        
        if (idp->pointers)
        {
            /* Copy old table into new */
            CopyMem(idp->pointers, newtab, idp->size * sizeof(IPTR));
            
            /* Release old table */
            FreeVec(idp->pointers);
        }
        
        idp->pointers = newtab;
        idp->size = newsize;
    }
    
    return 1;
}

int idr_get_new_above(struct idr *idp, void *ptr, int starting_id, int *id)
{
    int i = starting_id;
    idp->last_starting_id = starting_id;

    for(;i < idp->size;i++)
    {
        if (idp->pointers[i] == (IPTR)NULL)
        {
            *id = i;
            idp->pointers[i] = (IPTR)ptr;
            idp->occupied++;
            return 0;
        }
    }
    
    return -EAGAIN;
}

void *idr_find(struct idr *idp, int id)
{
    if ((id < idp->size) && (id >= 0))
        return (APTR)idp->pointers[id];
    else
        return NULL;
}

void idr_remove(struct idr *idp, int id)
{
    if ((id < idp->size) && (id >= 0))
    {
        idp->pointers[id] = (IPTR)NULL;
        idp->occupied--;
    }
}

void idr_init(struct idr *idp)
{
    idp->size = 0;
    idp->pointers = NULL;
    idp->occupied = 0;
    idp->last_starting_id = 0;
}

#include "drm_aros.h"
#include <aros/libcall.h>
#include <proto/oop.h>
#include <hidd/pci.h>
#include <hidd/hidd.h>

void *ioremap(resource_size_t offset, unsigned long size)
{
#if !defined(HOSTED_BUILD)
    if (pciDriver)
    {
        struct pHidd_PCIDriver_MapPCI mappci,*msg = &mappci;
        mappci.mID = OOP_GetMethodID(IID_Hidd_PCIDriver, moHidd_PCIDriver_MapPCI);
        mappci.PCIAddress = (APTR)offset;
        mappci.Length = size;
        return (APTR)OOP_DoMethod(pciDriver, (OOP_Msg)msg);
    }
    else
    {
        bug("BUG: ioremap used without acquiring pciDriver\n");
        return NULL;
    }
#else
    /* For better simulation:
    a) make a list of already "mapped" buffers keyed by APTR buf
    b) check if a request (buf + size) is inside of already mapped region -> return pointer in mapped region
    Why: sometimes the same range is mapped more than once
    */
    return AllocVec(size, MEMF_PUBLIC | MEMF_CLEAR); /* This will leak */
#endif
}

void iounmap(void * addr)
{
#if !defined(HOSTED_BUILD)
    if (pciDriver)
    {
        struct pHidd_PCIDriver_UnmapPCI unmappci,*msg=&unmappci;

        unmappci.mID = OOP_GetMethodID(IID_Hidd_PCIDriver, moHidd_PCIDriver_UnmapPCI);
        unmappci.CPUAddress = addr;
        unmappci.Length = 0; /* This works on i386 but may create problems on other archs */

        OOP_DoMethod(pciDriver, (OOP_Msg)msg);
    }
#else
    /* If "better simulation" is implemented (see ioremap) memory
    can only be freed if there is no other mappings to this buffer */
    FreeVec(addr);
#endif
}

resource_size_t pci_resource_start(void * pdev, unsigned int resource)
{
#if !defined(HOSTED_BUILD)    
    APTR start = (APTR)NULL;
    switch(resource)
    {
        case(0): OOP_GetAttr(pdev, aHidd_PCIDevice_Base0, (APTR)&start); break;
        case(1): OOP_GetAttr(pdev, aHidd_PCIDevice_Base1, (APTR)&start); break;
        case(2): OOP_GetAttr(pdev, aHidd_PCIDevice_Base2, (APTR)&start); break;
        case(3): OOP_GetAttr(pdev, aHidd_PCIDevice_Base3, (APTR)&start); break;
        case(4): OOP_GetAttr(pdev, aHidd_PCIDevice_Base4, (APTR)&start); break;
        case(5): OOP_GetAttr(pdev, aHidd_PCIDevice_Base5, (APTR)&start); break;
        default: bug("ResourceID %d not supported\n", resource);
    }
    
    return (resource_size_t)start;
#else
#if HOSTED_BUILD_HARDWARE == HOSTED_BUILD_HARDWARE_NVIDIA
#if HOSTED_BUILD_CHIPSET >= 0x40
if (resource == 0) return (resource_size_t)0xcf000000;
if (resource == 1) return (resource_size_t)0xb0000000;
if (resource == 3) return (resource_size_t)0xce000000;
#else
if (resource == 0) return (resource_size_t)0xe7000000;
if (resource == 1) return (resource_size_t)0xf0000000;
if (resource == 2) return (resource_size_t)0xef800000;
#endif
#endif
#if HOSTED_BUILD_HARDWARE == HOSTED_BUILD_HARDWARE_I915
if (resource == 0) return (resource_size_t)0x50300000;
if (resource == 1) return (resource_size_t)0x000030e0;
if (resource == 2) return (resource_size_t)0x40000000;
if (resource == 3) return (resource_size_t)0x50380000;
#endif
return (resource_size_t)0;
#endif
}

unsigned long pci_resource_len(void * pdev, unsigned int resource)
{
#if !defined(HOSTED_BUILD)    
    IPTR len = (IPTR)0;
    
    if (pci_resource_start(pdev, resource) != 0)
    {
        /* 
         * FIXME:
         * The length reading is only correct when the resource actually exists.
         * pci.hidd can however return a non 0 lenght for a resource that does
         * not exsist. Possible fix in pci.hidd needed
         */
        
        switch(resource)
        {
            case(0): OOP_GetAttr(pdev, aHidd_PCIDevice_Size0, (APTR)&len); break;
            case(1): OOP_GetAttr(pdev, aHidd_PCIDevice_Size1, (APTR)&len); break;
            case(2): OOP_GetAttr(pdev, aHidd_PCIDevice_Size2, (APTR)&len); break;
            case(3): OOP_GetAttr(pdev, aHidd_PCIDevice_Size3, (APTR)&len); break;
            case(4): OOP_GetAttr(pdev, aHidd_PCIDevice_Size4, (APTR)&len); break;
            case(5): OOP_GetAttr(pdev, aHidd_PCIDevice_Size5, (APTR)&len); break;
            default: bug("ResourceID %d not supported\n", resource);
        }
    }
    
    return len;
#else
#if HOSTED_BUILD_HARDWARE == HOSTED_BUILD_HARDWARE_NVIDIA
#if HOSTED_BUILD_CHIPSET >= 0x40
if (resource == 0) return (IPTR)0x1000000;
if (resource == 1) return (IPTR)0x10000000;
if (resource == 3) return (IPTR)0x1000000;
#else
if (resource == 0) return (IPTR)0x1000000;
if (resource == 1) return (IPTR)0x8000000;
if (resource == 2) return (IPTR)0x80000;
#endif
#endif
#if HOSTED_BUILD_HARDWARE == HOSTED_BUILD_HARDWARE_I915
if (resource == 0) return (IPTR)0x80000;
if (resource == 1) return (IPTR)0x8;
if (resource == 2) return (IPTR)0x10000000;
if (resource == 3) return (IPTR)0x40000;
#endif
return (IPTR)0;
#endif
}

struct GetBusSlotEnumeratorData
{
    IPTR Bus;
    IPTR Dev;
    IPTR Sub;
    OOP_Object ** pciDevice;
};

AROS_UFH3(void, GetBusSlotEnumerator,
    AROS_UFHA(struct Hook *, hook, A0),
    AROS_UFHA(OOP_Object *, pciDevice, A2),
    AROS_UFHA(APTR, message, A1))
{
    AROS_USERFUNC_INIT
    
    struct GetBusSlotEnumeratorData * data = hook->h_Data;
    
    IPTR Bus;
    IPTR Dev;
    IPTR Sub;    

    if (*data->pciDevice)
        return; /* Already found, should not happen */
    
    /* Get the Device's properties */
    OOP_GetAttr(pciDevice, aHidd_PCIDevice_Bus, &Bus);
    OOP_GetAttr(pciDevice, aHidd_PCIDevice_Dev, &Dev);
    OOP_GetAttr(pciDevice, aHidd_PCIDevice_Sub, &Sub);

    if (data->Bus == Bus &&
        data->Dev == Dev &&
        data->Sub == Sub)
    {
        (*data->pciDevice) = pciDevice;
    }
    
    AROS_USERFUNC_EXIT
}   
void * pci_get_bus_and_slot(unsigned int bus, unsigned int dev, unsigned int fun)
{
#if !defined(HOSTED_BUILD)
    OOP_Object * pciDevice = NULL;

    if (pciBus)
    {
        struct GetBusSlotEnumeratorData data = {
        Bus: bus,
        Dev: dev,
        Sub: fun,
        pciDevice: &pciDevice,
        };
        
        struct Hook FindHook = {
        h_Entry:    (IPTR (*)())GetBusSlotEnumerator,
        h_Data:     &data,
        };

        struct TagItem Requirements[] = {
        { TAG_DONE,             0UL }
        };
    
        struct pHidd_PCI_EnumDevices enummsg = {
        mID:        OOP_GetMethodID(IID_Hidd_PCI, moHidd_PCI_EnumDevices),
        callback:   &FindHook,
        requirements:   (struct TagItem*)&Requirements,
        }, *msg = &enummsg;
        
        OOP_DoMethod(pciBus, (OOP_Msg)msg);
    }
    
    return pciDevice;
#else
    return AllocVec(1, MEMF_ANY);
#endif
}

int pci_read_config_word(void *dev, int where, u16 *val)
{
#if !defined(HOSTED_BUILD)
    struct pHidd_PCIDevice_ReadConfigWord rcwmsg = {
    mID: OOP_GetMethodID(IID_Hidd_PCIDevice, moHidd_PCIDevice_ReadConfigWord),
    reg: (UBYTE)where,
    }, *msg = &rcwmsg;
    
    *val = (UWORD)OOP_DoMethod((OOP_Object*)dev, (OOP_Msg)msg);
#else
#if HOSTED_BUILD_HARDWARE == HOSTED_BUILD_HARDWARE_I915
    switch(where)
    {
        case 0x52: *val = 48; break;
        default: *val = 0; IMPLEMENT("check correct value with iMica\n"); break;
    }
#endif
#endif
    bug("pci_read_config_word: %d -> %d\n", where, *val);
    
    return 0;
}

int pci_read_config_dword(void *dev, int where, u32 *val)
{
#if !defined(HOSTED_BUILD)
    struct pHidd_PCIDevice_ReadConfigLong rclmsg = {
    mID: OOP_GetMethodID(IID_Hidd_PCIDevice, moHidd_PCIDevice_ReadConfigLong),
    reg: (UBYTE)where,
    }, *msg = &rclmsg;
    
    *val = (ULONG)OOP_DoMethod((OOP_Object*)dev, (OOP_Msg)msg);
#else
#if HOSTED_BUILD_HARDWARE == HOSTED_BUILD_HARDWARE_I915
    switch(where)
    {
        default: *val = 0; IMPLEMENT("check correct value with iMica\n"); break;
    }
#endif
#endif
    bug("pci_read_config_dword: %d -> %d\n", where, *val);
    
    return 0;
}

int pci_write_config_dword(void *dev, int where, u32 val)
{
#if !defined(HOSTED_BUILD)
    struct pHidd_PCIDevice_WriteConfigLong wclmsg = {
    mID: OOP_GetMethodID(IID_Hidd_PCIDevice, moHidd_PCIDevice_ReadConfigLong),
    reg: (UBYTE)where,
    val: val,
    }, *msg = &wclmsg;
    
    OOP_DoMethod((OOP_Object*)dev, (OOP_Msg)msg);
#else
#endif
    bug("pci_write_config_dword: %d -> %d\n", where, val);
    
    return 0;
}









/* AGP handling */
struct agp_bridge_data *agp_backend_acquire(void * dev)
{
#if !defined(HOSTED_BUILD)
    IMPLEMENT("\n");
    return NULL;
#else
    struct agp_bridge_data * bridge = AllocVec(sizeof(struct agp_bridge_data), 
                                        MEMF_PUBLIC | MEMF_CLEAR);
    return bridge;
#endif
}

void agp_backend_release(struct agp_bridge_data * bridge)
{
    IMPLEMENT("\n");
}

struct agp_bridge_data * agp_find_bridge(void * dev)
{
#if !defined(HOSTED_BUILD)
    IMPLEMENT("\n");
    return NULL;
#else
    struct agp_bridge_data * bridge = AllocVec(sizeof(struct agp_bridge_data), 
                                        MEMF_PUBLIC | MEMF_CLEAR);
    return bridge;
#endif
}

int agp_copy_info(struct agp_bridge_data * bridge, struct agp_kern_info * info)
{
#if !defined(HOSTED_BUILD)
    IMPLEMENT("\n");
#else
    info->chipset = SUPPORTED;
    info->cant_use_aperture = 0;
    info->aper_base = 0xb0000000; /* FIXME: Probably makes no sense */
    info->aper_size = 64;
#endif
    return 1;
}

void agp_enable(struct agp_bridge_data * bridge, u32 mode)
{
    IMPLEMENT("\n");
}

struct agp_memory *agp_allocate_memory(struct agp_bridge_data * bridge, 
    size_t num_pages , u32 type)
{
    struct agp_memory * mem = AllocVec(sizeof(struct agp_memory), MEMF_PUBLIC | MEMF_CLEAR);
    mem->pages = AllocVec(sizeof(struct page *) * num_pages, MEMF_PUBLIC | MEMF_CLEAR);
    mem->type = type;
    mem->is_flushed = FALSE;
    mem->is_bound = FALSE;
    return mem;
}

int agp_bind_memory(struct agp_memory * mem, off_t offset)
{
#if !defined(HOSTED_BUILD)
    IMPLEMENT("\n");
    return -1;
#else
    mem->is_bound = TRUE;
    return 0;
#endif
}

int agp_unbind_memory(struct agp_memory * mem)
{
#if !defined(HOSTED_BUILD)
    IMPLEMENT("\n");
    return -1;
#else
    mem->is_bound = FALSE;
    return 0;
#endif
}

void agp_free_memory(struct agp_memory * mem)
{
    FreeVec(mem->pages);
    FreeVec(mem);
}
