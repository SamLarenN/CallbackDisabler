#pragma once

#ifndef UTILS_H
#define UTILS_H

/*
	Traversing the DriverObjects linked list for the specified driver.
*/
BOOLEAN GetDriverInformation(
	_In_ wchar_t* DriverName,
	_In_ PDRIVER_OBJECT pDriver,
	_Out_ UINT64* ModuleBase,
	_Out_ UINT64* ModuleSize
);

UINT64 FindPattern(
	_In_ UINT64 Base,
	_In_ UINT64 Size,
	_In_ char* Pattern,
	_In_ size_t PatternSize
);

BOOLEAN InitDynamicData();

#endif // !UTILS_H
