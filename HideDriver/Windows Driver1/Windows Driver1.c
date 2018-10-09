#include <ntddk.h>
#include "GET_MIPROCESSLOADERENTRY.h"
#include "SYSTEM_MODULE_STRUCT.h"

VOID Reinitialize(
	PDRIVER_OBJECT        DriverObject,
	PVOID                 Context,
	ULONG                 Count
	)
{
	MiProcessLoaderEntry m_MiProcessLoaderEntry = Get_MiProcessLoaderEntry();
	if (m_MiProcessLoaderEntry == NULL)
		return;

	m_MiProcessLoaderEntry(DriverObject->DriverSection, 0);
	DriverObject->DriverSection = NULL;
	DriverObject->DriverStart = NULL;
	DriverObject->DriverSize = NULL;
	DriverObject->DriverUnload = NULL;
	DriverObject->DriverInit = NULL;
	DriverObject->DeviceObject = NULL;
}

VOID Unload(PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Unload Success!\n"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegString)
{
	KdPrint(("Entry Driver!\n"));
	//HideDriver(DriverObject);
	DbgBreakPoint();
	IoRegisterDriverReinitialization(DriverObject, Reinitialize, NULL);
	//DriverObject->DriverUnload = Unload;
	return STATUS_SUCCESS;
}
