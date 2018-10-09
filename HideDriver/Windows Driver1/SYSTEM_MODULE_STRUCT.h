#pragma once

#ifndef SYSTEM_MODULE_STRUCT_H
#define SYSTEM_MODULE_STRUCT_H

#include <ntddk.h>

#define SystemModuleInformation 11

typedef NTSTATUS(*NTQUERYSYSTEMINFORMATION)(
	IN ULONG SystemInformationClass,
	OUT PVOID   SystemInformation,
	IN ULONG_PTR    SystemInformationLength,
	OUT PULONG_PTR  ReturnLength OPTIONAL);

typedef struct _SYSTEM_MODULE_INFORMATION{
	HANDLE Section;
	PVOID MappedBase;
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT PathLength;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

#endif