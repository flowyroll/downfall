#include "ntifs.h"
#include "ntddk.h"

#define PTEDITOR_READ_PAGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_WRITE_PAGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA)
#define PTEDITOR_GET_CR3 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_SET_CR3 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_FLUSH_TLB CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_READ_PHYS_VAL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_WRITE_PHYS_VAL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_SET_PAT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_GET_PAT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\PTEditor");
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\PTEditorLink");

PDEVICE_OBJECT DeviceObject = NULL;

#define IA32_PAT 0x277

#define TAG_INFO "[PTEditor:Info] "
#define TAG_WARN "[PTEditor:Warning] "
#define TAG_ERROR "[PTEditor:ERROR] "

VOID Unload(_In_ PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);

    IoDeleteSymbolicLink(&SymLinkName);
    IoDeleteDevice(DeviceObject);

    DbgPrint(TAG_INFO "PTEditor unloaded\r\n");
}

__pragma(pack(push, 1))
typedef struct {
    char content[4096];
    size_t paddr;
} PageContent;
__pragma(pack(pop))

static ULONG_PTR invalidate_tlb(ULONG_PTR addr) {
    __invlpg((void*)addr);
    return (ULONG_PTR)NULL;
}

static ULONG_PTR set_pat(ULONG_PTR pat) {
    __writemsr(IA32_PAT, pat);
    return (ULONG_PTR)NULL;
}

NTSTATUS DispatchDeviceCtl(PDEVICE_OBJECT DevObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DevObject);

    PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG returnLength = 0;
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    PEPROCESS Process;
    SIZE_T transferred;
    MM_COPY_ADDRESS mem;
    PageContent* content;
    PHYSICAL_ADDRESS pa;
    SIZE_T val;
    PVOID vaddr;

    switch (irpsp->Parameters.DeviceIoControl.IoControlCode) {
    case PTEDITOR_WRITE_PAGE:
        content = (PageContent*)buffer;
        DbgPrint(TAG_INFO "write page %zx\r\n", content->paddr);
        pa.QuadPart = content->paddr;
        vaddr = MmGetVirtualForPhysical(pa);
        mem.VirtualAddress = (PVOID)(content->content);
        DbgPrint(TAG_INFO " virtual address to write to: %zx\r\n", vaddr);
        if (vaddr) {
            MmCopyMemory(vaddr, mem, 4096, MM_COPY_MEMORY_VIRTUAL, &transferred);
        }
        else {
            DbgPrint(TAG_WARN " could not write to page!\r\n");
        }
        returnLength = 0;
        break;
    case PTEDITOR_READ_PAGE:
        DbgPrint(TAG_INFO "read page %zx\r\n", *(SIZE_T*)buffer);
        mem.PhysicalAddress = *(PHYSICAL_ADDRESS*)buffer;
        MmCopyMemory(buffer, mem, 4096, MM_COPY_MEMORY_PHYSICAL, &transferred);
        returnLength = 4096;
        break;
    case PTEDITOR_GET_CR3:
        DbgPrint(TAG_INFO "get CR3 for %zx\r\n", *((SIZE_T*)buffer));
        if(PsLookupProcessByProcessId((HANDLE)(*((PHANDLE)buffer)), &Process) == STATUS_SUCCESS) {
            KAPC_STATE apcState;
            KeStackAttachProcess(Process, &apcState);
            SIZE_T cr3 = __readcr3();
            KeUnstackDetachProcess(&apcState);
            DbgPrint(TAG_INFO " -> CR3: %zx\r\n", cr3);
            *((SIZE_T*)buffer) = cr3;
        }
        else {
            DbgPrint(TAG_WARN "could not find process!\r\n");
            *((SIZE_T*)buffer) = 0;
        }
        returnLength = sizeof(SIZE_T);
        break;
    case PTEDITOR_SET_CR3:
        DbgPrint(TAG_INFO "set CR3 for %zx\r\n", *((SIZE_T*)buffer));
        if(PsLookupProcessByProcessId((HANDLE)(*((PHANDLE)buffer)), &Process) == STATUS_SUCCESS) {
            KAPC_STATE apcState;
            KeStackAttachProcess(Process, &apcState);
            __writecr3(*(((SIZE_T*)buffer) + 1));
            KeUnstackDetachProcess(&apcState);
            DbgPrint(TAG_INFO " -> new CR3: %zx\r\n", *(((SIZE_T*)buffer) + 1));
        }
        else {
            DbgPrint(TAG_WARN "could not find process!\r\n");
        }
        returnLength = 0;
    case PTEDITOR_FLUSH_TLB:
        DbgPrint(TAG_INFO "flush TLB for %zx\r\n", *((SIZE_T*)buffer));
        KeIpiGenericCall(invalidate_tlb, (ULONG_PTR)(*(SIZE_T*)buffer));
        returnLength = 0;
        break;
    case PTEDITOR_READ_PHYS_VAL:
        DbgPrint(TAG_INFO "read physical value %zx\r\n", *(SIZE_T*)buffer);
        mem.PhysicalAddress = *(PHYSICAL_ADDRESS*)buffer;
        MmCopyMemory(buffer, mem, sizeof(SIZE_T), MM_COPY_MEMORY_PHYSICAL, &transferred);
        returnLength = sizeof(SIZE_T);
        break;
    case PTEDITOR_WRITE_PHYS_VAL:
        val = *(((SIZE_T*)buffer) + 1);
        DbgPrint(TAG_INFO "write physical value %zx to %zx\r\n", val, *(SIZE_T*)buffer);
        pa.QuadPart = *(SIZE_T*)buffer;
        vaddr = MmGetVirtualForPhysical(pa);
        if (vaddr) {
            *(SIZE_T*)vaddr = val;
        }
        else {
            DbgPrint(TAG_WARN "could not write to address!\r\n");
        }
        returnLength = 0;
        break;
    case PTEDITOR_GET_PAT:
        *(SIZE_T*)buffer = __readmsr(IA32_PAT); 
        returnLength = sizeof(SIZE_T);
        break;
    case PTEDITOR_SET_PAT:
        KeIpiGenericCall(set_pat, (ULONG_PTR)(*(SIZE_T*)buffer));
        returnLength = 0;
        break;
    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    Irp->IoStatus.Information = returnLength;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DispatchPassThru(PDEVICE_OBJECT DevObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DevObject);

    PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    switch (irpsp->MajorFunction) {
    case IRP_MJ_CREATE:
        DbgPrint(TAG_INFO "PTEditor create\r\n");
        break;
    case IRP_MJ_CLOSE:
        DbgPrint(TAG_INFO "PTeditor close\r\n");
        break;
    case IRP_MJ_CLEANUP:
        DbgPrint(TAG_INFO "No application uses PTeditor anymore\r\n");
        break;
    default:
        DbgPrint(TAG_WARN "Invalid major function %d\r\n", irpsp->MajorFunction);
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    int i;

    DriverObject->DriverUnload = Unload;

    DbgPrint(TAG_INFO "Initializing PTEditor!\r\n");

    NTSTATUS status;

    status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
    if (!NT_SUCCESS(status)) {
        DbgPrint(TAG_ERROR "Failed to create PTEditor device\r\n");
        return status;
    }

    status = IoCreateSymbolicLink(&SymLinkName, &DeviceName);
    if (!NT_SUCCESS(status)) {
        DbgPrint(TAG_ERROR "Failed to create symbolic link for PTEditor\r\n");
        IoDeleteDevice(DeviceObject);
        return status;
    }

    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = DispatchPassThru;
    }

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceCtl;

    DbgPrint(TAG_INFO "PTEditor successfully loaded\r\n");

    return status;
}
