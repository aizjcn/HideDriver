#include <ntddk.h>
#include "GET_MIPROCESSLOADERENTRY.h"
#include "SYSTEM_MODULE_STRUCT.h"

MiProcessLoaderEntry m_MiProcessLoaderEntry = NULL;

BOOLEAN JudgeLoadDriver()
{
	NTQUERYSYSTEMINFORMATION m_NtQuerySystemInformation = NULL;
	UNICODE_STRING NtQuerySystemInformation_Name;
	PSYSTEM_MODULE_INFORMATION ModuleEntry;
	ULONG_PTR RetLength, BaseAddr, EndAddr;
	ULONG ModuleNumbers, Index;
	NTSTATUS Status;
	PVOID Buffer;

	RtlInitUnicodeString(&NtQuerySystemInformation_Name, L"NtQuerySystemInformation");
	m_NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)MmGetSystemRoutineAddress(&NtQuerySystemInformation_Name);
	if (m_NtQuerySystemInformation == NULL)
	{
		KdPrint(("获取NtQuerySystemInformation函数失败！\n"));
		return FALSE;
	}

	RetLength = 0;
	Status = m_NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &RetLength);
	if (!NT_SUCCESS(Status) && Status != STATUS_INFO_LENGTH_MISMATCH)
	{
		KdPrint(("NtQuerySystemInformation调用失败！错误码是：%x\n", Status));
		return FALSE;
	}

	Buffer = ExAllocatePoolWithTag(NonPagedPool, RetLength, 'ytz');
	if (Buffer == NULL)
	{
		KdPrint(("分配内存失败！\n"));
		return FALSE;
	}

	Status = m_NtQuerySystemInformation(SystemModuleInformation, Buffer, RetLength, &RetLength);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("NtQuerySystemInformation调用失败！错误码是：%x\n", Status));
		return FALSE;
	}

	ModuleNumbers = *(ULONG*)Buffer;
	ModuleEntry = (PSYSTEM_MODULE_INFORMATION)((ULONG_PTR)Buffer + 8);
	for (Index = 0; Index < ModuleNumbers; ++Index)
	{
		BaseAddr = (ULONG_PTR)ModuleEntry->Base;
		EndAddr = BaseAddr + ModuleEntry->Size;
		if (BaseAddr <= (ULONG_PTR)JudgeLoadDriver && (ULONG_PTR)JudgeLoadDriver <= EndAddr)
		{
			KdPrint(("模块名称是：%s\n", ModuleEntry->ImageName));
			return TRUE;
		}
		++ModuleEntry;
	}

	return FALSE;
}

VOID HideDriverThread(PVOID pDriverObject)
{
	PDRIVER_OBJECT DriverObject;
	LARGE_INTEGER SleepTime;
	BOOLEAN RetValue;

	SleepTime.QuadPart = -100 * 1000 * 1000 * 10;				//找不到则休眠10毫秒

	//如果传入的是0，则代表没有找到
	while (TRUE)
	{
		RetValue = JudgeLoadDriver();
		if (RetValue == FALSE)
			KeDelayExecutionThread(KernelMode, 0, &SleepTime);
		else
			break;
	}

	DriverObject = (PDRIVER_OBJECT)pDriverObject;

	m_MiProcessLoaderEntry(DriverObject->DriverSection, 0);
	DriverObject->DriverSection = NULL;
	DriverObject->DriverStart = NULL;
	DriverObject->DriverSize = NULL;
	DriverObject->DriverUnload = NULL;
	DriverObject->DriverInit = NULL;
	DriverObject->DeviceObject = NULL;

	PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID HideDriver(PDRIVER_OBJECT DriverObject)
{
	HANDLE hThread = NULL;

	m_MiProcessLoaderEntry = Get_MiProcessLoaderEntry();
	if (m_MiProcessLoaderEntry == NULL)
		return;

	PsCreateSystemThread(&hThread, 0, NULL, NULL, NULL, HideDriverThread, (PVOID)DriverObject);
	ZwClose(hThread);
}

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
